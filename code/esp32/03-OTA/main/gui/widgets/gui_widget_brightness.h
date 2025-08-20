#ifndef GUI_WIDGET_BRIGHTNESS_H
#define GUI_WIDGET_BRIGHTNESS_H

#include "lvgl.h"

typedef struct {
    uint8_t current_value;
    bool show_autodim_toggle;
    void (*value_changed_cb)(uint8_t brightness);
    void (*autodim_changed_cb)(bool enabled);
} gui_brightness_config_t;

/**
 * @brief Create brightness control widget
 * @param parent Parent container
 * @param config Brightness configuration
 * @return Brightness widget container
 */
lv_obj_t* gui_widget_brightness_create(lv_obj_t *parent, const gui_brightness_config_t *config);

#endif // GUI_WIDGET_BRIGHTNESS_H
