#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_system.h"
#include "esp_log.h"
#include <string.h>

#define OTA_URL CONFIG_GMAKER_OTA_URL

static const char *TAG = "ota";


void perform_ota_update(void)
{
    esp_http_client_config_t config = {
        .url = OTA_URL,
        .timeout_ms = 10000,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        return ;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        return;
    }


    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "Failed to find OTA partition");
        return;
    }

    // LOG 1: esp_ota_begin
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    ESP_LOGI(TAG, "esp_ota_begin returned %s", esp_err_to_name(err));
    if (err != ESP_OK) {
        return;
    }

    esp_http_client_fetch_headers(client);

    // Leer y escribir los datos
    int total_bytes_read = 0;
    int data_read;
    uint8_t buffer[1024];

    while ((data_read = esp_http_client_read(client, (char *)buffer, sizeof(buffer))) > 0) {
        total_bytes_read += data_read;

        // LOG 2: esp_ota_write
        err = esp_ota_write(update_handle, buffer, data_read);
        ESP_LOGI(TAG, "esp_ota_write returned %s (%d bytes)", esp_err_to_name(err), data_read);
        if (err != ESP_OK) {
            esp_ota_end(update_handle);
            return ;
        }
    }

    ESP_LOGI(TAG, "Total bytes read: %d", data_read);

    if (data_read < 0) {
        ESP_LOGE(TAG, "Error reading data");
        esp_ota_end(update_handle);
        return;
    }

    // LOG 3: esp_ota_end
    err = esp_ota_end(update_handle);
    ESP_LOGI(TAG, "esp_ota_end returned %s", esp_err_to_name(err));
    if (err != ESP_OK) {
        return;
    }

    // Activar la nueva partición
    err = esp_ota_set_boot_partition(update_partition);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "OTA update successful, restarting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
    }

}