#ifndef TOTP_ENGINE_H
#define TOTP_ENGINE_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

/**
 * @brief Generate TOTP code from secret
 * @param secret_b32 Base32-encoded secret string
 * @param time_step Time step in seconds (usually 30)
 * @param digits Number of digits in code (6 or 8)
 * @param code Output TOTP code
 * @return ESP_OK on success
 */
esp_err_t totp_generate_code(const char *secret_b32, uint32_t time_step, uint8_t digits, uint32_t *code);

/**
 * @brief Get current TOTP code using current time
 * @param secret_b32 Base32-encoded secret string
 * @param digits Number of digits (6 or 8)
 * @param code Output TOTP code
 * @return ESP_OK on success
 */
esp_err_t totp_get_code(const char *secret_b32, uint8_t digits, uint32_t *code);

/**
 * @brief Get remaining seconds until code changes
 * @param period Time period in seconds (usually 30)
 * @return Seconds remaining (0 to period-1)
 */
uint32_t totp_get_remaining_seconds(uint32_t period);

#endif // TOTP_ENGINE_H
