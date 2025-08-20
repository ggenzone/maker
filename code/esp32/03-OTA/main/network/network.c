#include "network.h"
#include "storage/app_config.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/ip4_addr.h"
#include <string.h>

static const char *TAG = "NETWORK";

// Event group bits
#define NETWORK_CONNECTED_BIT    BIT0
#define NETWORK_FAIL_BIT        BIT1
#define NETWORK_SCAN_DONE_BIT   BIT2

// Static variables
static bool network_initialized = false;
static esp_netif_t *sta_netif = NULL;
static EventGroupHandle_t network_event_group = NULL;
static network_state_t current_state = NETWORK_STATE_DISCONNECTED;
static network_event_cb_t event_callback = NULL;
static network_scan_cb_t scan_callback = NULL;
static network_stats_t stats = {0};
static network_info_t current_info = {0};

// Auto-reconnect settings
static bool auto_reconnect_enabled = true;
static uint32_t auto_reconnect_max_attempts = 5;
static uint32_t auto_reconnect_delay_ms = 5000;
static uint32_t reconnect_attempt_count = 0;

// Connection settings
static char stored_ssid[33] = {0};
static char stored_password[64] = {0};
static uint32_t connection_timeout_ms = 15000;

// Scan results
static network_ap_info_t *scan_results = NULL;
static uint16_t scan_count = 0;

// Forward declarations
static void network_event_handler(void *arg, esp_event_base_t event_base, 
                                 int32_t event_id, void *event_data);
static void network_set_state(network_state_t new_state);
static void network_update_info(void);
static void network_auto_reconnect_task(void *pvParameters);
static network_security_t wifi_auth_mode_to_security(wifi_auth_mode_t auth_mode);

esp_err_t network_init(void) {
    ESP_LOGI(TAG, "Initializing network subsystem");
    
    if (network_initialized) {
        ESP_LOGW(TAG, "Network already initialized");
        return ESP_OK;
    }
    
    // Initialize network interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Create event group
    network_event_group = xEventGroupCreate();
    if (network_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_ERR_NO_MEM;
    }
    
    // Create default WiFi STA interface
    sta_netif = esp_netif_create_default_wifi_sta();
    if (sta_netif == NULL) {
        ESP_LOGE(TAG, "Failed to create STA interface");
        return ESP_FAIL;
    }
    
    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    // Register event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
                                              &network_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, 
                                              &network_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_LOST_IP, 
                                              &network_event_handler, NULL));
    
    // Set WiFi mode to STA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Load settings from configuration
    connection_timeout_ms = 3000; //app_config_get_connection_timeout();
    auto_reconnect_enabled = true; // app_config_get_auto_reconnect();
    
    // Initialize stats
    memset(&stats, 0, sizeof(stats));
    memset(&current_info, 0, sizeof(current_info));
    
    network_set_state(NETWORK_STATE_DISCONNECTED);
    network_initialized = true;
    
    ESP_LOGI(TAG, "Network subsystem initialized successfully");
    return ESP_OK;
}

esp_err_t network_deinit(void) {
    if (!network_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing network subsystem");
    
    // Disconnect if connected
    network_disconnect();
    
    // Unregister event handlers
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &network_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &network_event_handler);
    esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_LOST_IP, &network_event_handler);
    
    // Stop and deinit WiFi
    esp_wifi_stop();
    esp_wifi_deinit();
    
    // Cleanup
    if (network_event_group) {
        vEventGroupDelete(network_event_group);
        network_event_group = NULL;
    }
    
    if (scan_results) {
        free(scan_results);
        scan_results = NULL;
        scan_count = 0;
    }
    
    network_initialized = false;
    ESP_LOGI(TAG, "Network subsystem deinitialized");
    
    return ESP_OK;
}

bool network_is_ready(void) {
    return network_initialized && (sta_netif != NULL);
}

esp_err_t network_enable(void) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Enabling network");
    
    if (current_state == NETWORK_STATE_DISABLED) {
        network_set_state(NETWORK_STATE_DISCONNECTED);
        
        // Update configuration
        app_config_set_wifi_enabled(true);
        app_config_save();
    }
    
    return ESP_OK;
}

esp_err_t network_disable(void) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Disabling network");
    
    // Disconnect if connected
    if (current_state == NETWORK_STATE_CONNECTED || 
        current_state == NETWORK_STATE_CONNECTING ||
        current_state == NETWORK_STATE_RECONNECTING) {
        esp_wifi_disconnect();
    }
    
    network_set_state(NETWORK_STATE_DISABLED);
    
    // Update configuration
    app_config_set_wifi_enabled(false);
    app_config_save();
    
    return ESP_OK;
}

