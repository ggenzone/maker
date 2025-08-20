#include "gui_screen_wifi.h"
#include "../navigation/gui_navigator.h"
#include "../widgets/gui_widget_header.h"
#include "../gui_common.h"
#include "../gui_styles.h"
#include "network/network.h"
#include "network/network_config.h"
#include "storage/app_config.h"
#include "esp_log.h"

static const char *TAG = "GUI_SCREEN_WIFI";

// Static references to UI elements that need to be updated
static lv_obj_t *wifi_toggle = NULL;

static void wifi_toggle_event_handler(lv_event_t *e);

void gui_screen_wifi_show(void *user_data) {
    ESP_LOGI(TAG, "Showing WiFi settings screen");
    
    // Create main container
    lv_obj_t *container = gui_create_clean_container(lv_scr_act());
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_center(container);
    
    // Configure flex layout vertical
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);

    // Header with back button
    gui_widget_header_create_with_back(container, "WiFi Settings", false);

    // WiFi Enable/Disable Container
    lv_obj_t *wifi_container = gui_create_clean_container(container);
    lv_obj_set_size(wifi_container, lv_pct(100), 60);
    lv_obj_clear_flag(wifi_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(wifi_container, 15, 0);
    
    // WiFi Label
    lv_obj_t *wifi_label = lv_label_create(wifi_container);
    lv_label_set_text(wifi_label, "Enable WiFi");
    lv_obj_align(wifi_label, LV_ALIGN_LEFT_MID, 0, 0);

    // WiFi Toggle
    wifi_toggle = lv_switch_create(wifi_container);
    lv_obj_align(wifi_toggle, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(wifi_toggle, wifi_toggle_event_handler, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Initialize switch value
    if (app_config_get_wifi_enabled()) {
        lv_obj_add_state(wifi_toggle, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(wifi_toggle, LV_STATE_CHECKED);
    }

    // WiFi Networks item (placeholder for future implementation)
    lv_obj_t *wifi_networks_item = gui_create_setting_item(container, "WiFi Networks", LV_SYMBOL_WIFI);
    // TODO: Add event handler when network selection is implemented

    // Placeholder for future WiFi functions
    lv_obj_t *placeholder = lv_label_create(container);
    lv_label_set_text(placeholder, "WiFi scan and connect\ncoming soon...");
    lv_obj_set_style_text_align(placeholder, LV_TEXT_ALIGN_CENTER, 0);
    
    ESP_LOGI(TAG, "WiFi settings screen displayed");
}

static void wifi_toggle_event_handler(lv_event_t *e) {
    lv_obj_t *sw = lv_event_get_target(e);
    bool on = lv_obj_has_state(sw, LV_STATE_CHECKED);

    ESP_LOGI(TAG, "WiFi toggle: %s", on ? "ON" : "OFF");
    
    if (on) {
        network_enable();

        // Add credentials to configuration with auto-connect and high priority
        esp_err_t err = network_config_add_credentials(CONFIG_GMAKER_WIFI_SSID, CONFIG_GMAKER_WIFI_PASSWORD, true, 10);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to add credentials: %s", esp_err_to_name(err));
            return;
        }
        
        // Save network configuration
        err = network_config_save();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save network config: %s", esp_err_to_name(err));
        }
        
        // Connect using network system
        network_connect(CONFIG_GMAKER_WIFI_SSID, CONFIG_GMAKER_WIFI_PASSWORD, 0);

    } else {
        network_disable();
    }
    
}

