#ifndef TOUCH_HELPER_H
#define TOUCH_HELPER_H

#include "esp_err.h"
#include "esp_lcd_touch.h"

/**
 * @brief Initialize touch controller (XPT2046)
 * @return ESP_OK on success
 */
esp_err_t lcd_touch_init(void);

/**
 * @brief Deinitialize touch controller
 * @return ESP_OK on success
 */
esp_err_t touch_deinit(void);

/**
 * @brief Check if touch is initialized
 * @return true if touch is ready
 */
bool touch_is_ready(void);

/**
 * @brief Calibrate touch controller
 * @return ESP_OK on success
 */
esp_err_t touch_calibrate(void);

/**
 * @brief Get touch handle for LVGL integration
 * @return Touch handle or NULL if not initialized
 */
esp_lcd_touch_handle_t touch_get_handle(void);

// Touch configuration constants
#define TOUCH_MAX_POINTS       1
#define TOUCH_WIDTH            240
#define TOUCH_HEIGHT           320

// GPIO pin definitions
extern const int TOUCH_CS_PIN;
extern const int TOUCH_IRQ_PIN;

#endif // TOUCH_HELPER_H