esp_err_t network_connect(const char *ssid, const char *password, uint32_t timeout_ms) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (ssid == NULL) {
        ESP_LOGE(TAG, "SSID cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (current_state == NETWORK_STATE_DISABLED) {
        ESP_LOGW(TAG, "Network is disabled");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Connecting to network: %s", ssid);
    
    // Store credentials
    strncpy(stored_ssid, ssid, sizeof(stored_ssid) - 1);
    stored_ssid[sizeof(stored_ssid) - 1] = '\0';
    
    if (password != NULL) {
        strncpy(stored_password, password, sizeof(stored_password) - 1);
        stored_password[sizeof(stored_password) - 1] = '\0';
    } else {
        stored_password[0] = '\0';
    }
    
    // Use provided timeout or default
    if (timeout_ms > 0) {
        connection_timeout_ms = timeout_ms;
    }
    
    // Configure WiFi
    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    if (password != NULL) {
        strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    }
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    
    // Clear event bits
    xEventGroupClearBits(network_event_group, NETWORK_CONNECTED_BIT | NETWORK_FAIL_BIT);
    
    // Update stats
    stats.connect_attempts++;
    
    // Set state and connect
    network_set_state(NETWORK_STATE_CONNECTING);
    esp_err_t err = esp_wifi_connect();
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start connection: %s", esp_err_to_name(err));
        stats.failed_connections++;
        network_set_state(NETWORK_STATE_FAILED);
        return err;
    }
    
    return ESP_OK;
}

esp_err_t network_disconnect(void) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Disconnecting from network");
    
    // Stop auto-reconnect
    reconnect_attempt_count = auto_reconnect_max_attempts;
    
    if (current_state == NETWORK_STATE_CONNECTED || 
        current_state == NETWORK_STATE_CONNECTING ||
        current_state == NETWORK_STATE_RECONNECTING) {
        esp_wifi_disconnect();
    }
    
    network_set_state(NETWORK_STATE_DISCONNECTED);
    
    return ESP_OK;
}

esp_err_t network_reconnect(void) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Forcing reconnection");
    
    // Reset reconnect attempts
    reconnect_attempt_count = 0;
    
    // Disconnect first if connected
    if (current_state == NETWORK_STATE_CONNECTED) {
        esp_wifi_disconnect();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait a bit
    }
    
    // Reconnect with stored credentials
    return network_connect(stored_ssid, stored_password, connection_timeout_ms);
}

