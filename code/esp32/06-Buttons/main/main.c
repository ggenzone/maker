#include <stdio.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hardware/hardware.h"
#include "hardware/button_helper.h"


static const char *TAG = "MAIN";

void app_main(void) {

    // Inicializar todo el hardware
    ESP_ERROR_CHECK(hardware_init());

    
    if (hardware_is_ready()) {
        ESP_LOGI(TAG, "Hardware initialized successfully");
        
        while (true) {
            ESP_LOGI(TAG, "System running... Time: %lu s", (unsigned long)(xTaskGetTickCount() / 1000));
            vTaskDelay(pdMS_TO_TICKS(5000));
        }

        
        ESP_ERROR_CHECK(hardware_deinit());
    } else {
        ESP_LOGE(TAG, "Hardware initialization failed");
    }
}