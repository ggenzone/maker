#include "sd_spi.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdarg.h>

static const char *TAG = "SD SPI";
static sdmmc_card_t *card  = NULL;
static bool sd_spi_initialized = false;

// Logger interno
static FILE *g_sd_log_file = NULL;
static const char *g_sd_log_path = "/sdcard/log.txt";

static void gmaker_sd_logger_open_if_needed(void) {
    if (!g_sd_log_file) {
        g_sd_log_file = fopen(g_sd_log_path, "a");
        if (g_sd_log_file) {
            unsigned long ms = (unsigned long)(esp_timer_get_time() / 1000ULL);
            fprintf(g_sd_log_file, "[%8lu ms] I/SD_LOG: Logger abierto\n", ms);
            fflush(g_sd_log_file);
        } else {
            ESP_LOGW(TAG, "No se pudo abrir %s", g_sd_log_path);
        }
    }
}

void gmaker_sd_log_raw(char level, const char *tag, const char *fmt, ...) {
    if (!sd_spi_is_ready() || !g_sd_log_file) return;
    unsigned long ms = (unsigned long)(esp_timer_get_time() / 1000ULL);
    fprintf(g_sd_log_file, "[%8lu ms] %c/%s: ", ms, level, tag);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_sd_log_file, fmt, ap);
    va_end(ap);
    fputc('\n', g_sd_log_file);
    fflush(g_sd_log_file); // optimizable
}

esp_err_t sd_spi_init(void) {
    // Configuración del bus SPI
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = GMAKER_MOSI,
        .miso_io_num = GMAKER_MISO,
        .sclk_io_num = GMAKER_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    esp_err_t ret = spi_bus_initialize(GMAKER_SPI_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // Pequeño delay para que el regulador del módulo estabilice
    vTaskDelay(pdMS_TO_TICKS(200));

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = GMAKER_SPI_HOST;
    host.max_freq_khz = 400;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = GMAKER_CS;
    slot_config.host_id = GMAKER_SPI_HOST;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
        spi_bus_free(GMAKER_SPI_HOST);
        return ret;
    }

    sd_spi_initialized = true;
    sdmmc_card_print_info(stdout, card);
    ESP_LOGI(TAG, "SD SPI initialization complete");

    // Abrir logger
    gmaker_sd_logger_open_if_needed();
    if (g_sd_log_file) {
        SD_LOGI("SD_SPI", "Montaje OK, logger activo");
    }

    return ESP_OK;
}

esp_err_t sd_spi_deinit(void) {
    if (!sd_spi_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing SD SPI");

    if (g_sd_log_file) {
        SD_LOGI("SD_SPI", "Cerrando logger");
        fflush(g_sd_log_file);
        fclose(g_sd_log_file);
        g_sd_log_file = NULL;
    }

    if (card) {
        esp_vfs_fat_sdcard_unmount("/sdcard", card);
        spi_bus_free(GMAKER_SPI_HOST);
        card = NULL;
    }
    
    sd_spi_initialized = false;
    ESP_LOGI(TAG, "SD SPI deinitialization complete");

    return ESP_OK;
}

bool sd_spi_is_ready(void) {
    return sd_spi_initialized && (card != NULL);
}





