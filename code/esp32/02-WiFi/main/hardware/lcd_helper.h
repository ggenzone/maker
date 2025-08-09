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
 * @brief Dim screen to low brightness (for power saving)
 * @return ESP_OK on success
 */
esp_err_t lcd_dim_screen(void);

/**
 * @brief Restore previous brightness level
 * @return ESP_OK on success
 */
esp_err_t lcd_restore_brightness(void);

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

// GPIO pin definitions (will be read from Kconfig)
extern const int LCD_SCLK_PIN;
extern const int LCD_MOSI_PIN;
extern const int LCD_MISO_PIN;
extern const int LCD_DC_PIN;
extern const int LCD_RST_PIN;
extern const int LCD_CS_PIN;
extern const int LCD_BACKLIGHT_PIN;

#endif // LCD_HELPER_H
