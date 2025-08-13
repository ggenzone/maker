#include "storage.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "STORAGE";
#define STORAGE_NAMESPACE "app_config"

// Static variables
static bool storage_initialized = false;
static nvs_handle_t storage_handle = 0;

esp_err_t storage_init(void) {
    ESP_LOGI(TAG, "Initializing storage system");
    
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_LOGW(TAG, "NVS partition needs to be erased, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(err));
        return err;
    }
    
    // Open storage namespace
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &storage_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS namespace '%s': %s", STORAGE_NAMESPACE, esp_err_to_name(err));
        return err;
    }
    
    storage_initialized = true;
    ESP_LOGI(TAG, "Storage system initialized successfully");
    
    return ESP_OK;
}

esp_err_t storage_deinit(void) {
    if (!storage_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing storage system");
    
    if (storage_handle != 0) {
        nvs_close(storage_handle);
        storage_handle = 0;
    }
    
    // Note: We don't call nvs_flash_deinit() as other components might be using NVS
    
    storage_initialized = false;
    ESP_LOGI(TAG, "Storage system deinitialized");
    
    return ESP_OK;
}

bool storage_is_ready(void) {
    return storage_initialized && (storage_handle != 0);
}

esp_err_t storage_set_bool(const char *key, bool value) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t val = value ? 1 : 0;
    esp_err_t err = nvs_set_u8(storage_handle, key, val);
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Saved bool %s = %s", key, value ? "true" : "false");
    } else {
        ESP_LOGE(TAG, "Failed to save bool %s: %s", key, esp_err_to_name(err));
    }
    
    return err;
}

esp_err_t storage_get_bool(const char *key, bool *value, bool default_value) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (value == NULL) {
        ESP_LOGE(TAG, "Value pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t val = 0;
    esp_err_t err = nvs_get_u8(storage_handle, key, &val);
    
    if (err == ESP_OK) {
        *value = (val != 0);
        ESP_LOGD(TAG, "Loaded bool %s = %s", key, *value ? "true" : "false");
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        *value = default_value;
        ESP_LOGD(TAG, "Key %s not found, using default: %s", key, default_value ? "true" : "false");
    } else {
        ESP_LOGE(TAG, "Failed to load bool %s: %s", key, esp_err_to_name(err));
        *value = default_value;
    }
    
    return err;
}

esp_err_t storage_set_u8(const char *key, uint8_t value) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = nvs_set_u8(storage_handle, key, value);
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Saved u8 %s = %u", key, value);
    } else {
        ESP_LOGE(TAG, "Failed to save u8 %s: %s", key, esp_err_to_name(err));
    }
    
    return err;
}

esp_err_t storage_get_u8(const char *key, uint8_t *value, uint8_t default_value) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (value == NULL) {
        ESP_LOGE(TAG, "Value pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t err = nvs_get_u8(storage_handle, key, value);
    
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Loaded u8 %s = %u", key, *value);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        *value = default_value;
        ESP_LOGD(TAG, "Key %s not found, using default: %u", key, default_value);
    } else {
        ESP_LOGE(TAG, "Failed to load u8 %s: %s", key, esp_err_to_name(err));
        *value = default_value;
    }
    
    return err;
}

esp_err_t storage_set_u16(const char *key, uint16_t value) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = nvs_set_u16(storage_handle, key, value);
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Saved u16 %s = %u", key, value);
    } else {
        ESP_LOGE(TAG, "Failed to save u16 %s: %s", key, esp_err_to_name(err));
    }
    
    return err;
}

esp_err_t storage_get_u16(const char *key, uint16_t *value, uint16_t default_value) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (value == NULL) {
        ESP_LOGE(TAG, "Value pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t err = nvs_get_u16(storage_handle, key, value);
    
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Loaded u16 %s = %u", key, *value);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        *value = default_value;
        ESP_LOGD(TAG, "Key %s not found, using default: %u", key, default_value);
    } else {
        ESP_LOGE(TAG, "Failed to load u16 %s: %s", key, esp_err_to_name(err));
        *value = default_value;
    }
    
    return err;
}

esp_err_t storage_set_u32(const char *key, uint32_t value) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = nvs_set_u32(storage_handle, key, value);
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Saved u32 %s = %lu", key, value);
    } else {
        ESP_LOGE(TAG, "Failed to save u32 %s: %s", key, esp_err_to_name(err));
    }
    
    return err;
}

esp_err_t storage_get_u32(const char *key, uint32_t *value, uint32_t default_value) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (value == NULL) {
        ESP_LOGE(TAG, "Value pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t err = nvs_get_u32(storage_handle, key, value);
    
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Loaded u32 %s = %lu", key, *value);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        *value = default_value;
        ESP_LOGD(TAG, "Key %s not found, using default: %lu", key, default_value);
    } else {
        ESP_LOGE(TAG, "Failed to load u32 %s: %s", key, esp_err_to_name(err));
        *value = default_value;
    }
    
    return err;
}

