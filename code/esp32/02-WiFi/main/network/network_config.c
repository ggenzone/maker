#include "network_config.h"
#include "storage/storage.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "NETWORK_CONFIG";

// Storage keys
#define KEY_NET_PROFILE         "net_profile"
#define KEY_NET_CREDENTIALS     "net_creds"

// Default network profile
static const network_profile_t default_profile = {
    .connection_timeout_ms = 15000,
    .scan_timeout_ms = 10000,
    .auto_reconnect = true,
    .reconnect_max_attempts = 5,
    .reconnect_delay_ms = 5000,
    .power_save_mode = 1,  // WIFI_PS_MIN_MODEM
    .sleep_on_idle = false,
    .idle_timeout_ms = 300000, // 5 minutes
    .use_static_ip = false,
    .static_ip = 0,
    .static_netmask = 0,
    .static_gateway = 0,
    .static_dns1 = 0,
    .static_dns2 = 0,
    .hostname = "esp32-device",
    .channel_preference = 0,
    .fast_scan = true,
    .pmf_required = false,
    .credential_count = 0
};

// Current profile
static network_profile_t current_profile;
static bool config_initialized = false;

esp_err_t network_config_init(void) {
    ESP_LOGI(TAG, "Initializing network configuration");
    
    if (config_initialized) {
        ESP_LOGW(TAG, "Network config already initialized");
        return ESP_OK;
    }
    
    // Copy default profile
    memcpy(&current_profile, &default_profile, sizeof(network_profile_t));
    
    config_initialized = true;
    ESP_LOGI(TAG, "Network configuration initialized");
    
    return ESP_OK;
}

esp_err_t network_config_deinit(void) {
    if (!config_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing network configuration");
    config_initialized = false;
    
    return ESP_OK;
}

esp_err_t network_config_load(void) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Loading network configuration from storage");
    
    // Load profile from storage
    size_t actual_size = 0;
    esp_err_t err = storage_get_blob(KEY_NET_PROFILE, &current_profile, 
                                    sizeof(network_profile_t), &actual_size);
    
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No stored profile found, using defaults");
        memcpy(&current_profile, &default_profile, sizeof(network_profile_t));
        return ESP_OK;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load profile: %s", esp_err_to_name(err));
        memcpy(&current_profile, &default_profile, sizeof(network_profile_t));
        return err;
    }
    
    // Validate loaded profile
    if (actual_size != sizeof(network_profile_t)) {
        ESP_LOGW(TAG, "Profile size mismatch, using defaults");
        memcpy(&current_profile, &default_profile, sizeof(network_profile_t));
        return ESP_ERR_INVALID_SIZE;
    }
    
    ESP_LOGI(TAG, "Network configuration loaded successfully");
    ESP_LOGI(TAG, "  Connection timeout: %lu ms", current_profile.connection_timeout_ms);
    ESP_LOGI(TAG, "  Auto-reconnect: %s", current_profile.auto_reconnect ? "yes" : "no");
    ESP_LOGI(TAG, "  Stored credentials: %u", current_profile.credential_count);
    ESP_LOGI(TAG, "  Hostname: %s", current_profile.hostname);
    
    return ESP_OK;
}

esp_err_t network_config_save(void) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Saving network configuration to storage");
    
    esp_err_t err = storage_set_blob(KEY_NET_PROFILE, &current_profile, 
                                    sizeof(network_profile_t));
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save profile: %s", esp_err_to_name(err));
        return err;
    }
    
    // Commit changes
    err = storage_commit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to commit profile: %s", esp_err_to_name(err));
        return err;
    }
    
    ESP_LOGI(TAG, "Network configuration saved successfully");
    return ESP_OK;
}

esp_err_t network_config_reset_to_defaults(void) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Resetting network configuration to factory defaults");
    
    // Reset to default profile
    memcpy(&current_profile, &default_profile, sizeof(network_profile_t));
    
    // Save to storage
    return network_config_save();
}

