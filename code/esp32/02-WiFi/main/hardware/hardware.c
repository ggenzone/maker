#include "hardware.h"
#include "esp_log.h"

static const char *TAG = "HARDWARE";

static bool hardware_initialized = false;

esp_err_t hardware_init(void) {
    ESP_LOGI(TAG, "Initializing hardware components");
    
    // Initialize LCD first
    esp_err_t ret = lcd_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LCD: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Initialize touch controller
    ret = lcd_touch_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize touch controller: %s", esp_err_to_name(ret));
        lcd_deinit(); // Cleanup LCD on failure
        return ret;
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
    
    touch_deinit();
    lcd_deinit();
    
    hardware_initialized = false;
    ESP_LOGI(TAG, "Hardware deinitialization complete");
    
    return ESP_OK;
}

bool hardware_is_ready(void) {
    return hardware_initialized && lcd_is_ready() && touch_is_ready();
}
