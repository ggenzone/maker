#ifndef NVS_HELPER_H
#define NVS_HELPER_H

#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>

#define NVS_NAMESPACE "totp_storage"

/**
 * @brief Initialize NVS helper
 * @return ESP_OK on success
 */
esp_err_t nvs_helper_init(void);

/**
 * @brief Deinitialize NVS helper
 * @return ESP_OK on success
 */
esp_err_t nvs_helper_deinit(void);

/**
 * @brief Check if NVS helper is initialized
 * @return true if NVS helper is ready
 */
bool nvs_helper_is_ready(void);

/**
 * @brief Save data to NVS
 * @param key Key name
 * @param data Data to save
 * @param size Size of data
 * @return ESP_OK on success
 */
esp_err_t nvs_helper_save(const char *key, const void *data, size_t size);

/**
 * @brief Load data from NVS
 * @param key Key name
 * @param data Buffer to load data into
 * @param size Pointer to size (input: buffer size, output: actual data size)
 * @return ESP_OK on success
 */
esp_err_t nvs_helper_load(const char *key, void *data, size_t *size);

/**
 * @brief Delete data from NVS
 * @param key Key name
 * @return ESP_OK on success
 */
esp_err_t nvs_helper_delete(const char *key);

/**
 * @brief Check if key exists in NVS
 * @param key Key name
 * @return true if key exists
 */
bool nvs_helper_exists(const char *key);

#endif // NVS_HELPER_H
