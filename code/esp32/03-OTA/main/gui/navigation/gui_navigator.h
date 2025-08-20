#ifndef GUI_NAVIGATOR_H
#define GUI_NAVIGATOR_H

#include "lvgl.h"

typedef enum {
    GUI_SCREEN_MAIN,
    GUI_SCREEN_SETTINGS,
    GUI_SCREEN_GENERAL,
    GUI_SCREEN_WIFI,
    GUI_SCREEN_WIFI_INFO,
    GUI_SCREEN_SETTINGS_SYSTEM
} gui_screen_id_t;

/**
 * @brief Initialize the navigation system
 */
void gui_navigator_init(void);

/**
 * @brief Navigate to a specific screen
 * @param screen_id Target screen ID
 * @param user_data Optional data to pass to screen
 */
void gui_navigate_to(gui_screen_id_t screen_id, void *user_data);

/**
 * @brief Go back to previous screen
 */
void gui_navigate_back(void);

/**
 * @brief Get current screen ID
 */
gui_screen_id_t gui_get_current_screen(void);

/**
 * @brief Clear navigation stack (used for going to main screen)
 */
void gui_navigate_clear_stack(void);

#endif // GUI_NAVIGATOR_H
