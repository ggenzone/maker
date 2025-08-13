#include "touch_helper.h"
#include "lcd_helper.h"
#include "esp_lcd_touch_xpt2046.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "lvgl.h"
#include "config.h"

static const char *TAG = "TOUCH_HELPER";

// GPIO pin assignments from Kconfig
const int TOUCH_CS_PIN = CONFIG_GMAKER_TOUCH_CS_PIN;
const int TOUCH_IRQ_PIN = -1; // Not used in this implementation

// Static variables
static esp_lcd_touch_handle_t touch_handle = NULL;
static bool touch_initialized = false;

// Private function declarations
static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data);

esp_err_t lcd_touch_init(void) {
    ESP_LOGI(TAG, "Initialize touch controller XPT2046");
    
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_spi_config_t tp_io_config = ESP_LCD_TOUCH_IO_SPI_XPT2046_CONFIG(GMAKER_TOUCH_CS_PIN);
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &tp_io_config, &tp_io_handle));

    esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_WIDTH,
        .y_max = LCD_HEIGHT,
        .rst_gpio_num = -1,
        .int_gpio_num = TOUCH_IRQ_PIN,
        //.levels = {
        //    .reset = 0,
        //    .interrupt = 0,
        //},
        .flags = {
            .swap_xy = 0, //1,
            .mirror_x = 0,
            .mirror_y = 1,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_touch_new_spi_xpt2046(tp_io_handle, &tp_cfg, &touch_handle));

    // Register touch input device in LVGL
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);
    lv_indev_set_user_data(indev, touch_handle);

    touch_initialized = true;
    ESP_LOGI(TAG, "Touch controller initialization complete");
    
    return ESP_OK;
}

esp_err_t touch_deinit(void) {
    if (!touch_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing touch controller");
    
    if (touch_handle) {
        esp_lcd_touch_del(touch_handle);
        touch_handle = NULL;
    }
    
    touch_initialized = false;
    ESP_LOGI(TAG, "Touch controller deinitialization complete");
    
    return ESP_OK;
}

bool touch_is_ready(void) {
    return touch_initialized && (touch_handle != NULL);
}

esp_err_t touch_calibrate(void) {
    if (!touch_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Touch calibration not implemented yet");
    // TODO: Implement touch calibration routine
    
    return ESP_OK;
}

esp_lcd_touch_handle_t touch_get_handle(void) {
    return touch_handle;
}

// Private callback functions
static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    //esp_lcd_touch_handle_t *touch_handle = lv_indev_get_user_data(indev);
    esp_lcd_touch_handle_t tp = (esp_lcd_touch_handle_t)lv_indev_get_user_data(indev);

    assert(tp);
    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    esp_lcd_touch_read_data(tp);

    bool touchpad_pressed = esp_lcd_touch_get_coordinates(tp, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

    if (touchpad_pressed && touchpad_cnt > 0) {
        if (!lcd_is_dimmed()) {
            data->point.x = touchpad_x[0];
            data->point.y = touchpad_y[0];
            data->state = LV_INDEV_STATE_PRESSED;
        }

        // Reset auto-dim timer on touch
        lcd_reset_autodim_timer();
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
