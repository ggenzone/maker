#include "lcd_helper.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/lock.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "lvgl.h"
#include "esp_lcd_ili9341.h"

static const char *TAG = "LCD_HELPER";

// GPIO pin assignments from Kconfig
const int LCD_SCLK_PIN = CONFIG_GMAKER_SPI_SCLK_PIN;
const int LCD_MOSI_PIN = CONFIG_GMAKER_SPI_MOSI_PIN;
const int LCD_MISO_PIN = CONFIG_GMAKER_SPI_MISO_PIN;
const int LCD_DC_PIN = CONFIG_GMAKER_LCD_DC_PIN;
const int LCD_RST_PIN = CONFIG_GMAKER_LCD_RST_PIN;
const int LCD_CS_PIN = CONFIG_GMAKER_LCD_CS_PIN;







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


// The pixel number in horizontal and vertical
#define LCD_H_RES              240
#define LCD_V_RES              320

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

// PWM configuration for backlight
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // 13-bit resolution
#define LEDC_FREQUENCY          (4000) // 4 kHz

// Static variables
static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;
static uint8_t current_brightness = 80; // Initial brightness 80%
static uint8_t saved_brightness = 80;
static bool screen_is_dimmed = false;
static bool lcd_initialized = false;

// Private function declarations
static esp_err_t lcd_backlight_init(void);
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *color_map);
static void lvgl_tick_timer_cb(void *arg);

esp_err_t lcd_init(void) {

    ESP_LOGI(TAG, "Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GMAKER_LCD_BACKLIGHT_PIN
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_LOGI(TAG, "Initialize SPI bus");
        spi_bus_config_t buscfg = {
        .sclk_io_num = GMAKER_SPI_SCLK_PIN,
        .mosi_io_num = GMAKER_SPI_MOSI_PIN,
        .miso_io_num = GMAKER_SPI_MISO_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * 80 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = GMAKER_LCD_DC_PIN,
        .cs_gpio_num = GMAKER_LCD_CS_PIN,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        //.on_color_trans_done = NULL, // Will set this later
        //.user_ctx = NULL,
    };
    // Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install ILI9341 panel driver");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = GMAKER_LCD_RST_PIN,
        .rgb_endian = LCD_RGB_ELEMENT_ORDER_BGR, //LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    // ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, LCD_MIRROR_Y));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false));
    //ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));
    //ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // Initialize PWM for backlight
    ESP_ERROR_CHECK(lcd_backlight_init());
    
    // Set initial brightness
    lcd_set_brightness(current_brightness);

    ESP_LOGI(TAG, "Initialize LVGL library");
    lv_init();
    
    // Create display
    lv_display_t *disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);

                
    
    // Alloc draw buffers used by LVGL
    lv_color_t *buf1 = lv_malloc(LCD_WIDTH * LVGL_DRAW_BUF_LINES * sizeof(lv_color_t));
    assert(buf1);
    lv_color_t *buf2 = lv_malloc(LCD_WIDTH * LVGL_DRAW_BUF_LINES * sizeof(lv_color_t));
    assert(buf2);
    
    // Set draw buffers
    lv_display_set_buffers(disp, buf1, buf2, LCD_WIDTH * LVGL_DRAW_BUF_LINES * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    // set color depth
    // ORIGINAL ->  lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);

    // Set flush callback and user data
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
    lv_display_set_user_data(disp, panel_handle);

    ESP_LOGI(TAG, "Install LVGL tick timer");
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick_timer_cb,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));


    // ORIGINAL ->
    // ESP_LOGI(TAG, "Register io panel event callback for LVGL flush ready notification");
    // const esp_lcd_panel_io_callbacks_t cbs = {
    //    .on_color_trans_done = example_notify_lvgl_flush_ready,
    // };
    // / *Register done callback * /
    // ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, display));

    lcd_initialized = true;
    ESP_LOGI(TAG, "LCD initialization complete");
    
    return ESP_OK;
}

esp_err_t lcd_deinit(void) {
    if (!lcd_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing LCD");
    
    if (panel_handle) {
        esp_lcd_panel_del(panel_handle);
        panel_handle = NULL;
    }
    
    if (io_handle) {
        esp_lcd_panel_io_del(io_handle);
        io_handle = NULL;
    }
    
    spi_bus_free(LCD_HOST);
    
    lcd_initialized = false;
    ESP_LOGI(TAG, "LCD deinitialization complete");
    
    return ESP_OK;
}

static esp_err_t lcd_backlight_init(void) {
    // Configure LEDC timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Configure LEDC channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = GMAKER_LCD_BACKLIGHT_PIN,
        .duty           = 0, // Start off
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    return ESP_OK;
}

esp_err_t lcd_set_brightness(uint8_t brightness) {
    if (brightness > 100) {
        brightness = 100;
    }
    
    // Invert logic: 100% brightness = low duty cycle
    // 0% brightness = high duty cycle (8191)
    uint32_t duty = ((100 - brightness) * 8191) / 100;
    
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
    
    current_brightness = brightness;
    
    ESP_LOGD(TAG, "LCD brightness set to %d%% (inverted duty: %ld)", brightness, duty);
    return ESP_OK;
}

uint8_t lcd_get_brightness(void) {
    return current_brightness;
}

esp_err_t lcd_dim_screen(void) {
    if (!screen_is_dimmed) {
        saved_brightness = current_brightness;
        lcd_set_brightness(5); // Very dim
        screen_is_dimmed = true;
        ESP_LOGD(TAG, "Screen dimmed to 5%%");
    }
    return ESP_OK;
}

esp_err_t lcd_restore_brightness(void) {
    if (screen_is_dimmed) {
        lcd_set_brightness(saved_brightness);
        screen_is_dimmed = false;
        ESP_LOGD(TAG, "Screen brightness restored to %d%%", saved_brightness);
    }
    return ESP_OK;
}

esp_err_t lcd_screen_off(void) {
    saved_brightness = current_brightness;
    lcd_set_brightness(0);
    ESP_LOGD(TAG, "Screen turned off");
    return ESP_OK;
}

bool lcd_is_ready(void) {
    return lcd_initialized && (panel_handle != NULL);
}

// Private callback functions
static void lvgl_tick_timer_cb(void *arg) {
    (void) arg;
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *color_map) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) lv_display_get_user_data(disp);
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;

    // because SPI LCD is big-endian, we need to swap the RGB bytes order
    lv_draw_sw_rgb565_swap(color_map, (offsetx2 + 1 - offsetx1) * (offsety2 + 1 - offsety1));

    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
    lv_display_flush_ready(disp);
}
