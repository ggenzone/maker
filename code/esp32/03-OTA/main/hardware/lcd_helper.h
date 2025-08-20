#ifndef LCD_HELPER_H
#define LCD_HELPER_H

#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"

/**
 * @brief Initialize LCD display with ILI9341 driver
 * @return ESP_OK on success
 */
esp_err_t lcd_init(void);

/**
 * @brief Deinitialize LCD display
 * @return ESP_OK on success
 */
esp_err_t lcd_deinit(void);

/**
 * @brief Set LCD backlight brightness using PWM
 * @param brightness Brightness level (0-100)
 * @return ESP_OK on success
 */
esp_err_t lcd_set_brightness(uint8_t brightness);

/**
 * @brief Get current LCD brightness
 * @return Current brightness level (0-100)
 */
uint8_t lcd_get_brightness(void);

/**
 * @brief Enable/disable auto-dim functionality
 * @param enabled True to enable auto-dim, false to disable
 * @return ESP_OK on success
 */
esp_err_t lcd_set_autodim_enabled(bool enabled);

/**
 * @brief Check if auto-dim is enabled
 * @return True if auto-dim is enabled
 */
bool lcd_is_autodim_enabled(void);

/**
 * @brief Reset the auto-dim timer (call on user interaction)
 */
void lcd_reset_autodim_timer(void);

/**
 * @brief Manually trigger auto-dim
 */
esp_err_t lcd_trigger_autodim(void);

/**
 * @brief Restore from auto-dim state
 */
esp_err_t lcd_restore_from_autodim(void);

/**
 * @brief Check if display is currently dimmed
 */
bool lcd_is_dimmed(void);

/**
 * @brief Turn screen completely off
 * @return ESP_OK on success
 */
esp_err_t lcd_screen_off(void);

/**
 * @brief Check if LCD is initialized
 * @return true if LCD is ready
 */
bool lcd_is_ready(void);

// LCD configuration constants
#define LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#define LCD_WIDTH              240
#define LCD_HEIGHT             320


#endif // LCD_HELPER_H
