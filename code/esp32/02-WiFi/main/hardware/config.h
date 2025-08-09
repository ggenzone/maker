#ifndef CONFIG_H
#define CONFIG_H

// LCD configuration constants
#define LCD_PIXEL_CLOCK_HZ     (20 * 1000 * 1000)
#define LCD_BK_LIGHT_ON_LEVEL  0
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL

// The GPIO pins used for SPI communication
#define GMAKER_SPI_SCLK_PIN           CONFIG_GMAKER_SPI_SCLK_PIN
#define GMAKER_SPI_MOSI_PIN           CONFIG_GMAKER_SPI_MOSI_PIN
#define GMAKER_SPI_MISO_PIN           CONFIG_GMAKER_SPI_MISO_PIN

// The GPIO pins used for LCD control
#define GMAKER_LCD_DC_PIN         CONFIG_GMAKER_LCD_DC_PIN
#define GMAKER_LCD_RST_PIN        CONFIG_GMAKER_LCD_RST_PIN
#define GMAKER_LCD_CS_PIN         CONFIG_GMAKER_LCD_CS_PIN
#define GMAKER_LCD_BACKLIGHT_PIN  CONFIG_GMAKER_LCD_BACKLIGHT_PIN

// The GPIO pin used for touch controller chip select
#define GMAKER_TOUCH_CS_PIN       CONFIG_GMAKER_TOUCH_CS_PIN


// Bit number used to represent command and parameter
#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

#define LVGL_DRAW_BUF_LINES    20 // number of display lines in each draw buffer
#define LVGL_TICK_PERIOD_MS    2



// LCD configuration constants
#define LCD_HOST               SPI2_HOST
#define LCD_MIRROR_Y           1
#define LCD_WIDTH              240
#define LCD_HEIGHT             320
#define LCD_H_RES              240
#define LCD_V_RES              320

// PWM configuration for backlight
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // 13-bit resolution
#define LEDC_FREQUENCY          (4000) // 4 kHz

#endif // CONFIG_H