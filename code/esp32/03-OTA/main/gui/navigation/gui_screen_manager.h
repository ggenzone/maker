#ifndef GUI_SCREEN_MANAGER_H
#define GUI_SCREEN_MANAGER_H

#include "gui_navigator.h"

/**
 * @brief Initialize screen manager
 */
void gui_screen_manager_init(void);

/**
 * @brief Show specific screen
 * @param screen_id Screen to show
 * @param user_data Optional data for the screen
 */
void gui_screen_manager_show(gui_screen_id_t screen_id, void *user_data);

#endif // GUI_SCREEN_MANAGER_H
