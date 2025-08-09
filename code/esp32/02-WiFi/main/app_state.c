#include "app_state.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "app_state";
#define STORAGE_NAMESPACE "app_config"
#define WIFI_ENABLED_KEY  "wifi_enabled"

app_state_t app_state = {
    .wifi_connected = false,
    .wifi_enabled = false  // será sobreescrito por load
};


void app_state_save(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return;
    }

    err = nvs_set_u8(handle, WIFI_ENABLED_KEY, app_state.wifi_enabled ? 1 : 0);
    if (err == ESP_OK) {
        nvs_commit(handle);
        ESP_LOGI(TAG, "Saved wifi_enabled = %d", app_state.wifi_enabled);
    } else {
        ESP_LOGE(TAG, "Failed to write wifi_enabled: %s", esp_err_to_name(err));
    }

    nvs_close(handle);
}

void app_state_load(void) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS not ready or key missing, defaulting wifi_enabled = false");
        app_state.wifi_enabled = false;
        return;
    }

    uint8_t enabled = 0;
    err = nvs_get_u8(handle, WIFI_ENABLED_KEY, &enabled);
    if (err == ESP_OK) {
        app_state.wifi_enabled = (enabled != 0);
        ESP_LOGI(TAG, "Loaded wifi_enabled = %d", app_state.wifi_enabled);
    } else {
        ESP_LOGW(TAG, "Key not found, defaulting wifi_enabled = false");
        app_state.wifi_enabled = false;
    }

    nvs_close(handle);
}
