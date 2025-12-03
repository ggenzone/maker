#include <stdio.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "core/system_init.h"
#include "bus/i2c_helper.h"
#include "drivers/display/oled_helper.h"
#include "app/tasks/button_task.h"

static const char *TAG = "MAIN";

// Callback de aplicación para botones
void on_button_pressed(uint8_t button_num) {
    ESP_LOGI(TAG, "=== APP: Button %d pressed ===", button_num);
    
    if (oled_is_ready()) {
        char msg[32];
        snprintf(msg, sizeof(msg), "BTN %d pressed!", button_num);
        oled_print_line(5, msg);
    }
}

void app_main(void) {

    // 1. Initialize core system (NVS, event loop, network interface)
    ESP_LOGI(TAG, "=== Initializing Core System ===");
    ESP_ERROR_CHECK(system_init());

    // 2. Initialize communication buses
    ESP_LOGI(TAG, "=== Initializing Communication Buses ===");
    ESP_ERROR_CHECK(i2c_helper_init());
    
    // 3. Initialize drivers
    ESP_LOGI(TAG, "=== Initializing Drivers ===");
    
    // Initialize OLED display (optional - won't fail if not connected)
    esp_err_t ret = oled_init();
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

    // 4. Initialize application tasks
    ESP_LOGI(TAG, "=== Initializing Application Tasks ===");
    ESP_ERROR_CHECK(button_task_init());
    button_task_register_callback(on_button_pressed);

    // 5. Start application logic
    ESP_LOGI(TAG, "=== Starting Application ===");
    
    if (oled_is_ready()) {
        oled_print_line(0, "ESP32 Ready");
        oled_print_line(1, "Press buttons!");
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        int counter = 0;
        
        while (true) {
            char buffer[32];
            
            snprintf(buffer, sizeof(buffer), "Counter: %d", counter++);
            oled_print_line(3, buffer);
            
            snprintf(buffer, sizeof(buffer), "Time: %lu s", (unsigned long)(xTaskGetTickCount() / 1000));
            oled_print_line(4, buffer);
            
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    } else {
        ESP_LOGW(TAG, "OLED not available - running without display");
        
        // Loop simple sin display
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}