#include <stdio.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hardware/hardware.h"
#include "network/server.h"
#include "totp/totp_storage.h"
#include "utils/ntp.h"


static const char *TAG = "MAIN";

void app_main(void) {
    // Inicializar TOTP storage (inicializa NVS internamente)
    ESP_ERROR_CHECK(totp_storage_init());

    // Inicializar todo el hardware
    ESP_ERROR_CHECK(hardware_init());

    
    if (hardware_is_ready()) {
        ntp_sync();
        ESP_LOGI(TAG, "Hardware initialized successfully");
        
        // Iniciar servidor web
        esp_err_t ret = server_init();
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Web server started successfully");
            ESP_LOGI(TAG, "TOTP services loaded: %d", totp_storage_count());
        } else {
            ESP_LOGE(TAG, "Failed to start web server: %s", esp_err_to_name(ret));
        }
        
        while (true) {

            ESP_LOGI(TAG, "Server running on http://[ESP32_IP]/");

            vTaskDelay(pdMS_TO_TICKS(5000)); // Esperar 5 segundos
        }
        
        server_deinit();
        ESP_ERROR_CHECK(hardware_deinit());
        totp_storage_deinit();
    } else {
        ESP_LOGE(TAG, "Hardware initialization failed");
    }
}