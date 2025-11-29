#include <stdio.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "core/system_init.h"
#include "bus/i2c_helper.h"
#include "drivers/display/oled_helper.h"
#include "drivers/input/button_helper.h"


static const char *TAG = "MAIN";

void app_main(void) {

    // 1. Initialize core system (NVS, event loop, network interface)
    ESP_LOGI(TAG, "=== Initializing Core System ===");
    ESP_ERROR_CHECK(system_init());

    // 2. Initialize communication buses
    ESP_LOGI(TAG, "=== Initializing Communication Buses ===");
    ESP_ERROR_CHECK(i2c_helper_init());
    
    // 3. Initialize drivers
    ESP_LOGI(TAG, "=== Initializing Drivers ===");
    
    // Initialize buttons
    esp_err_t ret = button_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to initialize buttons: %s - continuing without them", esp_err_to_name(ret));
        button_deinit();
    } else {
        ESP_LOGI(TAG, "Buttons initialized successfully");
    }
    
    // Initialize OLED display (optional - won't fail if not connected)
    ret = oled_init();
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGW(TAG, "OLED display not found - continuing without it");
        } else {
            ESP_LOGW(TAG, "Failed to initialize OLED: %s - continuing without it", esp_err_to_name(ret));
        }
        oled_deinit();
    } else {
        ESP_LOGI(TAG, "OLED display initialized successfully");
    }

    // 4. Start application logic
    ESP_LOGI(TAG, "=== Starting Application ===");
    
    if (oled_is_ready()) {
        ESP_LOGI(TAG, "OLED is ready");
        
        // Mostrar mensaje de bienvenida
        oled_print_line(0, "ESP32 OLED Test");
        oled_print_line(1, "SSD1306/SSD1315");
        oled_print_line(3, "Inicializando...");
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        // Limpiar pantalla
        oled_clear();
        
        int counter = 0;
        
        while (true) {
            char buffer[32];
            
            // Mostrar información
            oled_print_line(0, "ESP32 I2C Demo");
            oled_print_line(1, "----------------");
            
            snprintf(buffer, sizeof(buffer), "Counter: %d", counter);
            oled_print_line(3, buffer);
            

            snprintf(buffer, sizeof(buffer), "Time: %lu s", (unsigned long)(xTaskGetTickCount() / 1000));
            oled_print_line(4, buffer);
            
            
            oled_print_line(6, "Hardware OK!");
            
            counter++;
            
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        
        // Cleanup (won't reach here in this example)
        oled_deinit();
        button_deinit();
        i2c_helper_deinit();
    } else {
        ESP_LOGE(TAG, "OLED initialization failed");
    }
}