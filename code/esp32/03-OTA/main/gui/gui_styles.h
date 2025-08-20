#ifndef GUI_STYLES_H
#define GUI_STYLES_H

#include "lvgl.h"

/**
 * @brief Initialize all GUI styles
 * Call this once at the beginning before creating any GUI elements
 */
void gui_styles_init(void);

/**
 * @brief Create a clean container without borders, radius, shadows
 * @param parent Parent object
 * @return Clean container object
 */
lv_obj_t* gui_create_clean_container(lv_obj_t *parent);

/**
 * @brief Apply clean styling to existing object
 * @param obj Object to clean
 */
void gui_apply_clean_style(lv_obj_t *obj);

// Global styles
extern lv_style_t gui_clean_style;

#endif // GUI_STYLES_H
