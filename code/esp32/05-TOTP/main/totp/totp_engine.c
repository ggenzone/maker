#include "totp_engine.h"
#include "utils/base32.h"
#include "esp_log.h"
#include "mbedtls/md.h"
#include <string.h>
#include <sys/time.h>

static const char *TAG = "totp_engine";

// HMAC-SHA1 implementation using mbedtls
static esp_err_t hmac_sha1(const uint8_t *key, size_t key_len, 
                           const uint8_t *data, size_t data_len,
                           uint8_t *output) {
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    if (md_info == NULL) {
        return ESP_FAIL;
    }

    int ret = mbedtls_md_hmac(md_info, key, key_len, data, data_len, output);
    if (ret != 0) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

/**
static inline uint64_t get_timestamp(void) {
    return esp_timer_get_time() / 1000000ULL;
}*/

// Get current Unix timestamp
static uint64_t get_timestamp(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ESP_LOGI(TAG, "Current time: %ld seconds since the Epoch", tv.tv_sec);
    return (uint64_t)tv.tv_sec;
}

esp_err_t totp_generate_code(const char *secret_b32, uint32_t time_step, uint8_t digits, uint32_t *code) {
    if (secret_b32 == NULL || code == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }

    if (digits < 6 || digits > 8) {
        ESP_LOGE(TAG, "Digits must be 6, 7, or 8");
        return ESP_ERR_INVALID_ARG;
    }

    // Decode Base32 secret
    uint8_t secret[128];
    int secret_len = base32_decode(secret_b32, secret, sizeof(secret));
    if (secret_len <= 0) {
        ESP_LOGE(TAG, "Failed to decode Base32 secret");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Decoded secret length: %d bytes", secret_len);
    ESP_LOGI(TAG, "Decoded secret %.*s", secret_len, secret);

    // Get current time counter
    uint64_t timestamp = get_timestamp();
    uint64_t counter = timestamp / time_step;
    
    ESP_LOGD(TAG, "Timestamp: %llu, Counter: %llu", timestamp, counter);

    // Convert counter to big-endian bytes
    uint8_t counter_bytes[8];
    for (int i = 7; i >= 0; i--) {
        counter_bytes[i] = counter & 0xFF;
        counter >>= 8;
    }

    // Compute HMAC-SHA1
    uint8_t hmac[20];
    esp_err_t err = hmac_sha1(secret, secret_len, counter_bytes, sizeof(counter_bytes), hmac);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HMAC-SHA1 failed");
        return err;
    }

    // Dynamic truncation (RFC 4226)
    int offset = hmac[19] & 0x0F;
    uint32_t binary = ((hmac[offset] & 0x7F) << 24) |
                      ((hmac[offset + 1] & 0xFF) << 16) |
                      ((hmac[offset + 2] & 0xFF) << 8) |
                      (hmac[offset + 3] & 0xFF);

    // Generate code with specified number of digits
    uint32_t modulo = 1;
    for (int i = 0; i < digits; i++) {
        modulo *= 10;
    }
    *code = binary % modulo;

    ESP_LOGI(TAG, "Generated TOTP code: %0*lu", digits, *code);

    return ESP_OK;
}

esp_err_t totp_get_code(const char *secret_b32, uint8_t digits, uint32_t *code) {
    return totp_generate_code(secret_b32, 30, digits, code);
}

uint32_t totp_get_remaining_seconds(uint32_t period) {
    uint64_t timestamp = get_timestamp();
    return period - (timestamp % period);
}
