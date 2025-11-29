#ifndef OLED_HELPER_H
#define OLED_HELPER_H

#include "esp_err.h"
#include "driver/i2c_master.h"
#include <stdbool.h>
#include <stdint.h>
#include "bus/i2c_helper.h"

// SSD1306/SSD1315 I2C Address
#define OLED_I2C_ADDR           0x3C

// Screen dimensions
#define OLED_WIDTH              128
#define OLED_HEIGHT             64

// Control bytes
#define OLED_CONTROL_CMD        0x00
#define OLED_CONTROL_DATA       0x40

// SSD1306 Commands
#define OLED_CMD_SET_CONTRAST           0x81
#define OLED_CMD_DISPLAY_ALL_ON_RESUME  0xA4
#define OLED_CMD_DISPLAY_ALL_ON         0xA5
#define OLED_CMD_NORMAL_DISPLAY         0xA6
#define OLED_CMD_INVERT_DISPLAY         0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3
#define OLED_CMD_SET_COM_PINS           0xDA
#define OLED_CMD_SET_VCOM_DETECT        0xDB
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5
#define OLED_CMD_SET_PRECHARGE          0xD9
#define OLED_CMD_SET_MULTIPLEX          0xA8
#define OLED_CMD_SET_LOW_COLUMN         0x00
#define OLED_CMD_SET_HIGH_COLUMN        0x10
#define OLED_CMD_SET_START_LINE         0x40
#define OLED_CMD_MEMORY_MODE            0x20
#define OLED_CMD_COLUMN_ADDR            0x21
#define OLED_CMD_PAGE_ADDR              0x22
#define OLED_CMD_COM_SCAN_INC           0xC0
#define OLED_CMD_COM_SCAN_DEC           0xC8
#define OLED_CMD_SEG_REMAP              0xA0
#define OLED_CMD_CHARGE_PUMP            0x8D
#define OLED_CMD_ACTIVATE_SCROLL        0x2F
#define OLED_CMD_DEACTIVATE_SCROLL      0x2E
#define OLED_CMD_SET_VERTICAL_SCROLL    0xA3

/**
 * @brief Initialize OLED display
 * @return ESP_OK on success
 */
esp_err_t oled_init(void);

/**
 * @brief Deinitialize OLED display
 * @return ESP_OK on success
 */
esp_err_t oled_deinit(void);

/**
 * @brief Check if OLED is initialized
 * @return true if OLED is ready
 */
bool oled_is_ready(void);

/**
 * @brief Clear the entire display
 * @return ESP_OK on success
 */
esp_err_t oled_clear(void);

/**
 * @brief Fill the entire display with a pattern
 * @param pattern 8-bit pattern to fill with
 * @return ESP_OK on success
 */
esp_err_t oled_fill(uint8_t pattern);

/**
 * @brief Set pixel at specific coordinates
 * @param x X coordinate (0-127)
 * @param y Y coordinate (0-63)
 * @param color 1 for white, 0 for black
 * @return ESP_OK on success
 */
esp_err_t oled_set_pixel(uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief Write a character at specific position
 * @param x X coordinate (0-127)
 * @param y Y coordinate in pages (0-7)
 * @param c Character to write
 * @return ESP_OK on success
 */
esp_err_t oled_write_char(uint8_t x, uint8_t y, char c);

/**
 * @brief Write a string at specific position
 * @param x X coordinate (0-127)
 * @param y Y coordinate in pages (0-7)
 * @param str String to write
 * @return ESP_OK on success
 */
esp_err_t oled_write_string(uint8_t x, uint8_t y, const char *str);

/**
 * @brief Print text on a specific line (0-7)
 * @param line Line number (0-7)
 * @param text Text to print
 * @return ESP_OK on success
 */
esp_err_t oled_print_line(uint8_t line, const char *text);

/**
 * @brief Set contrast level
 * @param contrast Contrast value (0-255)
 * @return ESP_OK on success
 */
esp_err_t oled_set_contrast(uint8_t contrast);

/**
 * @brief Invert display colors
 * @param invert true to invert, false for normal
 * @return ESP_OK on success
 */
esp_err_t oled_invert_display(bool invert);

/**
 * @brief Update display with current buffer
 * @return ESP_OK on success
 */
esp_err_t oled_update(void);

#endif // OLED_HELPER_H
