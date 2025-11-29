#include "hardware.h"
#include "esp_log.h"

static const char *TAG = "HARDWARE";

static bool hardware_initialized = false;

esp_err_t hardware_init(void) {
    esp_err_t ret;
    
    ESP_LOGI(TAG, "Initializing hardware components");

    // Initialize buttons
    ret = button_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to initialize buttons: %s - continuing without them", esp_err_to_name(ret));
        button_deinit();
    } else {
        ESP_LOGI(TAG, "Buttons initialized successfully");
    }

    
    hardware_initialized = true;
    ESP_LOGI(TAG, "Hardware initialization complete");
    
    return ESP_OK;
}

esp_err_t hardware_deinit(void) {
    if (!hardware_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing hardware components");

    button_deinit();

    hardware_initialized = false;
    ESP_LOGI(TAG, "Hardware deinitialization complete");
    
    return ESP_OK;
}

bool hardware_is_ready(void) {
    return hardware_initialized && button_is_ready(); 
}
