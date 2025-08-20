#include "gui_main.h"
#include "gui_common.h"
#include "gui_styles.h"
#include "gui_settings.h"
#include "esp_log.h"

static const char *TAG = "GUI_MAIN";

// Static variables
static lv_obj_t *main_menu = NULL;

// Forward declarations
static void settings_btn_event_handler(lv_event_t *e);

void gui_show_main_menu(void) {
    ESP_LOGI(TAG, "Showing main menu");
    
    lv_obj_clean(lv_scr_act());

    main_menu = gui_create_clean_container(lv_scr_act());
    lv_obj_set_size(main_menu, lv_pct(100), lv_pct(100));
    lv_obj_center(main_menu);
    
    // Configure flex layout vertical
    lv_obj_set_flex_flow(main_menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(main_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(main_menu, 0, 0);
    lv_obj_set_style_pad_all(main_menu, 0, 0);

    // Header with WiFi icon
    gui_create_header(main_menu, "Main Menu", true);

    // Spacer to center the button
    lv_obj_t *spacer = gui_create_clean_container(main_menu);
    lv_obj_set_size(spacer, 1, 1);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);

    // Settings Button
    lv_obj_t *settings_btn = lv_btn_create(main_menu);
    lv_obj_set_width(settings_btn, lv_pct(80));
    lv_obj_set_height(settings_btn, 50);
    lv_obj_add_event_cb(settings_btn, settings_btn_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *settings_label = lv_label_create(settings_btn);
    lv_label_set_text(settings_label, LV_SYMBOL_SETTINGS " Settings");
    lv_obj_center(settings_label);

    // Another spacer to balance
    lv_obj_t *spacer2 = gui_create_clean_container(main_menu);
    lv_obj_set_size(spacer2, 1, 1);
    lv_obj_set_flex_grow(spacer2, 1);
    lv_obj_set_style_bg_opa(spacer2, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(spacer2, LV_OBJ_FLAG_CLICKABLE);
    
    ESP_LOGI(TAG, "Main menu displayed");
}

// Event handlers
static void settings_btn_event_handler(lv_event_t *e) {
    ESP_LOGI(TAG, "Settings button clicked");
    gui_show_settings_menu();
}