static void network_event_handler(void *arg, esp_event_base_t event_base, 
                                 int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "WiFi station started");
                break;
                
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Connected to access point");
                break;
                
            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t *disconnected = 
                    (wifi_event_sta_disconnected_t *)event_data;
                ESP_LOGW(TAG, "Disconnected from AP, reason: %d", disconnected->reason);
                
                stats.disconnections++;
                stats.last_disconnect_time = esp_timer_get_time() / 1000000; // Convert to seconds
                
                xEventGroupSetBits(network_event_group, NETWORK_FAIL_BIT);
                
                // Handle auto-reconnect
                if (auto_reconnect_enabled && 
                    current_state != NETWORK_STATE_DISABLED &&
                    reconnect_attempt_count < auto_reconnect_max_attempts) {
                    
                    reconnect_attempt_count++;
                    stats.reconnections++;
                    
                    ESP_LOGI(TAG, "Auto-reconnect attempt %lu/%lu in %lu ms", 
                            reconnect_attempt_count, auto_reconnect_max_attempts, 
                            auto_reconnect_delay_ms);
                    
                    network_set_state(NETWORK_STATE_RECONNECTING);
                    
                    // Create reconnect task
                    xTaskCreate(network_auto_reconnect_task, "net_reconnect", 
                               2048, NULL, 5, NULL);
                } else {
                    network_set_state(NETWORK_STATE_FAILED);
                }
                break;
            }
            
            case WIFI_EVENT_SCAN_DONE: {
                wifi_event_sta_scan_done_t *scan_done = 
                    (wifi_event_sta_scan_done_t *)event_data;
                ESP_LOGI(TAG, "WiFi scan done, found %d APs", scan_done->number);
                
                // Get scan results
                uint16_t max_aps = scan_done->number;
                if (max_aps > 0) {
                    wifi_ap_record_t *ap_records = malloc(max_aps * sizeof(wifi_ap_record_t));
                    if (ap_records) {
                        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&max_aps, ap_records));
                        
                        // Convert to our format
                        if (scan_results) {
                            free(scan_results);
                        }
                        scan_results = malloc(max_aps * sizeof(network_ap_info_t));
                        
                        if (scan_results) {
                            scan_count = max_aps;
                            for (int i = 0; i < max_aps; i++) {
                                strncpy(scan_results[i].ssid, (char *)ap_records[i].ssid, 
                                       sizeof(scan_results[i].ssid) - 1);
                                scan_results[i].ssid[sizeof(scan_results[i].ssid) - 1] = '\0';
                                
                                memcpy(scan_results[i].bssid, ap_records[i].bssid, 6);
                                scan_results[i].rssi = ap_records[i].rssi;
                                scan_results[i].channel = ap_records[i].primary;
                                scan_results[i].security = wifi_auth_mode_to_security(ap_records[i].authmode);
                                scan_results[i].is_hidden = (strlen((char *)ap_records[i].ssid) == 0);
                            }
                            
                            // Call callback if registered
                            if (scan_callback) {
                                scan_callback(scan_results, scan_count);
                            }
                        }
                        
                        free(ap_records);
                    }
                }
                
                xEventGroupSetBits(network_event_group, NETWORK_SCAN_DONE_BIT);
                break;
            }
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
            
            stats.successful_connections++;
            stats.last_connect_time = esp_timer_get_time() / 1000000; // Convert to seconds
            reconnect_attempt_count = 0; // Reset reconnect counter
            
            network_update_info();
            network_set_state(NETWORK_STATE_CONNECTED);
            xEventGroupSetBits(network_event_group, NETWORK_CONNECTED_BIT);
            
        } else if (event_id == IP_EVENT_STA_LOST_IP) {
            ESP_LOGW(TAG, "Lost IP address");
            memset(&current_info, 0, sizeof(current_info));
        }
    }
}

static void network_set_state(network_state_t new_state) {
    if (current_state != new_state) {
        network_state_t old_state = current_state;
        current_state = new_state;
        
        ESP_LOGI(TAG, "State changed: %s -> %s", 
                network_state_to_string(old_state),
                network_state_to_string(new_state));
        
        // Call event callback if registered
        if (event_callback) {
            network_info_t *info_ptr = (new_state == NETWORK_STATE_CONNECTED) ? &current_info : NULL;
            event_callback(new_state, info_ptr);
        }
    }
}

static void network_update_info(void) {
    if (!network_is_ready()) {
        return;
    }
    
    memset(&current_info, 0, sizeof(current_info));
    
    // Get WiFi info
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        strncpy(current_info.ssid, (char *)ap_info.ssid, sizeof(current_info.ssid) - 1);
        current_info.ssid[sizeof(current_info.ssid) - 1] = '\0';
        memcpy(current_info.bssid, ap_info.bssid, 6);
        current_info.rssi = ap_info.rssi;
        current_info.channel = ap_info.primary;
    }
    
    // Get IP info
    esp_netif_ip_info_t ip_info;
    if (esp_netif_get_ip_info(sta_netif, &ip_info) == ESP_OK) {
        current_info.ip_addr = ip_info.ip.addr;
        current_info.netmask = ip_info.netmask.addr;
        current_info.gateway = ip_info.gw.addr;
        current_info.dhcp_enabled = true; // Assume DHCP for now
    }
    
    // Get DNS info
    esp_netif_dns_info_t dns_info;
    if (esp_netif_get_dns_info(sta_netif, ESP_NETIF_DNS_MAIN, &dns_info) == ESP_OK) {
        current_info.dns1 = dns_info.ip.u_addr.ip4.addr;
    }
    if (esp_netif_get_dns_info(sta_netif, ESP_NETIF_DNS_BACKUP, &dns_info) == ESP_OK) {
        current_info.dns2 = dns_info.ip.u_addr.ip4.addr;
    }
}

static void network_auto_reconnect_task(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(auto_reconnect_delay_ms));
    
    if (current_state == NETWORK_STATE_RECONNECTING) {
        ESP_LOGI(TAG, "Attempting auto-reconnect...");
        esp_wifi_connect();
    }
    
    vTaskDelete(NULL);
}

