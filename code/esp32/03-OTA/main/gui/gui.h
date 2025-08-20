#ifndef GUI_H
#define GUI_H

#include "lvgl.h"

// Include all GUI modules
#include "gui_styles.h"
#include "gui_common.h"
#include "gui_main.h"
#include "gui_settings.h"

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

// Navigation states
typedef enum {
    GUI_SCREEN_MAIN,
    GUI_SCREEN_SETTINGS,
    GUI_SCREEN_GENERAL_SETTINGS,
    GUI_SCREEN_WIFI_SETTINGS
} gui_screen_t;

#endif // GUI_H
