#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

/**
 * @brief Initialize the storage system (NVS)
 * @return ESP_OK on success
 */
esp_err_t storage_init(void);

/**
 * @brief Deinitialize the storage system
 * @return ESP_OK on success
 */
esp_err_t storage_deinit(void);

/**
 * @brief Check if storage system is ready
 * @return true if storage is ready for use
 */
bool storage_is_ready(void);

/**
 * @brief Save a boolean value to storage
 * @param key The key name
 * @param value The boolean value to save
 * @return ESP_OK on success
 */
esp_err_t storage_set_bool(const char *key, bool value);

/**
 * @brief Load a boolean value from storage
 * @param key The key name
 * @param value Pointer to store the loaded value
 * @param default_value Default value if key not found
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if key doesn't exist
 */
esp_err_t storage_get_bool(const char *key, bool *value, bool default_value);

/**
 * @brief Save an 8-bit integer to storage
 * @param key The key name
 * @param value The uint8_t value to save
 * @return ESP_OK on success
 */
esp_err_t storage_set_u8(const char *key, uint8_t value);

/**
 * @brief Load an 8-bit integer from storage
 * @param key The key name
 * @param value Pointer to store the loaded value
 * @param default_value Default value if key not found
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if key doesn't exist
 */
esp_err_t storage_get_u8(const char *key, uint8_t *value, uint8_t default_value);

/**
 * @brief Save a 16-bit integer to storage
 * @param key The key name
 * @param value The uint16_t value to save
 * @return ESP_OK on success
 */
esp_err_t storage_set_u16(const char *key, uint16_t value);

/**
 * @brief Load a 16-bit integer from storage
 * @param key The key name
 * @param value Pointer to store the loaded value
 * @param default_value Default value if key not found
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if key doesn't exist
 */
esp_err_t storage_get_u16(const char *key, uint16_t *value, uint16_t default_value);

/**
 * @brief Save a 32-bit integer to storage
 * @param key The key name
 * @param value The uint32_t value to save
 * @return ESP_OK on success
 */
esp_err_t storage_set_u32(const char *key, uint32_t value);

/**
 * @brief Load a 32-bit integer from storage
 * @param key The key name
 * @param value Pointer to store the loaded value
 * @param default_value Default value if key not found
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if key doesn't exist
 */
esp_err_t storage_get_u32(const char *key, uint32_t *value, uint32_t default_value);

/**
 * @brief Save a string to storage
 * @param key The key name
 * @param value The string value to save
 * @return ESP_OK on success
 */
esp_err_t storage_set_string(const char *key, const char *value);

/**
 * @brief Load a string from storage
 * @param key The key name
 * @param buffer Buffer to store the loaded string
 * @param buffer_size Size of the buffer
 * @param default_value Default value if key not found (can be NULL)
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if key doesn't exist
 */
esp_err_t storage_get_string(const char *key, char *buffer, size_t buffer_size, const char *default_value);

/**
 * @brief Save a binary blob to storage
 * @param key The key name
 * @param data Pointer to the data to save
 * @param length Length of the data
 * @return ESP_OK on success
 */
esp_err_t storage_set_blob(const char *key, const void *data, size_t length);

/**
 * @brief Load a binary blob from storage
 * @param key The key name
 * @param buffer Buffer to store the loaded data
 * @param buffer_size Size of the buffer
 * @param actual_size Pointer to store the actual size read (can be NULL)
 * @return ESP_OK on success, ESP_ERR_NOT_FOUND if key doesn't exist
 */
esp_err_t storage_get_blob(const char *key, void *buffer, size_t buffer_size, size_t *actual_size);

/**
 * @brief Erase a key from storage
 * @param key The key name to erase
 * @return ESP_OK on success
 */
esp_err_t storage_erase_key(const char *key);

/**
 * @brief Erase all keys in the storage namespace
 * @return ESP_OK on success
 */
esp_err_t storage_erase_all(void);

/**
 * @brief Force commit all pending writes to storage
 * @return ESP_OK on success
 */
esp_err_t storage_commit(void);

#endif // STORAGE_H
