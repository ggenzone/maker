#include "gui_settings.h"
#include "gui_common.h"
#include "gui_styles.h"
#include "gui_main.h"
#include "app_state.h"
#include "wifi.h"
#include "lcd_helper.h"
#include "esp_log.h"

static const char *TAG = "GUI_SETTINGS";

// Static variables
static lv_obj_t *settings_menu = NULL;
static lv_obj_t *general_settings_menu = NULL;
static lv_obj_t *wifi_settings_menu = NULL;
static lv_obj_t *wifi_toggle = NULL;

// Forward declarations
static void back_btn_event_handler(lv_event_t *e);
static void general_settings_btn_event_handler(lv_event_t *e);
static void wifi_settings_btn_event_handler(lv_event_t *e);
static void settings_back_btn_event_handler(lv_event_t *e);
static void wifi_toggle_event_handler(lv_event_t *e);
static void brightness_slider_cb(lv_event_t *e);
static void create_brightness_control(lv_obj_t *parent);

void gui_show_settings_menu(void) {
    ESP_LOGI(TAG, "Showing settings menu");
    
    lv_obj_clean(lv_scr_act());

    settings_menu = gui_create_clean_container(lv_scr_act());
    lv_obj_set_size(settings_menu, lv_pct(100), lv_pct(100));
    lv_obj_center(settings_menu);
    
    // Configure flex layout vertical
    lv_obj_set_flex_flow(settings_menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(settings_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(settings_menu, 0, 0);
    lv_obj_set_style_pad_all(settings_menu, 0, 0);

    // Header without WiFi icon
    lv_obj_t *header = gui_create_header(settings_menu, "Settings", false);
    
    // Change left icon to "back" in settings
    lv_obj_t *left_icon = lv_obj_get_child(header, 0);
    lv_label_set_text(left_icon, LV_SYMBOL_LEFT);
    lv_obj_add_event_cb(left_icon, back_btn_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(left_icon, LV_OBJ_FLAG_CLICKABLE);

    // General Settings item
    lv_obj_t *general_item = gui_create_setting_item(settings_menu, "General", LV_SYMBOL_SETTINGS);
    lv_obj_add_event_cb(general_item, general_settings_btn_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(general_item, LV_OBJ_FLAG_CLICKABLE);

    // WiFi Settings item
    lv_obj_t *wifi_item = gui_create_setting_item(settings_menu, "WiFi", LV_SYMBOL_WIFI);
    lv_obj_add_event_cb(wifi_item, wifi_settings_btn_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(wifi_item, LV_OBJ_FLAG_CLICKABLE);

    // Spacer to push back button down
    lv_obj_t *spacer = gui_create_clean_container(settings_menu);
    lv_obj_set_size(spacer, 1, 1);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);

    // Back Button
    gui_create_back_button(settings_menu, back_btn_event_handler);
    
    ESP_LOGI(TAG, "Settings menu displayed");
}

void gui_show_general_settings(void) {
    ESP_LOGI(TAG, "Showing general settings");
    
    lv_obj_clean(lv_scr_act());

    general_settings_menu = gui_create_clean_container(lv_scr_act());
    lv_obj_set_size(general_settings_menu, lv_pct(100), lv_pct(100));
    lv_obj_center(general_settings_menu);
    
    // Configure flex layout vertical
    lv_obj_set_flex_flow(general_settings_menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(general_settings_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(general_settings_menu, 0, 0);
    lv_obj_set_style_pad_all(general_settings_menu, 0, 0);

    // Header
    lv_obj_t *header = gui_create_header(general_settings_menu, "General Settings", false);
    
    // Change left icon to "back"
    lv_obj_t *left_icon = lv_obj_get_child(header, 0);
    lv_label_set_text(left_icon, LV_SYMBOL_LEFT);
    lv_obj_add_event_cb(left_icon, settings_back_btn_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(left_icon, LV_OBJ_FLAG_CLICKABLE);

    // Brightness Control Container
    lv_obj_t *brightness_container = gui_create_clean_container(general_settings_menu);
    lv_obj_set_size(brightness_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(brightness_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(brightness_container, 15, 0);
    
    create_brightness_control(brightness_container);

    // Spacer
    lv_obj_t *spacer = gui_create_clean_container(general_settings_menu);
    lv_obj_set_size(spacer, 1, 1);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);

    // Back Button
    gui_create_back_button(general_settings_menu, settings_back_btn_event_handler);
    
    ESP_LOGI(TAG, "General settings displayed");
}

void gui_show_wifi_settings(void) {
    ESP_LOGI(TAG, "Showing WiFi settings");
    
    lv_obj_clean(lv_scr_act());

    wifi_settings_menu = gui_create_clean_container(lv_scr_act());
    lv_obj_set_size(wifi_settings_menu, lv_pct(100), lv_pct(100));
    lv_obj_center(wifi_settings_menu);
    
    // Configure flex layout vertical
    lv_obj_set_flex_flow(wifi_settings_menu, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(wifi_settings_menu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(wifi_settings_menu, 0, 0);
    lv_obj_set_style_pad_all(wifi_settings_menu, 0, 0);

    // Header
    lv_obj_t *header = gui_create_header(wifi_settings_menu, "WiFi Settings", false);
    
    // Change left icon to "back"
    lv_obj_t *left_icon = lv_obj_get_child(header, 0);
    lv_label_set_text(left_icon, LV_SYMBOL_LEFT);
    lv_obj_add_event_cb(left_icon, settings_back_btn_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(left_icon, LV_OBJ_FLAG_CLICKABLE);

    // WiFi Enable/Disable Container
    lv_obj_t *wifi_container = gui_create_clean_container(wifi_settings_menu);
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
    if (app_state.wifi_enabled)
        lv_obj_add_state(wifi_toggle, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(wifi_toggle, LV_STATE_CHECKED);

    // Placeholder for future WiFi functions
    lv_obj_t *placeholder = lv_label_create(wifi_settings_menu);
    lv_label_set_text(placeholder, "WiFi scan and connect\ncoming soon...");
    lv_obj_set_style_text_align(placeholder, LV_TEXT_ALIGN_CENTER, 0);

    // Spacer
    lv_obj_t *spacer = gui_create_clean_container(wifi_settings_menu);
    lv_obj_set_size(spacer, 1, 1);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(spacer, LV_OBJ_FLAG_CLICKABLE);

    // Back Button
    gui_create_back_button(wifi_settings_menu, settings_back_btn_event_handler);
    
    ESP_LOGI(TAG, "WiFi settings displayed");
}

// Event handlers
static void back_btn_event_handler(lv_event_t *e) {
    ESP_LOGI(TAG, "Back to main menu");
    gui_show_main_menu();
}

static void general_settings_btn_event_handler(lv_event_t *e) {
    ESP_LOGI(TAG, "General settings selected");
    gui_show_general_settings();
}

static void wifi_settings_btn_event_handler(lv_event_t *e) {
    ESP_LOGI(TAG, "WiFi settings selected");
    gui_show_wifi_settings();
}

static void settings_back_btn_event_handler(lv_event_t *e) {
    ESP_LOGI(TAG, "Back to settings menu");
    gui_show_settings_menu();
}

static void wifi_toggle_event_handler(lv_event_t *e) {
    lv_obj_t *sw = lv_event_get_target(e);
    bool on = lv_obj_has_state(sw, LV_STATE_CHECKED);

    ESP_LOGI(TAG, "WiFi toggle: %s", on ? "ON" : "OFF");
    
    if (on) {
        wifi_enable();
    } else {
        wifi_disable();
    }
}

static void brightness_slider_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t brightness = lv_slider_get_value(slider);
    
    // Apply brightness immediately
    lcd_set_brightness((uint8_t)brightness);
    
    // Update value label if available
    lv_obj_t *value_label = lv_event_get_user_data(e);
    if (value_label) {
        lv_label_set_text_fmt(value_label, "%ld%%", brightness);
    }
    
    ESP_LOGI(TAG, "Brightness changed to: %ld%%", brightness);
}

static void create_brightness_control(lv_obj_t *parent) {
    // Value label
    lv_obj_t *value_label = lv_label_create(parent);
    lv_label_set_text_fmt(value_label, "Brightness (%d%%)", lcd_get_brightness());
    lv_obj_align(value_label, LV_ALIGN_TOP_LEFT, 10, 10);
    
    // Slider
    lv_obj_t *slider = lv_slider_create(parent);
    lv_obj_set_width(slider, lv_pct(80));
    lv_obj_align_to(slider, value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_slider_set_range(slider, 10, 100); // Minimum 10% so it doesn't turn off
    
    // LVGL 9.2 correct syntax
    lv_slider_set_value(slider, lcd_get_brightness(), LV_ANIM_OFF);
    
    // Callback for changes - pass value_label as user_data
    lv_obj_add_event_cb(slider, brightness_slider_cb, LV_EVENT_VALUE_CHANGED, value_label);
}
