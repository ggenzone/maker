#ifndef WIFI_HELPER_H
#define WIFI_HELPER_H

#include "esp_err.h"
#include <stdbool.h>

// WiFi Configuration - Use Kconfig values
#define WIFI_MAXIMUM_RETRY      5
#define WIFI_CONNECTED_BIT      BIT0
#define WIFI_FAIL_BIT           BIT1

/**
 * @brief Initialize WiFi helper
 * @return ESP_OK on success
 */
esp_err_t wifi_helper_init(void);

/**
 * @brief Deinitialize WiFi helper
 * @return ESP_OK on success
 */
esp_err_t wifi_helper_deinit(void);

/**
 * @brief Check if WiFi helper is initialized
 * @return true if WiFi helper is ready
 */
bool wifi_helper_is_ready(void);

/**
 * @brief Connect to WiFi network
 * @param ssid WiFi SSID (NULL to use default WIFI_SSID)
 * @param password WiFi password (NULL to use default WIFI_PASS)
 * @return ESP_OK on success
 */
esp_err_t wifi_helper_connect(const char *ssid, const char *password);

/**
 * @brief Check if WiFi is connected
 * @return true if connected to WiFi network
 */
bool wifi_helper_is_connected(void);

#endif // WIFI_HELPER_H
