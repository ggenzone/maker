#include "ntp.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include <sys/time.h>

static const char *TAG = "NTP";

void ntp_sync(void)
{
    ESP_LOGI(TAG, "Inicializando SNTP");

    // Opcional: solo sincronizar una vez
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);

    // Servidor NTP
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");

    esp_sntp_init();

    // Esperar sincronización (máx 10 segundos)
    int retry = 0;
    const int retry_count = 20;

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && retry < retry_count) {
        ESP_LOGI(TAG, "Esperando sincronización SNTP...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        retry++;
    }

    if (sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
        ESP_LOGI(TAG, "Tiempo sincronizado!");
    } else {
        ESP_LOGW(TAG, "No se pudo sincronizar con NTP");
    }

    // Log del tiempo actual
    time_t now = 0;
    struct tm timeinfo = {0};
    time(&now);
    localtime_r(&now, &timeinfo);

    ESP_LOGI(TAG, "Tiempo: %02d:%02d:%02d  %02d/%02d/%04d",
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
             timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
}