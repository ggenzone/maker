#include "hardware.h"
#include "esp_log.h"
#include "wifi_helper.h"

static const char *TAG = "HARDWARE";

static bool hardware_initialized = false;

esp_err_t hardware_init(void) {
    esp_err_t err;
    
    ESP_LOGI(TAG, "Initializing hardware components");

    err = wifi_helper_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize WiFi helper: %s", esp_err_to_name(err));
        wifi_helper_deinit(); // Cleanup WiFi on failure
        return err;
    }

    // Conectar con credenciales de Kconfig
    wifi_helper_connect(CONFIG_GMAKER_WIFI_SSID, CONFIG_GMAKER_WIFI_PASSWORD);
    
    hardware_initialized = true;
    ESP_LOGI(TAG, "Hardware initialization complete");
    
    return ESP_OK;
}

esp_err_t hardware_deinit(void) {
    if (!hardware_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing hardware components");

    wifi_helper_deinit();
    esp_err_t err = wifi_helper_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to deinitialize WiFi helper: %s", esp_err_to_name(err));
    }


    hardware_initialized = false;
    ESP_LOGI(TAG, "Hardware deinitialization complete");
    
    return ESP_OK;
}

bool hardware_is_ready(void) {
    return hardware_initialized && wifi_helper_is_connected(); 
}