esp_err_t storage_set_string(const char *key, const char *value) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (value == NULL) {
        ESP_LOGE(TAG, "Value string is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t err = nvs_set_str(storage_handle, key, value);
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Saved string %s = '%s'", key, value);
    } else {
        ESP_LOGE(TAG, "Failed to save string %s: %s", key, esp_err_to_name(err));
    }
    
    return err;
}

esp_err_t storage_get_string(const char *key, char *buffer, size_t buffer_size, const char *default_value) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (buffer == NULL || buffer_size == 0) {
        ESP_LOGE(TAG, "Invalid buffer parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    size_t required_size = 0;
    esp_err_t err = nvs_get_str(storage_handle, key, NULL, &required_size);
    
    if (err == ESP_OK) {
        if (required_size <= buffer_size) {
            err = nvs_get_str(storage_handle, key, buffer, &required_size);
            if (err == ESP_OK) {
                ESP_LOGD(TAG, "Loaded string %s = '%s'", key, buffer);
            } else {
                ESP_LOGE(TAG, "Failed to load string %s: %s", key, esp_err_to_name(err));
            }
        } else {
            ESP_LOGE(TAG, "Buffer too small for string %s: need %zu, have %zu", key, required_size, buffer_size);
            err = ESP_ERR_INVALID_SIZE;
        }
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        if (default_value != NULL) {
            strncpy(buffer, default_value, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            ESP_LOGD(TAG, "Key %s not found, using default: '%s'", key, default_value);
        } else {
            buffer[0] = '\0';
            ESP_LOGD(TAG, "Key %s not found, no default provided", key);
        }
    } else {
        ESP_LOGE(TAG, "Failed to check string %s: %s", key, esp_err_to_name(err));
        if (default_value != NULL) {
            strncpy(buffer, default_value, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
        } else {
            buffer[0] = '\0';
        }
    }
    
    return err;
}

esp_err_t storage_set_blob(const char *key, const void *data, size_t length) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (data == NULL || length == 0) {
        ESP_LOGE(TAG, "Invalid blob parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t err = nvs_set_blob(storage_handle, key, data, length);
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Saved blob %s (%zu bytes)", key, length);
    } else {
        ESP_LOGE(TAG, "Failed to save blob %s: %s", key, esp_err_to_name(err));
    }
    
    return err;
}

esp_err_t storage_get_blob(const char *key, void *buffer, size_t buffer_size, size_t *actual_size) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (buffer == NULL || buffer_size == 0) {
        ESP_LOGE(TAG, "Invalid buffer parameters");
        return ESP_ERR_INVALID_ARG;
    }
    
    size_t required_size = 0;
    esp_err_t err = nvs_get_blob(storage_handle, key, NULL, &required_size);
    
    if (err == ESP_OK) {
        if (required_size <= buffer_size) {
            err = nvs_get_blob(storage_handle, key, buffer, &required_size);
            if (err == ESP_OK) {
                if (actual_size != NULL) {
                    *actual_size = required_size;
                }
                ESP_LOGD(TAG, "Loaded blob %s (%zu bytes)", key, required_size);
            } else {
                ESP_LOGE(TAG, "Failed to load blob %s: %s", key, esp_err_to_name(err));
            }
        } else {
            ESP_LOGE(TAG, "Buffer too small for blob %s: need %zu, have %zu", key, required_size, buffer_size);
            err = ESP_ERR_INVALID_SIZE;
        }
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGD(TAG, "Blob %s not found", key);
        if (actual_size != NULL) {
            *actual_size = 0;
        }
    } else {
        ESP_LOGE(TAG, "Failed to check blob %s: %s", key, esp_err_to_name(err));
        if (actual_size != NULL) {
            *actual_size = 0;
        }
    }
    
    return err;
}

esp_err_t storage_erase_key(const char *key) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = nvs_erase_key(storage_handle, key);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Erased key: %s", key);
    } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key %s not found for erasure", key);
    } else {
        ESP_LOGE(TAG, "Failed to erase key %s: %s", key, esp_err_to_name(err));
    }
    
    return err;
}

esp_err_t storage_erase_all(void) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = nvs_erase_all(storage_handle);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Erased all keys in namespace");
    } else {
        ESP_LOGE(TAG, "Failed to erase all keys: %s", esp_err_to_name(err));
    }
    
    return err;
}

esp_err_t storage_commit(void) {
    if (!storage_is_ready()) {
        ESP_LOGE(TAG, "Storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = nvs_commit(storage_handle);
    if (err == ESP_OK) {
        ESP_LOGD(TAG, "Storage committed successfully");
    } else {
        ESP_LOGE(TAG, "Failed to commit storage: %s", esp_err_to_name(err));
    }
    
    return err;
}
