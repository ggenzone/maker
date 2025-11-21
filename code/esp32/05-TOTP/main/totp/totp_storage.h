#ifndef TOTP_STORAGE_H
#define TOTP_STORAGE_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_SERVICE_NAME_LEN    64
#define MAX_ACCOUNT_NAME_LEN    64
#define MAX_SECRET_LEN          128
#define MAX_ISSUER_LEN          64
#define MAX_SERVICES            20

typedef struct {
    char service_name[MAX_SERVICE_NAME_LEN];
    char account[MAX_ACCOUNT_NAME_LEN];
    char secret[MAX_SECRET_LEN];
    char issuer[MAX_ISSUER_LEN];
    uint8_t digits;         // Usually 6 or 8
    uint32_t period;        // Usually 30 seconds
} totp_service_t;

/**
 * @brief Initialize TOTP storage
 * @return ESP_OK on success
 */
esp_err_t totp_storage_init(void);

/**
 * @brief Deinitialize TOTP storage
 * @return ESP_OK on success
 */
esp_err_t totp_storage_deinit(void);

/**
 * @brief Check if TOTP storage is initialized
 * @return true if ready
 */
bool totp_storage_is_ready(void);

/**
 * @brief Add a new TOTP service
 * @param service Service to add
 * @return ESP_OK on success, ESP_ERR_NO_MEM if storage is full
 */
esp_err_t totp_storage_add(const totp_service_t *service);

/**
 * @brief Get service by index
 * @param index Service index (0 to count-1)
 * @param service Output service structure
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if index out of range
 */
esp_err_t totp_storage_get(uint8_t index, totp_service_t *service);

/**
 * @brief Get total number of services
 * @return Number of services stored
 */
uint8_t totp_storage_count(void);

/**
 * @brief Delete service by index
 * @param index Service index to delete
 * @return ESP_OK on success
 */
esp_err_t totp_storage_delete(uint8_t index);

/**
 * @brief Clear all services
 * @return ESP_OK on success
 */
esp_err_t totp_storage_clear(void);

/**
 * @brief Get all services as JSON string
 * @return Allocated JSON string (caller must free), or NULL on error
 */
char* totp_storage_list_json(void);

#endif // TOTP_STORAGE_H
