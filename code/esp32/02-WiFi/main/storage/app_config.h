#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

/**
 * @brief Application configuration structure
 */
typedef struct {
    // WiFi settings
    bool wifi_enabled;
    bool wifi_connected;
    
    // Display settings
    uint8_t lcd_brightness;        // 0-100%
    uint16_t screen_timeout_sec;   // Screen timeout in seconds (0 = never)
    bool auto_dim_enabled;         // Auto-dim before turning off
    uint8_t dim_brightness;        // Brightness level for dimming (0-100%)
    
    
} app_config_t;

/**
 * @brief Initialize the application configuration system
 * @return ESP_OK on success
 */
esp_err_t app_config_init(void);

/**
 * @brief Deinitialize the application configuration system
 * @return ESP_OK on success
 */
esp_err_t app_config_deinit(void);

/**
 * @brief Load configuration from storage
 * @return ESP_OK on success
 */
esp_err_t app_config_load(void);

/**
 * @brief Save configuration to storage
 * @return ESP_OK on success
 */
esp_err_t app_config_save(void);

/**
 * @brief Reset configuration to factory defaults
 * @return ESP_OK on success
 */
esp_err_t app_config_reset_to_defaults(void);

/**
 * @brief Get a copy of the current configuration
 * @param config Pointer to configuration structure to fill
 * @return ESP_OK on success
 */
esp_err_t app_config_get(app_config_t *config);

/**
 * @brief Set the entire configuration
 * @param config Pointer to new configuration
 * @return ESP_OK on success
 */
esp_err_t app_config_set(const app_config_t *config);

// Individual setting getters and setters
bool app_config_get_wifi_enabled(void);
esp_err_t app_config_set_wifi_enabled(bool enabled);

bool app_config_get_wifi_connected(void);
esp_err_t app_config_set_wifi_connected(bool connected);

uint8_t app_config_get_lcd_brightness(void);
esp_err_t app_config_set_lcd_brightness(uint8_t brightness);

bool app_config_get_auto_dim_enabled(void);
esp_err_t app_config_set_auto_dim_enabled(bool enabled);


#endif // APP_CONFIG_H