static network_security_t wifi_auth_mode_to_security(wifi_auth_mode_t auth_mode) {
    switch (auth_mode) {
        case WIFI_AUTH_OPEN:
            return NETWORK_SECURITY_OPEN;
        case WIFI_AUTH_WEP:
            return NETWORK_SECURITY_WEP;
        case WIFI_AUTH_WPA_PSK:
            return NETWORK_SECURITY_WPA_PSK;
        case WIFI_AUTH_WPA2_PSK:
            return NETWORK_SECURITY_WPA2_PSK;
        case WIFI_AUTH_WPA_WPA2_PSK:
            return NETWORK_SECURITY_WPA_WPA2_PSK;
        case WIFI_AUTH_WPA3_PSK:
            return NETWORK_SECURITY_WPA3_PSK;
        default:
            return NETWORK_SECURITY_UNKNOWN;
    }
}

// Continue in next part due to length...

esp_err_t network_scan_start(network_scan_cb_t callback, bool active_scan) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Starting WiFi scan (%s)", active_scan ? "active" : "passive");
    
    scan_callback = callback;
    
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = active_scan ? WIFI_SCAN_TYPE_ACTIVE : WIFI_SCAN_TYPE_PASSIVE,
        .scan_time.active.min = 120,
        .scan_time.active.max = 200,
        .scan_time.passive = 300
    };
    
    return esp_wifi_scan_start(&scan_config, false);
}

esp_err_t network_scan_stop(void) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Stopping WiFi scan");
    scan_callback = NULL;
    
    return esp_wifi_scan_stop();
}

network_state_t network_get_state(void) {
    return current_state;
}

esp_err_t network_get_info(network_info_t *info) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (info == NULL) {
        ESP_LOGE(TAG, "Info pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (current_state != NETWORK_STATE_CONNECTED) {
        ESP_LOGW(TAG, "Not connected to network");
        return ESP_ERR_INVALID_STATE;
    }
    
    network_update_info();
    memcpy(info, &current_info, sizeof(network_info_t));
    
    return ESP_OK;
}

esp_err_t network_get_stats(network_stats_t *stats_out) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (stats_out == NULL) {
        ESP_LOGE(TAG, "Stats pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Update uptime if connected
    if (current_state == NETWORK_STATE_CONNECTED && stats.last_connect_time > 0) {
        uint32_t current_time = esp_timer_get_time() / 1000000;
        stats.uptime_seconds += (current_time - stats.last_connect_time);
        stats.last_connect_time = current_time;
    }
    
    memcpy(stats_out, &stats, sizeof(network_stats_t));
    
    return ESP_OK;
}

esp_err_t network_reset_stats(void) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Resetting network statistics");
    memset(&stats, 0, sizeof(network_stats_t));
    
    return ESP_OK;
}

esp_err_t network_register_event_callback(network_event_cb_t callback) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    event_callback = callback;
    ESP_LOGI(TAG, "Event callback registered");
    
    return ESP_OK;
}

esp_err_t network_unregister_event_callback(void) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    event_callback = NULL;
    ESP_LOGI(TAG, "Event callback unregistered");
    
    return ESP_OK;
}

esp_err_t network_set_auto_reconnect(bool enabled, uint32_t max_attempts, uint32_t delay_ms) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    auto_reconnect_enabled = enabled;
    auto_reconnect_max_attempts = max_attempts;
    auto_reconnect_delay_ms = delay_ms;
    
    ESP_LOGI(TAG, "Auto-reconnect: %s, max attempts: %lu, delay: %lu ms",
             enabled ? "enabled" : "disabled", max_attempts, delay_ms);
    
    // Save to configuration
    // app_config_set_auto_reconnect(enabled);
    // app_config_save();
    
    return ESP_OK;
}

