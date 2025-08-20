#ifndef GUI_H
#define GUI_H

#include "lvgl.h"

// Include all GUI modules
#include "gui_styles.h"
#include "gui_common.h"
#include "navigation/gui_navigator.h"

/**
 * @brief Initialize the entire GUI system
 * Call this once at startup before using any GUI functions
 */
void gui_init(void);

/**
 * @brief Update WiFi icon status
 * @param connected WiFi connection status
 */
void gui_update_wifi_status(bool connected);

// Convenience functions for main navigation (maintains backward compatibility)
#define gui_show_main_menu()        gui_navigate_to(GUI_SCREEN_MAIN, NULL)
#define gui_show_settings_menu()    gui_navigate_to(GUI_SCREEN_SETTINGS, NULL)
#define gui_show_general_settings() gui_navigate_to(GUI_SCREEN_GENERAL, NULL)
#define gui_show_wifi_settings()    gui_navigate_to(GUI_SCREEN_WIFI, NULL)
#define gui_show_system_settings()  gui_navigate_to(GUI_SCREEN_SETTINGS_SYSTEM, NULL)

#endif // GUI_H
