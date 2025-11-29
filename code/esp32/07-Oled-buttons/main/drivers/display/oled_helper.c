#include "oled_helper.h"
#include "esp_log.h"
#include "bus/i2c_helper.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "OLED";
static bool oled_initialized = false;
static i2c_master_dev_handle_t oled_dev_handle = NULL;

// Frame buffer for the display (128x64 / 8 = 1024 bytes)
static uint8_t oled_buffer[OLED_WIDTH * OLED_HEIGHT / 8];

// Simple 5x8 font (ASCII 32-127)
static const uint8_t font5x8[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // Backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00}, // }
    {0x08, 0x04, 0x08, 0x10, 0x08}, // ~
};

// Helper function to send command to OLED
static esp_err_t oled_send_cmd(uint8_t cmd) {
    uint8_t buffer[2] = {OLED_CONTROL_CMD, cmd};
    return i2c_master_transmit(oled_dev_handle, buffer, sizeof(buffer), 1000);
}

// Helper function to send data to OLED
static esp_err_t oled_send_data(uint8_t *data, size_t len) {
    // Allocate buffer with control byte + data
    uint8_t *buffer = malloc(len + 1);
    if (buffer == NULL) {
        return ESP_ERR_NO_MEM;
    }
    
    buffer[0] = OLED_CONTROL_DATA;
    memcpy(buffer + 1, data, len);
    
    esp_err_t ret = i2c_master_transmit(oled_dev_handle, buffer, len + 1, 1000);
    free(buffer);
    return ret;
}

esp_err_t oled_init(void) {
    if (oled_initialized) {
        ESP_LOGW(TAG, "OLED already initialized");
        return ESP_OK;
    }

    if (!i2c_helper_is_ready()) {
        ESP_LOGE(TAG, "I2C bus not initialized. Call i2c_helper_init() first.");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Initializing OLED display...");

    // Add device to I2C bus first
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_I2C_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    esp_err_t ret = i2c_master_bus_add_device(i2c_helper_get_bus_handle(), &dev_cfg, &oled_dev_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add OLED device to I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "OLED device added to I2C bus at 0x%02X", OLED_I2C_ADDR);

    // Give the device a moment to be ready
    vTaskDelay(pdMS_TO_TICKS(50));

    // Initialization sequence for SSD1306/SSD1315
    
    ret = oled_send_cmd(OLED_CMD_DISPLAY_OFF);  // Display off
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send display off command: %s", esp_err_to_name(ret));
        ESP_LOGW(TAG, "OLED may not be responding properly");
        i2c_master_bus_rm_device(oled_dev_handle);
        oled_dev_handle = NULL;
        return ret;
    }
    
    ret = oled_send_cmd(OLED_CMD_SET_DISPLAY_CLK_DIV);  // Set clock divide ratio
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0x80);  // Suggested ratio
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_SET_MULTIPLEX);  // Set multiplex ratio
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(OLED_HEIGHT - 1);
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_SET_DISPLAY_OFFSET);  // Set display offset
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0x00);  // No offset
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_SET_START_LINE | 0x00);  // Set start line
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_CHARGE_PUMP);  // Charge pump
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0x14);  // Enable charge pump
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_MEMORY_MODE);  // Memory mode
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0x00);  // Horizontal addressing mode
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_SEG_REMAP | 0x01);  // Segment re-map
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_COM_SCAN_DEC);  // COM scan direction
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_SET_COM_PINS);  // Set COM pins
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0x12);
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_SET_CONTRAST);  // Set contrast
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0xCF);
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_SET_PRECHARGE);  // Set pre-charge period
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0xF1);
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_SET_VCOM_DETECT);  // Set VCOMH deselect level
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0x40);
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_DISPLAY_ALL_ON_RESUME);  // Resume to RAM content
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_NORMAL_DISPLAY);  // Normal display
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_DEACTIVATE_SCROLL);  // Deactivate scroll
    if (ret != ESP_OK) return ret;
    
    ret = oled_send_cmd(OLED_CMD_DISPLAY_ON);  // Display on
    if (ret != ESP_OK) return ret;

    // Clear buffer
    memset(oled_buffer, 0, sizeof(oled_buffer));
    
    oled_initialized = true;
    ESP_LOGI(TAG, "OLED initialized successfully");
    
    // Clear display
    oled_clear();
    
    return ESP_OK;
}

