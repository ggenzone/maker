#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include "esp_err.h"
#include <stdbool.h>

/**
 * @brief Initialize and start HTTP server
 * @return ESP_OK on success
 */
esp_err_t server_init(void);

/**
 * @brief Stop and deinitialize HTTP server
 * @return ESP_OK on success
 */
esp_err_t server_deinit(void);

/**
 * @brief Check if server is running
 * @return true if server is running
 */
bool server_is_running(void);

#endif // NETWORK_SERVER_H