esp_err_t network_config_get_profile(network_profile_t *profile) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (profile == NULL) {
        ESP_LOGE(TAG, "Profile pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(profile, &current_profile, sizeof(network_profile_t));
    return ESP_OK;
}

esp_err_t network_config_set_profile(const network_profile_t *profile) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (profile == NULL) {
        ESP_LOGE(TAG, "Profile pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&current_profile, profile, sizeof(network_profile_t));
    return ESP_OK;
}

esp_err_t network_config_add_credentials(const char *ssid, const char *password, 
                                        bool auto_connect, int8_t priority) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (ssid == NULL) {
        ESP_LOGE(TAG, "SSID cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Adding credentials for SSID: %s", ssid);
    
    // Check if SSID already exists
    int existing_index = -1;
    for (int i = 0; i < current_profile.credential_count; i++) {
        if (strcmp(current_profile.credentials[i].ssid, ssid) == 0) {
            existing_index = i;
            break;
        }
    }
    
    network_credential_t *cred;
    
    if (existing_index >= 0) {
        // Update existing credentials
        ESP_LOGI(TAG, "Updating existing credentials for SSID: %s", ssid);
        cred = &current_profile.credentials[existing_index];
    } else {
        // Add new credentials
        if (current_profile.credential_count >= 8) {
            ESP_LOGW(TAG, "Maximum credentials reached, removing oldest");
            // Remove oldest (first) credential and shift others
            for (int i = 0; i < 7; i++) {
                memcpy(&current_profile.credentials[i], &current_profile.credentials[i + 1], 
                       sizeof(network_credential_t));
            }
            current_profile.credential_count = 7;
        }
        
        cred = &current_profile.credentials[current_profile.credential_count];
        current_profile.credential_count++;
    }
    
    // Set credential data
    strncpy(cred->ssid, ssid, sizeof(cred->ssid) - 1);
    cred->ssid[sizeof(cred->ssid) - 1] = '\0';
    
    if (password != NULL) {
        strncpy(cred->password, password, sizeof(cred->password) - 1);
        cred->password[sizeof(cred->password) - 1] = '\0';
        cred->security = NETWORK_SECURITY_WPA2_PSK; // Assume WPA2 for password networks
    } else {
        cred->password[0] = '\0';
        cred->security = NETWORK_SECURITY_OPEN;
    }
    
    cred->auto_connect = auto_connect;
    cred->priority = priority;
    cred->last_used = esp_timer_get_time() / 1000000; // Current time in seconds
    
    ESP_LOGI(TAG, "Credentials added: SSID=%s, auto_connect=%s, priority=%d", 
             ssid, auto_connect ? "yes" : "no", priority);
    
    return ESP_OK;
}

esp_err_t network_config_remove_credentials(const char *ssid) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (ssid == NULL) {
        ESP_LOGE(TAG, "SSID cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Removing credentials for SSID: %s", ssid);
    
    // Find credential index
    int index = -1;
    for (int i = 0; i < current_profile.credential_count; i++) {
        if (strcmp(current_profile.credentials[i].ssid, ssid) == 0) {
            index = i;
            break;
        }
    }
    
    if (index < 0) {
        ESP_LOGW(TAG, "Credentials not found for SSID: %s", ssid);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Shift remaining credentials
    for (int i = index; i < current_profile.credential_count - 1; i++) {
        memcpy(&current_profile.credentials[i], &current_profile.credentials[i + 1], 
               sizeof(network_credential_t));
    }
    
    current_profile.credential_count--;
    
    ESP_LOGI(TAG, "Credentials removed for SSID: %s", ssid);
    return ESP_OK;
}

esp_err_t network_config_get_credentials(const char *ssid, network_credential_t *credential) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (ssid == NULL || credential == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    for (int i = 0; i < current_profile.credential_count; i++) {
        if (strcmp(current_profile.credentials[i].ssid, ssid) == 0) {
            memcpy(credential, &current_profile.credentials[i], sizeof(network_credential_t));
            return ESP_OK;
        }
    }
    
    return ESP_ERR_NOT_FOUND;
}

esp_err_t network_config_get_all_credentials(network_credential_t *credentials, 
                                           uint8_t max_count, uint8_t *actual_count) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (credentials == NULL || actual_count == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t count = (current_profile.credential_count < max_count) ? 
                    current_profile.credential_count : max_count;
    
    for (int i = 0; i < count; i++) {
        memcpy(&credentials[i], &current_profile.credentials[i], sizeof(network_credential_t));
    }
    
    *actual_count = count;
    return ESP_OK;
}

esp_err_t network_config_find_best_credentials(network_credential_t *credential) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (credential == NULL) {
        ESP_LOGE(TAG, "Credential pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    network_credential_t *best = NULL;
    int8_t best_priority = INT8_MIN;
    uint32_t best_last_used = 0;
    
    // Find auto-connect credential with highest priority
    for (int i = 0; i < current_profile.credential_count; i++) {
        if (current_profile.credentials[i].auto_connect) {
            if (current_profile.credentials[i].priority > best_priority ||
                (current_profile.credentials[i].priority == best_priority && 
                 current_profile.credentials[i].last_used > best_last_used)) {
                best = &current_profile.credentials[i];
                best_priority = current_profile.credentials[i].priority;
                best_last_used = current_profile.credentials[i].last_used;
            }
        }
    }
    
    if (best != NULL) {
        memcpy(credential, best, sizeof(network_credential_t));
        return ESP_OK;
    }
    
    return ESP_ERR_NOT_FOUND;
}

esp_err_t network_config_update_last_used(const char *ssid) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (ssid == NULL) {
        ESP_LOGE(TAG, "SSID cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    for (int i = 0; i < current_profile.credential_count; i++) {
        if (strcmp(current_profile.credentials[i].ssid, ssid) == 0) {
            current_profile.credentials[i].last_used = esp_timer_get_time() / 1000000;
            ESP_LOGD(TAG, "Updated last used time for SSID: %s", ssid);
            return ESP_OK;
        }
    }
    
    return ESP_ERR_NOT_FOUND;
}

esp_err_t network_config_clear_all_credentials(void) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Network config not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Clearing all stored credentials");
    
    current_profile.credential_count = 0;
    memset(current_profile.credentials, 0, sizeof(current_profile.credentials));
    
    return ESP_OK;
}

// Individual getters and setters
uint32_t network_config_get_connection_timeout(void) {
    return current_profile.connection_timeout_ms;
}

esp_err_t network_config_set_connection_timeout(uint32_t timeout_ms) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    current_profile.connection_timeout_ms = timeout_ms;
    return ESP_OK;
}

bool network_config_get_auto_reconnect(void) {
    return current_profile.auto_reconnect;
}

esp_err_t network_config_set_auto_reconnect(bool enabled) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    current_profile.auto_reconnect = enabled;
    return ESP_OK;
}

uint8_t network_config_get_power_save_mode(void) {
    return current_profile.power_save_mode;
}

esp_err_t network_config_set_power_save_mode(uint8_t mode) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    current_profile.power_save_mode = mode;
    return ESP_OK;
}