esp_err_t oled_deinit(void) {
    if (!oled_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing OLED display");
    
    // Turn off display
    oled_send_cmd(OLED_CMD_DISPLAY_OFF);
    
    // Remove device from I2C bus
    if (oled_dev_handle != NULL) {
        i2c_master_bus_rm_device(oled_dev_handle);
        oled_dev_handle = NULL;
    }
    
    oled_initialized = false;
    
    return ESP_OK;
}

bool oled_is_ready(void) {
    return oled_initialized && i2c_helper_is_ready();
}

esp_err_t oled_clear(void) {
    if (!oled_initialized) {
        ESP_LOGE(TAG, "OLED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    memset(oled_buffer, 0, sizeof(oled_buffer));
    return oled_update();
}

esp_err_t oled_fill(uint8_t pattern) {
    if (!oled_initialized) {
        ESP_LOGE(TAG, "OLED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    memset(oled_buffer, pattern, sizeof(oled_buffer));
    return oled_update();
}

esp_err_t oled_set_pixel(uint8_t x, uint8_t y, uint8_t color) {
    if (!oled_initialized) {
        ESP_LOGE(TAG, "OLED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t index = x + (y / 8) * OLED_WIDTH;
    uint8_t bit = y % 8;

    if (color) {
        oled_buffer[index] |= (1 << bit);
    } else {
        oled_buffer[index] &= ~(1 << bit);
    }

    return ESP_OK;
}

esp_err_t oled_write_char(uint8_t x, uint8_t y, char c) {
    if (!oled_initialized) {
        ESP_LOGE(TAG, "OLED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (x >= OLED_WIDTH || y >= 8) {
        return ESP_ERR_INVALID_ARG;
    }

    // Only support printable ASCII characters
    if (c < 32 || c > 126) {
        c = ' ';
    }

    // Get font data
    const uint8_t *font_data = font5x8[c - 32];

    // Write character to buffer
    for (int i = 0; i < 5; i++) {
        if (x + i < OLED_WIDTH) {
            oled_buffer[x + i + y * OLED_WIDTH] = font_data[i];
        }
    }

    // Add spacing
    if (x + 5 < OLED_WIDTH) {
        oled_buffer[x + 5 + y * OLED_WIDTH] = 0x00;
    }

    return ESP_OK;
}

esp_err_t oled_write_string(uint8_t x, uint8_t y, const char *str) {
    if (!oled_initialized) {
        ESP_LOGE(TAG, "OLED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (str == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t cur_x = x;
    
    while (*str && cur_x < OLED_WIDTH) {
        esp_err_t ret = oled_write_char(cur_x, y, *str);
        if (ret != ESP_OK) {
            return ret;
        }
        cur_x += 6;  // 5 pixels + 1 spacing
        str++;
    }

    return ESP_OK;
}

esp_err_t oled_print_line(uint8_t line, const char *text) {
    if (!oled_initialized) {
        ESP_LOGE(TAG, "OLED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (line >= 8) {
        return ESP_ERR_INVALID_ARG;
    }

    // Clear the line first
    for (int i = 0; i < OLED_WIDTH; i++) {
        oled_buffer[i + line * OLED_WIDTH] = 0x00;
    }

    // Write text
    esp_err_t ret = oled_write_string(0, line, text);
    if (ret != ESP_OK) {
        return ret;
    }

    // Update only the affected line
    ret = oled_send_cmd(OLED_CMD_PAGE_ADDR);
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(line);
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(line);
    if (ret != ESP_OK) return ret;

    ret = oled_send_cmd(OLED_CMD_COLUMN_ADDR);
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0);
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(OLED_WIDTH - 1);
    if (ret != ESP_OK) return ret;

    return oled_send_data(&oled_buffer[line * OLED_WIDTH], OLED_WIDTH);
}

esp_err_t oled_set_contrast(uint8_t contrast) {
    if (!oled_initialized) {
        ESP_LOGE(TAG, "OLED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = oled_send_cmd(OLED_CMD_SET_CONTRAST);
    if (ret != ESP_OK) return ret;
    
    return oled_send_cmd(contrast);
}

esp_err_t oled_invert_display(bool invert) {
    if (!oled_initialized) {
        ESP_LOGE(TAG, "OLED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    return oled_send_cmd(invert ? OLED_CMD_INVERT_DISPLAY : OLED_CMD_NORMAL_DISPLAY);
}

esp_err_t oled_update(void) {
    if (!oled_initialized) {
        ESP_LOGE(TAG, "OLED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret;
    
    // Set column address range
    ret = oled_send_cmd(OLED_CMD_COLUMN_ADDR);
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0);  // Start column
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(OLED_WIDTH - 1);  // End column
    if (ret != ESP_OK) return ret;

    // Set page address range
    ret = oled_send_cmd(OLED_CMD_PAGE_ADDR);
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(0);  // Start page
    if (ret != ESP_OK) return ret;
    ret = oled_send_cmd(7);  // End page (64/8-1)
    if (ret != ESP_OK) return ret;

    // Send buffer data in chunks
    const size_t chunk_size = 16;  // Send 16 bytes at a time
    for (size_t i = 0; i < sizeof(oled_buffer); i += chunk_size) {
        size_t len = (i + chunk_size > sizeof(oled_buffer)) ? (sizeof(oled_buffer) - i) : chunk_size;
        ret = oled_send_data(&oled_buffer[i], len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send data chunk at offset %d", i);
            return ret;
        }
    }

    return ESP_OK;
}
