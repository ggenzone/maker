#include "gui_widget_header.h"
#include "../gui_common.h"
#include "../navigation/gui_navigator.h"
#include "esp_log.h"

static const char *TAG = "GUI_WIDGET_HEADER";

static void header_back_callback(lv_event_t *e) {
    void (*callback)(void) = lv_event_get_user_data(e);
    if (callback) {
        callback();
    } else {
        gui_navigate_back();
    }
}

lv_obj_t* gui_widget_header_create(lv_obj_t *parent, const gui_header_config_t *config) {
    if (!config || !config->title) {
        ESP_LOGE(TAG, "Invalid header configuration");
        return NULL;
    }
    
    // Use existing common header function
    lv_obj_t *header = gui_create_header(parent, config->title, config->show_wifi_icon);
    
    if (config->show_back_button) {
        // Modify the left icon to show back arrow and add click handler
        lv_obj_t *left_icon = lv_obj_get_child(header, 0);
        if (left_icon) {
            lv_label_set_text(left_icon, LV_SYMBOL_LEFT);
            lv_obj_add_flag(header, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(header, header_back_callback, LV_EVENT_CLICKED, config->back_callback);
        }
    }
    
    return header;
}

lv_obj_t* gui_widget_header_create_with_back(lv_obj_t *parent, const char *title, bool show_wifi_icon) {
    gui_header_config_t config = {
        .title = title,
        .show_wifi_icon = show_wifi_icon,
        .show_back_button = true,
        .back_callback = NULL  // Use default navigation back
    };
    
    return gui_widget_header_create(parent, &config);
}
