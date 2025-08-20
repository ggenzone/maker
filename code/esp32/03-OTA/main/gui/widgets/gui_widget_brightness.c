#include "gui_widget_brightness.h"
#include "../gui_styles.h"
#include "storage/app_config.h"
#include "lcd_helper.h"
#include "esp_log.h"

static const char *TAG = "GUI_WIDGET_BRIGHTNESS";

typedef struct {
    lv_obj_t *value_label;
    lv_obj_t *slider;
    lv_obj_t *autodim_switch;
    void (*value_changed_cb)(uint8_t brightness);
    void (*autodim_changed_cb)(bool enabled);
} brightness_widget_data_t;

static void brightness_slider_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    brightness_widget_data_t *data = lv_event_get_user_data(e);
    
    int32_t brightness = lv_slider_get_value(slider);
    
    // Apply brightness immediately
    lcd_set_brightness((uint8_t)brightness);
    
    // Save to configuration
    app_config_set_lcd_brightness((uint8_t)brightness);
    app_config_save();
    
    // Update value label
    if (data && data->value_label) {
        lv_label_set_text_fmt(data->value_label, "Brightness (%ld%%)", brightness);
    }
    
    // Call user callback if provided
    if (data && data->value_changed_cb) {
        data->value_changed_cb((uint8_t)brightness);
    }
    
    ESP_LOGI(TAG, "Brightness changed to: %ld%% and saved", brightness);
}

static void autodim_switch_cb(lv_event_t *e) {
    lv_obj_t *sw = lv_event_get_target(e);
    brightness_widget_data_t *data = lv_event_get_user_data(e);
    
    bool enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);

    ESP_LOGI(TAG, "Auto-dim toggle: %s", enabled ? "ON" : "OFF");
    
    // Update hardware auto-dim
    lcd_set_autodim_enabled(enabled);
    
    // Save to configuration
    app_config_set_auto_dim_enabled(enabled);
    app_config_save();
    
    // Call user callback if provided
    if (data && data->autodim_changed_cb) {
        data->autodim_changed_cb(enabled);
    }
    
    ESP_LOGI(TAG, "Auto-dim setting saved and applied: %s", enabled ? "enabled" : "disabled");
}

lv_obj_t* gui_widget_brightness_create(lv_obj_t *parent, const gui_brightness_config_t *config) {
    if (!parent) {
        ESP_LOGE(TAG, "Invalid parent container");
        return NULL;
    }
    
    // Get current values from config or use defaults
    uint8_t current_brightness = config ? config->current_value : app_config_get_lcd_brightness();
    bool show_autodim = config ? config->show_autodim_toggle : true;
    
    // Create main container
    lv_obj_t *container = gui_create_clean_container(parent);
    lv_obj_set_size(container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(container, 15, 0);
    
    // Allocate widget data
    brightness_widget_data_t *widget_data = lv_malloc(sizeof(brightness_widget_data_t));
    if (!widget_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for widget data");
        return container;
    }
    
    widget_data->value_changed_cb = config ? config->value_changed_cb : NULL;
    widget_data->autodim_changed_cb = config ? config->autodim_changed_cb : NULL;
    
    // Value label
    lv_obj_t *value_label = lv_label_create(container);
    lv_label_set_text_fmt(value_label, "Brightness (%d%%)", current_brightness);
    lv_obj_align(value_label, LV_ALIGN_TOP_LEFT, 10, 10);
    widget_data->value_label = value_label;
    
    // Slider
    lv_obj_t *slider = lv_slider_create(container);
    lv_obj_set_width(slider, lv_pct(80));
    lv_obj_align_to(slider, value_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_slider_set_range(slider, 10, 100);
    lv_slider_set_value(slider, current_brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, brightness_slider_cb, LV_EVENT_VALUE_CHANGED, widget_data);
    widget_data->slider = slider;

    if (show_autodim) {
        // Auto-dim setting container
        lv_obj_t *autodim_container = gui_create_clean_container(container);
        lv_obj_set_size(autodim_container, lv_pct(100), 50);
        lv_obj_align_to(autodim_container, slider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
        lv_obj_clear_flag(autodim_container, LV_OBJ_FLAG_SCROLLABLE);
        
        // Auto-dim label
        lv_obj_t *autodim_label = lv_label_create(autodim_container);
        lv_label_set_text(autodim_label, "Auto Dim");
        lv_obj_align(autodim_label, LV_ALIGN_LEFT_MID, 10, 0);
        
        // Auto-dim switch
        lv_obj_t *autodim_switch = lv_switch_create(autodim_container);
        lv_obj_align(autodim_switch, LV_ALIGN_RIGHT_MID, -10, 0);
        lv_obj_add_event_cb(autodim_switch, autodim_switch_cb, LV_EVENT_VALUE_CHANGED, widget_data);
        widget_data->autodim_switch = autodim_switch;
        
        // Set current state from config
        bool autodim_enabled = app_config_get_auto_dim_enabled();
        if (autodim_enabled) {
            lv_obj_add_state(autodim_switch, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(autodim_switch, LV_STATE_CHECKED);
        }
    } else {
        widget_data->autodim_switch = NULL;
    }
    
    ESP_LOGI(TAG, "Brightness widget created");
    return container;
}
