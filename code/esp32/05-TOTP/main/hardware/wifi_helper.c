#include "wifi_helper.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "wifi_helper";
static bool wifi_ready = false;
static bool wifi_connected = false;
static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group = NULL;

// Event handler for WiFi and IP events
static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WiFi started, attempting to connect");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_connected = false;
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry to connect to the AP (attempt %d/%d)", s_retry_num, WIFI_MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Failed to connect to WiFi after %d attempts", WIFI_MAXIMUM_RETRY);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        wifi_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_helper_init(void) {
    if (wifi_ready) {
        ESP_LOGW(TAG, "WiFi helper already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing WiFi helper");

    // Create event group for WiFi events
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Failed to create event group");
        return ESP_FAIL;
    }

    // Initialize TCP/IP network interface
    esp_err_t err = esp_netif_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_netif_init failed: %s", esp_err_to_name(err));
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
        return err;
    }

    // Create default event loop
    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_event_loop_create_default failed: %s", esp_err_to_name(err));
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
        return err;
    }

    // Create default WiFi station interface
    esp_netif_create_default_wifi_sta();

    // Initialize WiFi with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init failed: %s", esp_err_to_name(err));
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
        return err;
    }

    // Register event handlers
    err = esp_event_handler_instance_register(WIFI_EVENT,
                                              ESP_EVENT_ANY_ID,
                                              &event_handler,
                                              NULL,
                                              NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register WiFi event handler: %s", esp_err_to_name(err));
        esp_wifi_deinit();
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
        return err;
    }

    err = esp_event_handler_instance_register(IP_EVENT,
                                              IP_EVENT_STA_GOT_IP,
                                              &event_handler,
                                              NULL,
                                              NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register IP event handler: %s", esp_err_to_name(err));
        esp_wifi_deinit();
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
        return err;
    }

    // Set WiFi mode to station
    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode failed: %s", esp_err_to_name(err));
        esp_wifi_deinit();
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
        return err;
    }

    wifi_ready = true;
    ESP_LOGI(TAG, "WiFi helper initialized successfully");
    return ESP_OK;
}

esp_err_t wifi_helper_deinit(void) {
    if (!wifi_ready) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing WiFi helper");

    // Stop WiFi
    esp_wifi_stop();
    
    // Unregister event handlers
    esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler);
    esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler);
    
    // Deinitialize WiFi
    esp_err_t err = esp_wifi_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_deinit failed: %s", esp_err_to_name(err));
    }

    // Delete event group
    if (s_wifi_event_group != NULL) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }

    wifi_ready = false;
    wifi_connected = false;
    s_retry_num = 0;

    ESP_LOGI(TAG, "WiFi helper deinitialized successfully");
    return err;
}

bool wifi_helper_is_ready(void) {
    return wifi_ready;
}

bool wifi_helper_is_connected(void) {
    return wifi_connected;
}

esp_err_t wifi_helper_connect(const char *ssid, const char *password) {
    if (!wifi_ready) {
        ESP_LOGE(TAG, "WiFi helper not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    // Use default credentials if not provided
    const char *use_ssid = ssid ? ssid : WIFI_SSID;
    const char *use_password = password ? password : WIFI_PASS;

    ESP_LOGI(TAG, "Connecting to WiFi SSID: %s", use_ssid);

    // Configure WiFi
    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, use_ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, use_password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config failed: %s", esp_err_to_name(err));
        return err;
    }

    // Start WiFi
    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start failed: %s", esp_err_to_name(err));
        return err;
    }

    // Wait for connection or failure
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    // Clear the bits
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to WiFi SSID: %s", use_ssid);
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to WiFi SSID: %s", use_ssid);
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "Unexpected WiFi connection error");
        return ESP_FAIL;
    }
}
