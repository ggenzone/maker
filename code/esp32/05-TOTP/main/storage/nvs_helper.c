#include "nvs_helper.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "nvs_helper";
static bool nvs_ready = false;

esp_err_t nvs_helper_init(void) {
    if (nvs_ready) {
        ESP_LOGW(TAG, "NVS helper already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing NVS");

    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_LOGW(TAG, "NVS partition needs to be erased, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed: %s", esp_err_to_name(err));
        return err;
    }

    nvs_ready = true;
    ESP_LOGI(TAG, "NVS initialized successfully");
    return ESP_OK;
}

esp_err_t nvs_helper_deinit(void) {
    if (!nvs_ready) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing NVS");
    esp_err_t err = nvs_flash_deinit();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_deinit failed: %s", esp_err_to_name(err));
        return err;
    }

    nvs_ready = false;
    ESP_LOGI(TAG, "NVS deinitialized successfully");
    return ESP_OK;
}

bool nvs_helper_is_ready(void) {
    return nvs_ready;
}

esp_err_t nvs_helper_save(const char *key, const void *data, size_t size) {
    if (!nvs_ready) {
        ESP_LOGE(TAG, "NVS helper not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_set_blob(handle, key, data, size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_blob failed for key '%s': %s", key, esp_err_to_name(err));
        nvs_close(handle);
        return err;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Saved key '%s' (%d bytes)", key, size);
    }

    nvs_close(handle);
    return err;
}

esp_err_t nvs_helper_load(const char *key, void *data, size_t *size) {
    if (!nvs_ready) {
        ESP_LOGE(TAG, "NVS helper not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Namespace doesn't exist yet (first run) - this is normal
        ESP_LOGI(TAG, "Namespace '%s' not found (first run)", NVS_NAMESPACE);
        return ESP_ERR_NVS_NOT_FOUND;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_get_blob(handle, key, data, size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "Key '%s' not found", key);
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_get_blob failed for key '%s': %s", key, esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Loaded key '%s' (%d bytes)", key, *size);
    }

    nvs_close(handle);
    return err;
}

esp_err_t nvs_helper_delete(const char *key) {
    if (!nvs_ready) {
        ESP_LOGE(TAG, "NVS helper not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_erase_key(handle, key);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "Key '%s' not found for deletion", key);
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_erase_key failed for key '%s': %s", key, esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Deleted key '%s'", key);
    }

    nvs_commit(handle);
    nvs_close(handle);
    return err;
}

bool nvs_helper_exists(const char *key) {
    if (!nvs_ready) {
        return false;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        // Namespace doesn't exist yet
        return false;
    } else if (err != ESP_OK) {
        return false;
    }

    size_t size;
    err = nvs_get_blob(handle, key, NULL, &size);
    nvs_close(handle);

    return (err == ESP_OK);
}