esp_err_t network_set_static_ip(uint32_t ip_addr, uint32_t netmask, uint32_t gateway, 
                                uint32_t dns1, uint32_t dns2) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (ip_addr == 0) {
        // Enable DHCP
        ESP_LOGI(TAG, "Enabling DHCP");
        return esp_netif_dhcpc_start(sta_netif);
    } else {
        // Set static IP
        ESP_LOGI(TAG, "Setting static IP: " IPSTR, IP2STR((esp_ip4_addr_t*)&ip_addr));
        
        // Stop DHCP client
        esp_netif_dhcpc_stop(sta_netif);
        
        // Set IP info
        esp_netif_ip_info_t ip_info = {
            .ip.addr = ip_addr,
            .netmask.addr = netmask,
            .gw.addr = gateway
        };
        
        esp_err_t err = esp_netif_set_ip_info(sta_netif, &ip_info);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set IP info: %s", esp_err_to_name(err));
            return err;
        }
        
        // Set DNS servers
        if (dns1 != 0) {
            esp_netif_dns_info_t dns_info = {
                .ip.u_addr.ip4.addr = dns1,
                .ip.type = ESP_IPADDR_TYPE_V4
            };
            esp_netif_set_dns_info(sta_netif, ESP_NETIF_DNS_MAIN, &dns_info);
        }
        
        if (dns2 != 0) {
            esp_netif_dns_info_t dns_info = {
                .ip.u_addr.ip4.addr = dns2,
                .ip.type = ESP_IPADDR_TYPE_V4
            };
            esp_netif_set_dns_info(sta_netif, ESP_NETIF_DNS_BACKUP, &dns_info);
        }
    }
    
    return ESP_OK;
}

esp_err_t network_set_dhcp_enabled(bool enabled) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "%s DHCP", enabled ? "Enabling" : "Disabling");
    
    if (enabled) {
        return esp_netif_dhcpc_start(sta_netif);
    } else {
        return esp_netif_dhcpc_stop(sta_netif);
    }
}

esp_err_t network_set_hostname(const char *hostname) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (hostname == NULL) {
        ESP_LOGE(TAG, "Hostname cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Setting hostname: %s", hostname);
    
    return esp_netif_set_hostname(sta_netif, hostname);
}

esp_err_t network_get_hostname(char *hostname, size_t max_len) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (hostname == NULL || max_len == 0) {
        ESP_LOGE(TAG, "Invalid hostname buffer");
        return ESP_ERR_INVALID_ARG;
    }
    
    const char *stored_hostname = NULL;
    esp_err_t err = esp_netif_get_hostname(sta_netif, &stored_hostname);
    
    if (err == ESP_OK && stored_hostname != NULL) {
        strncpy(hostname, stored_hostname, max_len - 1);
        hostname[max_len - 1] = '\0';
    } else {
        hostname[0] = '\0';
    }
    
    return err;
}

esp_err_t network_set_power_save_mode(wifi_ps_type_t mode) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Setting power save mode: %d", mode);
    
    return esp_wifi_set_ps(mode);
}

int8_t network_get_rssi(void) {
    if (!network_is_ready() || current_state != NETWORK_STATE_CONNECTED) {
        return 0;
    }
    
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        return ap_info.rssi;
    }
    
    return 0;
}

bool network_is_connected(void) {
    return (current_state == NETWORK_STATE_CONNECTED);
}

esp_err_t network_get_ip_string(char *ip_str) {
    if (!network_is_ready()) {
        ESP_LOGE(TAG, "Network not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (ip_str == NULL) {
        ESP_LOGE(TAG, "IP string buffer cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (current_state != NETWORK_STATE_CONNECTED) {
        strcpy(ip_str, "0.0.0.0");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_netif_ip_info_t ip_info;
    esp_err_t err = esp_netif_get_ip_info(sta_netif, &ip_info);
    
    if (err == ESP_OK) {
        sprintf(ip_str, IPSTR, IP2STR(&ip_info.ip));
    } else {
        strcpy(ip_str, "0.0.0.0");
    }
    
    return err;
}

const char* network_security_to_string(network_security_t security) {
    switch (security) {
        case NETWORK_SECURITY_OPEN:
            return "Open";
        case NETWORK_SECURITY_WEP:
            return "WEP";
        case NETWORK_SECURITY_WPA_PSK:
            return "WPA";
        case NETWORK_SECURITY_WPA2_PSK:
            return "WPA2";
        case NETWORK_SECURITY_WPA_WPA2_PSK:
            return "WPA/WPA2";
        case NETWORK_SECURITY_WPA3_PSK:
            return "WPA3";
        default:
            return "Unknown";
    }
}

const char* network_state_to_string(network_state_t state) {
    switch (state) {
        case NETWORK_STATE_DISCONNECTED:
            return "Disconnected";
        case NETWORK_STATE_CONNECTING:
            return "Connecting";
        case NETWORK_STATE_CONNECTED:
            return "Connected";
        case NETWORK_STATE_RECONNECTING:
            return "Reconnecting";
        case NETWORK_STATE_FAILED:
            return "Failed";
        case NETWORK_STATE_DISABLED:
            return "Disabled";
        default:
            return "Unknown";
    }
}
