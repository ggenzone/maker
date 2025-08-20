#ifndef GUI_WIDGET_HEADER_H
#define GUI_WIDGET_HEADER_H

#include "lvgl.h"

typedef struct {
    const char *title;
    bool show_wifi_icon;
    bool show_back_button;
    void (*back_callback)(void);
} gui_header_config_t;

/**
 * @brief Create a header widget with navigation support
 * @param parent Parent container
 * @param config Header configuration
 * @return Header object
 */
lv_obj_t* gui_widget_header_create(lv_obj_t *parent, const gui_header_config_t *config);

/**
 * @brief Create a header with back navigation built-in
 * @param parent Parent container
 * @param title Header title
 * @param show_wifi_icon Whether to show WiFi icon
 * @return Header object
 */
lv_obj_t* gui_widget_header_create_with_back(lv_obj_t *parent, const char *title, bool show_wifi_icon);

#endif // GUI_WIDGET_HEADER_H
