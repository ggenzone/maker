#include "hardware.h"
#include "esp_log.h"

static const char *TAG = "HARDWARE";

static bool hardware_initialized = false;

esp_err_t hardware_init(void) {
    esp_err_t ret;
    
    ESP_LOGI(TAG, "Initializing hardware components");

    // Initialize SD card
    ret = sd_spi_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SD card: %s", esp_err_to_name(ret));
        sd_spi_deinit(); // Cleanup SD card on failure
        return ret;
    }
    
    hardware_initialized = true;
    ESP_LOGI(TAG, "Hardware initialization complete");
    SD_LOGI(TAG, "Hardware initialization complete");
    
    return ESP_OK;
}

esp_err_t hardware_deinit(void) {
    if (!hardware_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing hardware components");
    
    sd_spi_deinit();
    
    hardware_initialized = false;
    ESP_LOGI(TAG, "Hardware deinitialization complete");
    
    return ESP_OK;
}

bool hardware_is_ready(void) {
    return hardware_initialized && sd_spi_is_ready();
}
