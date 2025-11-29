#include "system_init.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

static const char *TAG = "system";

esp_err_t system_init(void) {
    ESP_LOGI(TAG, "Initializing system...");
    
    // 1. Initialize NVS (required for WiFi, Bluetooth, and storing configurations)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated or needs to be erased");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized");
    
    // 2. Initialize event loop (required for WiFi, Bluetooth, and other drivers)
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_LOGI(TAG, "Event loop created");
    
    // 3. Initialize network interface (required for WiFi/Ethernet)
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_LOGI(TAG, "Network interface initialized");
    
    // 4. Display chip information (useful for debugging)
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "ESP32 chip info:");
    ESP_LOGI(TAG, "  - Cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "  - Features: WiFi%s%s",
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    ESP_LOGI(TAG, "  - Silicon revision: %d", chip_info.revision);
    
    uint32_t flash_size;
    if (esp_flash_get_size(NULL, &flash_size) == ESP_OK) {
        ESP_LOGI(TAG, "  - Flash size: %lu MB", flash_size / (1024 * 1024));
    }
    
    ESP_LOGI(TAG, "System initialization complete");
    return ESP_OK;
}
