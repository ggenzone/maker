#ifndef I2C_HELPER_H
#define I2C_HELPER_H

#include "esp_err.h"
#include "driver/i2c_master.h"
#include <stdbool.h>

#define I2C_MASTER_SCL_IO           CONFIG_GMAKER_I2C_SCL // SCL GPIO
#define I2C_MASTER_SDA_IO           CONFIG_GMAKER_I2C_SDA // SDA GPIO
#define I2C_MASTER_NUM              I2C_NUM_0   // I2C port number
#define I2C_MASTER_FREQ_HZ          100000      // 100 kHz

/**
 * @brief Initialize I2C master
 * @return ESP_OK on success
 */
esp_err_t i2c_helper_init(void);

/**
 * @brief Deinitialize I2C master
 * @return ESP_OK on success
 */
esp_err_t i2c_helper_deinit(void);

/**
 * @brief Check if I2C master is initialized
 * @return true if I2C is ready
 */
bool i2c_helper_is_ready(void);

/**
 * @brief Get I2C master bus handle
 * @return I2C bus handle
 */
i2c_master_bus_handle_t i2c_helper_get_bus_handle(void);

/**
 * @brief Scan I2C bus for devices
 */
void i2c_helper_scan(void);

#endif // I2C_HELPER_H
