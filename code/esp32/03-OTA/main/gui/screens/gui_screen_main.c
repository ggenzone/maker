#include "gui_screen_main.h"
#include "../navigation/gui_navigator.h"
#include "../widgets/gui_widget_header.h"
#include "../gui_common.h"
#include "../gui_styles.h"
#include "esp_log.h"

static const char *TAG = "GUI_SCREEN_MAIN";

static void settings_btn_clicked(lv_event_t *e) {
    gui_navigate_to(GUI_SCREEN_SETTINGS, NULL);
}

void gui_screen_main_show(void *user_data) {
    ESP_LOGI(TAG, "Showing main screen");
    
    // Create main container
    lv_obj_t *container = gui_create_clean_container(lv_scr_act());
    lv_obj_set_size(container, lv_pct(100), lv_pct(100));
    lv_obj_center(container);
    
    // Configure flex layout vertical
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);

    // Header with WiFi icon (no back button on main screen)
    gui_header_config_t header_config = {
        .title = "Main Menu",
        .show_wifi_icon = true,
        .show_back_button = false,
        .back_callback = NULL
    };
    gui_widget_header_create(container, &header_config);

    // Spacer to center the button
    lv_obj_t *spacer = gui_create_clean_container(container);
    lv_obj_set_size(spacer, 1, 1);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);

    // Settings Button
    lv_obj_t *settings_btn = lv_btn_create(container);
    lv_obj_set_width(settings_btn, lv_pct(80));
    lv_obj_set_height(settings_btn, 50);
    lv_obj_add_event_cb(settings_btn, settings_btn_clicked, LV_EVENT_CLICKED, NULL);

    lv_obj_t *settings_label = lv_label_create(settings_btn);
    lv_label_set_text(settings_label, LV_SYMBOL_SETTINGS " Settings");
    lv_obj_center(settings_label);

    // Another spacer to balance
    lv_obj_t *spacer2 = gui_create_clean_container(container);
    lv_obj_set_size(spacer2, 1, 1);
    lv_obj_set_flex_grow(spacer2, 1);
    lv_obj_set_style_bg_opa(spacer2, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(spacer2, LV_OBJ_FLAG_CLICKABLE);
    
    ESP_LOGI(TAG, "Main screen displayed");
}
