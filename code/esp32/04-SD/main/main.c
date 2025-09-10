#include <stdio.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_log.h"
#include "hardware/hardware.h"


static const char *TAG = "DataLogger";

void app_main(void) {

    // Inicializar todo el hardware
    ESP_ERROR_CHECK(hardware_init());


    if (hardware_is_ready()) {

        SD_LOGI(TAG, "Main loop started");

        ESP_ERROR_CHECK(hardware_deinit());
    }
}