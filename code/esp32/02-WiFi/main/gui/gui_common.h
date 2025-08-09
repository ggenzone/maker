#ifndef GUI_COMMON_H
#define GUI_COMMON_H

#include "lvgl.h"

/**
 * @brief Create a common header with title and optional WiFi icon
 * @param parent Parent container
 * @param title Header title text
 * @param show_wifi_icon Whether to show WiFi status icon
 * @return Header object
 */
lv_obj_t* gui_create_header(lv_obj_t *parent, const char *title, bool show_wifi_icon);

/**
 * @brief Create a setting item with icon, text and arrow
 * @param parent Parent container
 * @param text Setting text
 * @param icon Setting icon (LV_SYMBOL_*)
 * @return Setting item object
 */
lv_obj_t* gui_create_setting_item(lv_obj_t *parent, const char *text, const char *icon);

/**
 * @brief Create a back button
 * @param parent Parent container
 * @param callback Button click callback
 * @return Back button object
 */
lv_obj_t* gui_create_back_button(lv_obj_t *parent, lv_event_cb_t callback);

/**
 * @brief Update WiFi icon color based on connection status
 * @param connected WiFi connection status
 */
void gui_update_wifi_icon(bool connected);

// Global WiFi icon reference
extern lv_obj_t *g_wifi_icon;

#endif // GUI_COMMON_H
