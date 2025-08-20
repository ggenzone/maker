#ifndef GUI_SETTINGS_H
#define GUI_SETTINGS_H

#include "lvgl.h"

/**
 * @brief Show the main settings menu
 */
void gui_show_settings_menu(void);

/**
 * @brief Show general settings (brightness, etc.)
 */
void gui_show_general_settings(void);

/**
 * @brief Show WiFi settings
 */
void gui_show_wifi_settings(void);

#endif // GUI_SETTINGS_H
