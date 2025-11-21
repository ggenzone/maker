#include "totp_storage.h"
#include "storage/nvs_helper.h"
#include "esp_log.h"
#include "nvs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char *TAG = "totp_storage";
static bool storage_ready = false;

#define NVS_KEY_COUNT "svc_count"
#define NVS_KEY_SERVICE_PREFIX "svc_"

// In-memory cache of services
static totp_service_t services[MAX_SERVICES];
static uint8_t service_count = 0;

// Helper function to generate service key
static void get_service_key(uint8_t index, char *key, size_t key_size) {
    snprintf(key, key_size, "%s%d", NVS_KEY_SERVICE_PREFIX, index);
}

// Load all services from NVS
static esp_err_t load_services_from_nvs(void) {
    // Load service count
    size_t size = sizeof(uint8_t);
    esp_err_t err = nvs_helper_load(NVS_KEY_COUNT, &service_count, &size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "No services found in NVS, starting fresh");
        service_count = 0;
        return ESP_OK;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to load service count: %s", esp_err_to_name(err));
        return err;
    }

    if (service_count > MAX_SERVICES) {
        ESP_LOGW(TAG, "Service count %d exceeds max %d, truncating", service_count, MAX_SERVICES);
        service_count = MAX_SERVICES;
    }

    ESP_LOGI(TAG, "Loading %d services from NVS", service_count);

    // Load each service
    for (uint8_t i = 0; i < service_count; i++) {
        char key[16];
        get_service_key(i, key, sizeof(key));
        
        size = sizeof(totp_service_t);
        err = nvs_helper_load(key, &services[i], &size);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to load service %d: %s", i, esp_err_to_name(err));
            // Continue loading other services
        } else {
            ESP_LOGI(TAG, "Loaded service %d: %s (%s)", i, services[i].issuer, services[i].account);
        }
    }

    return ESP_OK;
}

// Save all services to NVS
static esp_err_t save_services_to_nvs(void) {
    // Save service count
    esp_err_t err = nvs_helper_save(NVS_KEY_COUNT, &service_count, sizeof(uint8_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save service count: %s", esp_err_to_name(err));
        return err;
    }

    // Save each service
    for (uint8_t i = 0; i < service_count; i++) {
        char key[16];
        get_service_key(i, key, sizeof(key));
        
        err = nvs_helper_save(key, &services[i], sizeof(totp_service_t));
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to save service %d: %s", i, esp_err_to_name(err));
            return err;
        }
    }

    ESP_LOGI(TAG, "Saved %d services to NVS", service_count);
    return ESP_OK;
}

esp_err_t totp_storage_init(void) {
    if (storage_ready) {
        ESP_LOGW(TAG, "TOTP storage already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing TOTP storage");

    // Initialize NVS helper
    esp_err_t err = nvs_helper_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS helper: %s", esp_err_to_name(err));
        return err;
    }

    // Load services from NVS (this is OK to fail on first run)
    err = load_services_from_nvs();
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "Failed to load services: %s", esp_err_to_name(err));
        return err;
    }

    storage_ready = true;
    ESP_LOGI(TAG, "TOTP storage initialized with %d services", service_count);
    return ESP_OK;
}

esp_err_t totp_storage_deinit(void) {
    if (!storage_ready) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing TOTP storage");
    storage_ready = false;
    service_count = 0;
    
    return ESP_OK;
}

bool totp_storage_is_ready(void) {
    return storage_ready;
}

esp_err_t totp_storage_add(const totp_service_t *service) {
    if (!storage_ready) {
        ESP_LOGE(TAG, "TOTP storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (service == NULL) {
        ESP_LOGE(TAG, "Service is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (service_count >= MAX_SERVICES) {
        ESP_LOGE(TAG, "Storage is full (max %d services)", MAX_SERVICES);
        return ESP_ERR_NO_MEM;
    }

    // Add to in-memory cache
    memcpy(&services[service_count], service, sizeof(totp_service_t));
    service_count++;

    // Save to NVS
    esp_err_t err = save_services_to_nvs();
    if (err != ESP_OK) {
        // Rollback
        service_count--;
        ESP_LOGE(TAG, "Failed to save services after add: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Added service: %s (%s) - Total: %d", 
             service->issuer, service->account, service_count);
    return ESP_OK;
}

esp_err_t totp_storage_get(uint8_t index, totp_service_t *service) {
    if (!storage_ready) {
        ESP_LOGE(TAG, "TOTP storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (service == NULL) {
        ESP_LOGE(TAG, "Service output is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (index >= service_count) {
        ESP_LOGE(TAG, "Index %d out of range (count: %d)", index, service_count);
        return ESP_ERR_INVALID_ARG;
    }

    memcpy(service, &services[index], sizeof(totp_service_t));
    return ESP_OK;
}

uint8_t totp_storage_count(void) {
    return storage_ready ? service_count : 0;
}

esp_err_t totp_storage_delete(uint8_t index) {
    if (!storage_ready) {
        ESP_LOGE(TAG, "TOTP storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (index >= service_count) {
        ESP_LOGE(TAG, "Index %d out of range (count: %d)", index, service_count);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Deleting service %d: %s (%s)", 
             index, services[index].issuer, services[index].account);

    // Shift services down
    for (uint8_t i = index; i < service_count - 1; i++) {
        memcpy(&services[i], &services[i + 1], sizeof(totp_service_t));
    }
    service_count--;

    // Clear the last service key in NVS (now unused)
    char key[16];
    get_service_key(service_count, key, sizeof(key));
    nvs_helper_delete(key);

    // Save updated list to NVS
    esp_err_t err = save_services_to_nvs();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save services after delete: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "Service deleted - Remaining: %d", service_count);
    return ESP_OK;
}

esp_err_t totp_storage_clear(void) {
    if (!storage_ready) {
        ESP_LOGE(TAG, "TOTP storage not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Clearing all services");

    // Delete all service keys
    for (uint8_t i = 0; i < service_count; i++) {
        char key[16];
        get_service_key(i, key, sizeof(key));
        nvs_helper_delete(key);
    }

    service_count = 0;
    
    // Save count = 0
    esp_err_t err = nvs_helper_save(NVS_KEY_COUNT, &service_count, sizeof(uint8_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save cleared count: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "All services cleared");
    return ESP_OK;
}

char* totp_storage_list_json(void) {
    if (!storage_ready) {
        ESP_LOGE(TAG, "TOTP storage not initialized");
        return NULL;
    }

    // Calculate required size
    size_t json_size = 2; // "[]"
    for (uint8_t i = 0; i < service_count; i++) {
        // Rough estimate: each service ~300 bytes
        json_size += 350;
    }

    char *json = malloc(json_size);
    if (json == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for JSON");
        return NULL;
    }

    // Build JSON array
    strcpy(json, "[");
    
    for (uint8_t i = 0; i < service_count; i++) {
        char entry[400];
        snprintf(entry, sizeof(entry),
            "%s{\"service_name\":\"%s\",\"account\":\"%s\",\"issuer\":\"%s\",\"secret\":\"%s\",\"digits\":%d,\"period\":%lu}",
            (i > 0) ? "," : "",
            services[i].service_name,
            services[i].account,
            services[i].issuer,
            services[i].secret,
            services[i].digits,
            services[i].period
        );
        strcat(json, entry);
    }
    
    strcat(json, "]");

    return json;
}
