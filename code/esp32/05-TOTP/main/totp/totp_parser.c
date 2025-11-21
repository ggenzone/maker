#include "totp_parser.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static const char *TAG = "totp_parser";

// URL decode helper function
static void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16*a+b;
            src+=3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

// Extract query parameter value
static bool get_query_param(const char *query, const char *param, char *value, size_t value_size) {
    char search[64];
    snprintf(search, sizeof(search), "%s=", param);
    
    const char *start = strstr(query, search);
    if (start == NULL) {
        return false;
    }
    
    start += strlen(search);
    const char *end = strchr(start, '&');
    
    size_t len;
    if (end != NULL) {
        len = end - start;
    } else {
        len = strlen(start);
    }
    
    if (len >= value_size) {
        len = value_size - 1;
    }
    
    strncpy(value, start, len);
    value[len] = '\0';
    
    // URL decode the value
    char decoded[256];
    url_decode(decoded, value);
    strncpy(value, decoded, value_size - 1);
    value[value_size - 1] = '\0';
    
    return true;
}

esp_err_t totp_parse_uri(const char *uri, totp_service_t *service) {
    if (uri == NULL || service == NULL) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }

    // Clear service structure
    memset(service, 0, sizeof(totp_service_t));
    
    // Set defaults
    service->digits = 6;
    service->period = 30;

    // Check if URI starts with "otpauth://totp/"
    const char *prefix = "otpauth://totp/";
    if (strncmp(uri, prefix, strlen(prefix)) != 0) {
        ESP_LOGE(TAG, "Invalid URI prefix, must start with 'otpauth://totp/'");
        return ESP_ERR_INVALID_ARG;
    }

    // Skip prefix
    const char *path = uri + strlen(prefix);
    
    // Find the query string (starts with '?')
    const char *query = strchr(path, '?');
    if (query == NULL) {
        ESP_LOGE(TAG, "No query string found in URI");
        return ESP_ERR_INVALID_ARG;
    }

    // Extract label (path before '?')
    size_t label_len = query - path;
    char label[MAX_SERVICE_NAME_LEN + MAX_ACCOUNT_NAME_LEN];
    if (label_len >= sizeof(label)) {
        label_len = sizeof(label) - 1;
    }
    strncpy(label, path, label_len);
    label[label_len] = '\0';
    
    // URL decode label
    char decoded_label[MAX_SERVICE_NAME_LEN + MAX_ACCOUNT_NAME_LEN];
    url_decode(decoded_label, label);

    // Parse label: can be "Issuer:account" or just "account"
    char *colon = strchr(decoded_label, ':');
    if (colon != NULL) {
        // Format: "Issuer:account"
        *colon = '\0';
        strncpy(service->service_name, decoded_label, MAX_SERVICE_NAME_LEN - 1);
        strncpy(service->account, colon + 1, MAX_ACCOUNT_NAME_LEN - 1);
    } else {
        // Format: just "account"
        strncpy(service->account, decoded_label, MAX_ACCOUNT_NAME_LEN - 1);
        strncpy(service->service_name, decoded_label, MAX_SERVICE_NAME_LEN - 1);
    }

    // Skip '?' in query
    query++;

    // Extract secret (required)
    if (!get_query_param(query, "secret", service->secret, MAX_SECRET_LEN)) {
        ESP_LOGE(TAG, "Secret parameter is required");
        return ESP_ERR_INVALID_ARG;
    }

    // Extract issuer (optional, overrides label issuer)
    char issuer[MAX_ISSUER_LEN];
    if (get_query_param(query, "issuer", issuer, sizeof(issuer))) {
        strncpy(service->issuer, issuer, MAX_ISSUER_LEN - 1);
    } else {
        // Use service_name as issuer if not provided
        strncpy(service->issuer, service->service_name, MAX_ISSUER_LEN - 1);
    }

    // Extract digits (optional)
    char digits_str[4];
    if (get_query_param(query, "digits", digits_str, sizeof(digits_str))) {
        int digits = atoi(digits_str);
        if (digits >= 6 && digits <= 8) {
            service->digits = digits;
        } else {
            ESP_LOGW(TAG, "Invalid digits value %d, using default 6", digits);
        }
    }

    // Extract period (optional)
    char period_str[8];
    if (get_query_param(query, "period", period_str, sizeof(period_str))) {
        int period = atoi(period_str);
        if (period > 0 && period <= 120) {
            service->period = period;
        } else {
            ESP_LOGW(TAG, "Invalid period value %d, using default 30", period);
        }
    }

    ESP_LOGI(TAG, "Parsed URI - Issuer: %s, Account: %s, Digits: %d, Period: %lu",
             service->issuer, service->account, service->digits, service->period);

    return ESP_OK;
}