bool network_config_get_use_static_ip(void) {
    return current_profile.use_static_ip;
}

esp_err_t network_config_set_use_static_ip(bool enabled) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    current_profile.use_static_ip = enabled;
    return ESP_OK;
}

esp_err_t network_config_get_static_ip_info(uint32_t *ip, uint32_t *netmask, uint32_t *gateway,
                                           uint32_t *dns1, uint32_t *dns2) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    
    if (ip) *ip = current_profile.static_ip;
    if (netmask) *netmask = current_profile.static_netmask;
    if (gateway) *gateway = current_profile.static_gateway;
    if (dns1) *dns1 = current_profile.static_dns1;
    if (dns2) *dns2 = current_profile.static_dns2;
    
    return ESP_OK;
}

esp_err_t network_config_set_static_ip_info(uint32_t ip, uint32_t netmask, uint32_t gateway,
                                           uint32_t dns1, uint32_t dns2) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    
    current_profile.static_ip = ip;
    current_profile.static_netmask = netmask;
    current_profile.static_gateway = gateway;
    current_profile.static_dns1 = dns1;
    current_profile.static_dns2 = dns2;
    
    return ESP_OK;
}

esp_err_t network_config_get_hostname(char *hostname, size_t max_len) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    
    if (hostname == NULL || max_len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    
    strncpy(hostname, current_profile.hostname, max_len - 1);
    hostname[max_len - 1] = '\0';
    
    return ESP_OK;
}

esp_err_t network_config_set_hostname(const char *hostname) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    
    if (hostname == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    strncpy(current_profile.hostname, hostname, sizeof(current_profile.hostname) - 1);
    current_profile.hostname[sizeof(current_profile.hostname) - 1] = '\0';
    
    return ESP_OK;
}
