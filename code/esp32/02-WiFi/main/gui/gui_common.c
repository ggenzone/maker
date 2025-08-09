#include "gui_common.h"
#include "gui_styles.h"
#include "esp_log.h"

static const char *TAG = "GUI_COMMON";

// Global WiFi icon reference
lv_obj_t *g_wifi_icon = NULL;

lv_obj_t* gui_create_header(lv_obj_t *parent, const char *title, bool show_wifi_icon) {
    // Create header container
    lv_obj_t *header = gui_create_clean_container(parent);
    lv_obj_set_size(header, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(header, 10, 0);
    
    // Left icon (menu or back)
    lv_obj_t *left_icon = lv_label_create(header);
    lv_label_set_text(left_icon, LV_SYMBOL_LIST);
    
    // Center title
    lv_obj_t *title_label = lv_label_create(header);
    lv_label_set_text(title_label, title);
    
    // Right icon (WiFi or empty)
    lv_obj_t *right_icon = lv_label_create(header);
    if (show_wifi_icon) {
        g_wifi_icon = right_icon; // Save global reference
        lv_label_set_text(g_wifi_icon, LV_SYMBOL_WIFI);
    } else {
        lv_label_set_text(right_icon, ""); // Placeholder for spacing
    }
    
    ESP_LOGD(TAG, "Header created: %s", title);
    return header;
}

lv_obj_t* gui_create_setting_item(lv_obj_t *parent, const char *text, const char *icon) {
    lv_obj_t *item = gui_create_clean_container(parent);
    lv_obj_set_size(item, lv_pct(100), 60);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(item, 15, 0);
    
    // Left icon
    lv_obj_t *icon_label = lv_label_create(item);
    lv_label_set_text(icon_label, icon);
    lv_obj_align(icon_label, LV_ALIGN_LEFT_MID, 0, 0);
    
    // Text
    lv_obj_t *text_label = lv_label_create(item);
    lv_label_set_text(text_label, text);
    lv_obj_align_to(text_label, icon_label, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    
    // Right arrow
    lv_obj_t *arrow = lv_label_create(item);
    lv_label_set_text(arrow, LV_SYMBOL_RIGHT);
    lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, 0, 0);
    
    ESP_LOGD(TAG, "Setting item created: %s", text);
    return item;
}

lv_obj_t* gui_create_back_button(lv_obj_t *parent, lv_event_cb_t callback) {
    lv_obj_t *back_btn = lv_btn_create(parent);
    lv_obj_set_width(back_btn, lv_pct(80));
    lv_obj_set_height(back_btn, 50);
    lv_obj_add_event_cb(back_btn, callback, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, LV_SYMBOL_LEFT " Back");
    lv_obj_center(back_label);
    
    ESP_LOGD(TAG, "Back button created");
    return back_btn;
}

void gui_update_wifi_icon(bool connected) {
    if (!g_wifi_icon) return;

    lv_label_set_text(g_wifi_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(g_wifi_icon,
        connected ? lv_color_hex(0x00ff00) : lv_color_hex(0xff0000),
        LV_PART_MAIN);
    
    ESP_LOGD(TAG, "WiFi icon updated: %s", connected ? "connected" : "disconnected");
}
