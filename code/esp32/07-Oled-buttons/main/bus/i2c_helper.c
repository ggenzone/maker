#include "i2c_helper.h"
#include "driver/i2c_master.h"
#include "esp_log.h"

static const char *TAG = "i2c_helper";
static bool i2c_ready = false;
static i2c_master_bus_handle_t bus_handle = NULL;

esp_err_t i2c_helper_init(void) {
    if (i2c_ready) return ESP_OK;
    
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_MASTER_NUM,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    
    esp_err_t err = i2c_new_master_bus(&i2c_bus_config, &bus_handle);
    if (err == ESP_OK) {
        i2c_ready = true;
        ESP_LOGI(TAG, "I2C master bus initialized successfully");
    } else {
        ESP_LOGE(TAG, "i2c_new_master_bus failed: %s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t i2c_helper_deinit(void) {
    if (!i2c_ready) return ESP_OK;
    
    esp_err_t err = i2c_del_master_bus(bus_handle);
    if (err == ESP_OK) {
        bus_handle = NULL;
        i2c_ready = false;
        ESP_LOGI(TAG, "I2C master bus deinitialized");
    } else {
        ESP_LOGE(TAG, "i2c_del_master_bus failed: %s", esp_err_to_name(err));
    }
    return err;
}

bool i2c_helper_is_ready(void) {
    return i2c_ready;
}

i2c_master_bus_handle_t i2c_helper_get_bus_handle(void) {
    return bus_handle;
}

void i2c_helper_scan(void) {
    ESP_LOGI(TAG, "Escaneando bus I2C...");
    
    if (!i2c_ready) {
        ESP_LOGE(TAG, "I2C bus not initialized");
        return;
    }
    
    for (uint8_t addr = 1; addr < 127; addr++) {
        esp_err_t ret = i2c_master_probe(bus_handle, addr, 50);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Dispositivo encontrado en 0x%02X", addr);
        }
    }
    ESP_LOGI(TAG, "Escaneo I2C finalizado.");
}
