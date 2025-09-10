#ifndef SD_SPI_H
#define SD_SPI_H

#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include <stdio.h>

/**
 * @brief Initialize SD card
 * @return ESP_OK on success
 */
esp_err_t sd_spi_init(void);

/**
 * @brief Deinitialize SD card
 * @return ESP_OK on success
 */
esp_err_t sd_spi_deinit(void);

/**
 * @brief Check if SD card is initialized
 * @return true if SD card is ready
 */
bool sd_spi_is_ready(void);

// Pin configuration
#define GMAKER_MISO CONFIG_GMAKER_SPI_MISO
#define GMAKER_MOSI CONFIG_GMAKER_SPI_MOSI
#define GMAKER_CLK  CONFIG_GMAKER_SPI_CLK
#define GMAKER_CS   CONFIG_GMAKER_SD_CS

// Definir el host SPI según el modelo de ESP32
#if CONFIG_IDF_TARGET_ESP32S3
    #define GMAKER_SPI_HOST SPI3_HOST
#elif CONFIG_IDF_TARGET_ESP32C6
    #define GMAKER_SPI_HOST SPI2_HOST
#elif CONFIG_IDF_TARGET_ESP32C3
    #define GMAKER_SPI_HOST SPI2_HOST
#elif CONFIG_IDF_TARGET_ESP32
    #define GMAKER_SPI_HOST HSPI_HOST
#else
    #define GMAKER_SPI_HOST SPI2_HOST  // Default fallback
#endif

// Logging to SD card
void gmaker_sd_log_raw(char level, const char *tag, const char *fmt, ...);

#define SD_LOGI(tag, fmt, ...) gmaker_sd_log_raw('I', tag, fmt, ##__VA_ARGS__)
#define SD_LOGW(tag, fmt, ...) gmaker_sd_log_raw('W', tag, fmt, ##__VA_ARGS__)
#define SD_LOGE(tag, fmt, ...) gmaker_sd_log_raw('E', tag, fmt, ##__VA_ARGS__)
#define SD_LOGD(tag, fmt, ...) gmaker_sd_log_raw('D', tag, fmt, ##__VA_ARGS__)

#endif // SD_SPI_H
