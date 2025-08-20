#include "gui_screen_manager.h"
#include "../screens/gui_screen_main.h"
#include "../screens/gui_screen_settings.h"
#include "../screens/gui_screen_general.h"
#include "../screens/gui_screen_wifi.h"
#include "../screens/gui_screen_settings_system.h"
#include "esp_log.h"

static const char *TAG = "GUI_SCREEN_MANAGER";

void gui_screen_manager_init(void) {
    ESP_LOGI(TAG, "Screen manager initialized");
}

void gui_screen_manager_show(gui_screen_id_t screen_id, void *user_data) {
    // Clear current screen
    lv_obj_clean(lv_scr_act());
    
    // Show requested screen
    switch (screen_id) {
        case GUI_SCREEN_MAIN:
            gui_screen_main_show(user_data);
            break;
        case GUI_SCREEN_SETTINGS:
            gui_screen_settings_show(user_data);
            break;
        case GUI_SCREEN_GENERAL:
            gui_screen_general_show(user_data);
            break;
        case GUI_SCREEN_WIFI:
            gui_screen_wifi_show(user_data);
            break;
        case GUI_SCREEN_WIFI_INFO:
            // TODO: Implement wifi info screen
            ESP_LOGW(TAG, "WiFi info screen not implemented yet");
            gui_screen_main_show(NULL);
            break;
        case GUI_SCREEN_SETTINGS_SYSTEM:
            gui_screen_settings_system_show(user_data);
            break;
        default:
            ESP_LOGE(TAG, "Unknown screen ID: %d", screen_id);
            gui_screen_main_show(NULL); // Fallback to main
            break;
    }
}
