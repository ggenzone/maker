#include "lvgl_helper.h"

#include "wifi.h"
#include "gui/gui.h"
#include "app_state.h"
#include "hardware/hardware.h"

#include "nvs_flash.h"

// static const char *TAG = "MAIN";



void app_main(void)
{

    // Inicializar todo el hardware
    ESP_ERROR_CHECK(hardware_init());
    
    // Usar funciones específicas
    lcd_set_brightness(75);

    lvgl_task_init();

    // Init NVS (requerido por WiFi y app_state)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    app_state_load();

    wifi_init();



    if (hardware_is_ready()) {
        begin_lvgl_procedure();

        gui_init();

        wifi_register_status_callback(gui_update_wifi_status);

        end_lvgl_procedure();
    }
}
