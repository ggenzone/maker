#ifndef HARDWARE_H
#define HARDWARE_H

#include "esp_err.h"
#include "i2c_helper.h"
#include "wifi_helper.h"

#include <stdbool.h>

/**
 * @brief Initialize all hardware components
 * Call this once at startup before using any hardware
 */
esp_err_t hardware_init(void);

/**
 * @brief Deinitialize all hardware components
 */
esp_err_t hardware_deinit(void);

/**
 * @brief Get hardware status
 * @return true if all hardware is initialized correctly
 */
bool hardware_is_ready(void);

#endif // HARDWARE_H
