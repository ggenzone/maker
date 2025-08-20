#include "gui_screen_settings.h"
#include "../navigation/gui_navigator.h"
#include "../widgets/gui_widget_header.h"
#include "../gui_common.h"
#include "../gui_styles.h"
#include "esp_log.h"

static const char *TAG = "GUI_SCREEN_SETTINGS";

static void general_settings_btn_clicked(lv_event_t *e) {
    gui_navigate_to(GUI_SCREEN_GENERAL, NULL);
}

static void wifi_settings_btn_clicked(lv_event_t *e) {
    gui_navigate_to(GUI_SCREEN_WIFI, NULL);
}

static void system_settings_btn_clicked(lv_event_t *e) {
    gui_navigate_to(GUI_SCREEN_SETTINGS_SYSTEM, NULL);
}

static void back_btn_clicked(lv_event_t *e) {
    gui_navigate_back();
}

void gui_screen_settings_show(void *user_data) {
    ESP_LOGI(TAG, "Showing settings screen");
    
    // Create main container
    lv_obj_t *container = gui_create_clean_container(lv_scr_act());
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_center(container);
    
    // Configure flex layout vertical
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);

    // Header with back button (no WiFi icon on settings)
    gui_widget_header_create_with_back(container, "Settings", false);

    // General Settings item
    lv_obj_t *general_item = gui_create_setting_item(container, "General", LV_SYMBOL_SETTINGS);
    lv_obj_add_event_cb(general_item, general_settings_btn_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(general_item, LV_OBJ_FLAG_CLICKABLE);

    // WiFi Settings item
    lv_obj_t *wifi_item = gui_create_setting_item(container, "WiFi", LV_SYMBOL_WIFI);
    lv_obj_add_event_cb(wifi_item, wifi_settings_btn_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(wifi_item, LV_OBJ_FLAG_CLICKABLE);

    // System Settings item
    lv_obj_t *system_item = gui_create_setting_item(container, "System", LV_SYMBOL_LIST);
    lv_obj_add_event_cb(system_item, system_settings_btn_clicked, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(system_item, LV_OBJ_FLAG_CLICKABLE);

    // Spacer to push back button down
    lv_obj_t *spacer = gui_create_clean_container(container);
    lv_obj_set_size(spacer, 1, 1);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);

    // Back Button - Use navigation back function
    gui_create_back_button(container, back_btn_clicked);
    
    ESP_LOGI(TAG, "Settings screen displayed");
}
