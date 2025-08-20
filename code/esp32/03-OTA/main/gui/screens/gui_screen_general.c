#include "gui_screen_general.h"
#include "../navigation/gui_navigator.h"
#include "../widgets/gui_widget_header.h"
#include "../widgets/gui_widget_brightness.h"
#include "../gui_common.h"
#include "../gui_styles.h"
#include "esp_log.h"

static const char *TAG = "GUI_SCREEN_GENERAL";

void gui_screen_general_show(void *user_data) {
    ESP_LOGI(TAG, "Showing general settings screen");
    
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
    gui_widget_header_create_with_back(container, "General Settings", false);

    // Brightness Control Container
    lv_obj_t *brightness_container = gui_create_clean_container(container);
    lv_obj_set_size(brightness_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(brightness_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(brightness_container, 15, 0);
    
    // Create brightness widget with default configuration
    gui_brightness_config_t brightness_config = {
        .current_value = 0, // Will use value from app_config
        .show_autodim_toggle = true,
        .value_changed_cb = NULL,
        .autodim_changed_cb = NULL
    };
    gui_widget_brightness_create(brightness_container, &brightness_config);
    
    ESP_LOGI(TAG, "General settings screen displayed");
}
