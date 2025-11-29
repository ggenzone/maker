#ifndef SYSTEM_INIT_H
#define SYSTEM_INIT_H

#include "esp_err.h"

/**
 * @brief Initialize core system components
 * - NVS (Non-Volatile Storage)
 * - Event loop
 * - Network interface
 * - Display chip information
 * 
 * Call this first before any other hardware initialization
 * 
 * @return ESP_OK on success
 */
esp_err_t system_init(void);

#endif // SYSTEM_INIT_H
