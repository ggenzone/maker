#include "lvgl_helper.h"

#include "gui/gui.h"
#include "hardware/hardware.h"
#include "storage/storage.h"
#include "storage/app_config.h"
#include "network/network.h"
#include "network/network_config.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <stdbool.h>

static const char *TAG = "MAIN";
// typedef void (*wifi_status_cb_t)(bool connected);

// Network event callback for GUI updates
static void network_event_callback(network_state_t state, const network_info_t *info) {
    // Update app state for backward compatibility
    bool wifi_connected = (state == NETWORK_STATE_CONNECTED);

    app_config_set_wifi_enabled(wifi_connected);
    esp_err_t err = app_config_save();
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save app state: %s", esp_err_to_name(err));
    }
    // Update GUI (if the function exists)
    // gui_update_wifi_status(app_state.wifi_connected);
}

void app_main(void)
{
    // Initialize storage system first
    ESP_ERROR_CHECK(storage_init());
    ESP_ERROR_CHECK(app_config_init());
    ESP_ERROR_CHECK(app_config_load());

    // Initialize network configuration
    ESP_ERROR_CHECK(network_config_init());
    ESP_ERROR_CHECK(network_config_load());

    // Inicializar todo el hardware
    ESP_ERROR_CHECK(hardware_init());
    

    lvgl_task_init();

    // Initialize new network system
    ESP_ERROR_CHECK(network_init());
    network_register_event_callback(network_event_callback);

    // Enable network if WiFi was enabled
    if (app_config_get_wifi_enabled()) {
        network_enable();
        
        // Try to connect with stored credentials if available
        network_credential_t credential;
        if (network_config_find_best_credentials(&credential) == ESP_OK) {
            network_connect(credential.ssid, credential.password, 0);
        }
    }

    if (hardware_is_ready()) {
        begin_lvgl_procedure();

        gui_init();

        //network_register_status_callback(gui_update_wifi_status);

        end_lvgl_procedure();
    }
}
