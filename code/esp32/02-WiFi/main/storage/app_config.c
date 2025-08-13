#include "app_config.h"
#include "storage.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "APP_CONFIG";

// Configuration keys
#define KEY_WIFI_ENABLED        "wifi_en"
#define KEY_WIFI_CONNECTED      "wifi_conn"
#define KEY_LCD_BRIGHTNESS      "lcd_bright"
#define KEY_SCREEN_TIMEOUT      "scr_timeout"
#define KEY_AUTO_DIM_ENABLED    "auto_dim"
#define KEY_DIM_BRIGHTNESS      "dim_bright"


// Default configuration values
static const app_config_t default_config = {
    .wifi_enabled = false,
    .wifi_connected = false,        // Dynamic
    .lcd_brightness = 80,
    .screen_timeout_sec = 300,      // Dynamic | 5 minutes
    .auto_dim_enabled = true,
    .dim_brightness = 0             // Dynamic
};

// Current configuration
static app_config_t current_config;
static bool config_initialized = false;

esp_err_t app_config_init(void) {
    ESP_LOGI(TAG, "Initializing application configuration");
    
    // Copy default values
    memcpy(&current_config, &default_config, sizeof(app_config_t));
    
    config_initialized = true;
    ESP_LOGI(TAG, "Application configuration initialized");
    
    return ESP_OK;
}

esp_err_t app_config_deinit(void) {
    if (!config_initialized) {
        return ESP_OK;
    }
    
    ESP_LOGI(TAG, "Deinitializing application configuration");
    config_initialized = false;
    
    return ESP_OK;
}

esp_err_t app_config_load(void) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Configuration not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Loading configuration from storage");
    
    // Load each setting individually with defaults
    storage_get_bool(KEY_WIFI_ENABLED, &current_config.wifi_enabled, default_config.wifi_enabled);
    storage_get_u8(KEY_LCD_BRIGHTNESS, &current_config.lcd_brightness, default_config.lcd_brightness);
    storage_get_bool(KEY_AUTO_DIM_ENABLED, &current_config.auto_dim_enabled, default_config.auto_dim_enabled);
    
    ESP_LOGI(TAG, "Configuration loaded successfully");
    ESP_LOGI(TAG, "  WiFi enabled: %s", current_config.wifi_enabled ? "yes" : "no");
    ESP_LOGI(TAG, "  LCD brightness: %u%%", current_config.lcd_brightness);
    ESP_LOGI(TAG, "  Screen timeout: %u sec", current_config.screen_timeout_sec);
    ESP_LOGI(TAG, "  Auto-dim: %s", current_config.auto_dim_enabled ? "yes" : "no");
    ESP_LOGI(TAG, "  Dim brightness: %u%%", current_config.dim_brightness);

    return ESP_OK;
}

esp_err_t app_config_save(void) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Configuration not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Saving configuration to storage");
    
    esp_err_t ret = ESP_OK;
    
    // Save each setting
    if (storage_set_bool(KEY_WIFI_ENABLED, current_config.wifi_enabled) != ESP_OK) ret = ESP_FAIL;
    if (storage_set_u8(KEY_LCD_BRIGHTNESS, current_config.lcd_brightness) != ESP_OK) ret = ESP_FAIL;
    if (storage_set_bool(KEY_AUTO_DIM_ENABLED, current_config.auto_dim_enabled) != ESP_OK) ret = ESP_FAIL;


    // Commit changes
    if (storage_commit() != ESP_OK) ret = ESP_FAIL;
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Configuration saved successfully");
    } else {
        ESP_LOGE(TAG, "Failed to save some configuration values");
    }
    
    return ret;
}

esp_err_t app_config_reset_to_defaults(void) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Configuration not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Resetting configuration to factory defaults");
    
    // Copy default values
    memcpy(&current_config, &default_config, sizeof(app_config_t));
    
    // Save to storage
    return app_config_save();
}

esp_err_t app_config_get(app_config_t *config) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Configuration not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (config == NULL) {
        ESP_LOGE(TAG, "Config pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(config, &current_config, sizeof(app_config_t));
    return ESP_OK;
}

esp_err_t app_config_set(const app_config_t *config) {
    if (!config_initialized) {
        ESP_LOGE(TAG, "Configuration not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (config == NULL) {
        ESP_LOGE(TAG, "Config pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    memcpy(&current_config, config, sizeof(app_config_t));
    return ESP_OK;
}

// Individual getters and setters
bool app_config_get_wifi_enabled(void) {
    return current_config.wifi_enabled;
}

esp_err_t app_config_set_wifi_enabled(bool enabled) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    current_config.wifi_enabled = enabled;
    return ESP_OK;
}

bool app_config_get_wifi_connected(void) {
    return current_config.wifi_connected;
}

esp_err_t app_config_set_wifi_connected(bool connected) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    current_config.wifi_connected = connected;
    return ESP_OK;
}

uint8_t app_config_get_lcd_brightness(void) {
    return current_config.lcd_brightness;
}

esp_err_t app_config_set_lcd_brightness(uint8_t brightness) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    if (brightness > 100) brightness = 100;
    current_config.lcd_brightness = brightness;
    return ESP_OK;
}

uint16_t app_config_get_screen_timeout(void) {
    return current_config.screen_timeout_sec;
}

esp_err_t app_config_set_screen_timeout(uint16_t timeout_sec) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    current_config.screen_timeout_sec = timeout_sec;
    return ESP_OK;
}

bool app_config_get_auto_dim_enabled(void) {
    return current_config.auto_dim_enabled;
}

esp_err_t app_config_set_auto_dim_enabled(bool enabled) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    current_config.auto_dim_enabled = enabled;
    return ESP_OK;
}

uint8_t app_config_get_dim_brightness(void) {
    return current_config.dim_brightness;
}

esp_err_t app_config_set_dim_brightness(uint8_t brightness) {
    if (!config_initialized) return ESP_ERR_INVALID_STATE;
    if (brightness > 100) brightness = 100;
    current_config.dim_brightness = brightness;
    return ESP_OK;
}





