#ifndef BUTTON_HELPER_H
#define BUTTON_HELPER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// Definir los pines de los botones (ESP32-WROVER-E safe pins)
#define BUTTON_1_PIN    GPIO_NUM_25
#define BUTTON_2_PIN    GPIO_NUM_26
#define BUTTON_3_PIN    GPIO_NUM_32
#define BUTTON_4_PIN    GPIO_NUM_33

// Evento de botón crudo (sin procesar)
typedef struct {
    uint8_t button_num;     // 0-3 (índice interno)
    TickType_t timestamp;   // Momento del evento
} button_event_t;

/**
 * @brief Inicializa los GPIOs de los botones sin interrupciones (modo polling)
 * 
 * Configura los GPIOs pero no instala ISRs. Útil para lectura directa
 * con button_helper_read() o para implementar polling manual.
 * 
 * @return ESP_OK on success
 */
esp_err_t button_helper_init(void);

/**
 * @brief Inicializa los GPIOs de los botones con interrupciones
 * 
 * Configura los GPIOs e instala ISRs que envían eventos crudos a la cola.
 * La cola debe ser creada y gestionada por la capa de aplicación.
 * 
 * @param event_queue Cola donde se enviarán los eventos crudos (debe ser != NULL)
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG si event_queue es NULL
 */
esp_err_t button_helper_init_isr(QueueHandle_t event_queue);

/**
 * @brief Desinicializa los botones
 * 
 * Remueve ISRs si fueron instaladas y resetea el estado interno.
 * 
 * @return ESP_OK on success
 */
esp_err_t button_helper_deinit(void);

/**
 * @brief Verifica si los botones están inicializados
 * 
 * @return true si están inicializados (con o sin ISR)
 */
bool button_helper_is_ready(void);

/**
 * @brief Lee el estado actual de un botón directamente del GPIO
 * 
 * Esta función no usa interrupciones ni debounce. Lee el estado
 * directo del hardware. Útil para polling o verificación.
 * 
 * @param button_num Número de botón (1-4)
 * @return true si está presionado (nivel bajo con pull-up)
 */
bool button_helper_read(uint8_t button_num);

/**
 * @brief Obtiene el número total de botones configurados
 * 
 * @return Cantidad de botones (4 en esta configuración)
 */
uint8_t button_helper_get_count(void);

/**
 * @brief Verifica si el driver está en modo ISR
 * 
 * @return true si se inicializó con button_helper_init_isr()
 */
bool button_helper_is_isr_mode(void);

#endif // BUTTON_HELPER_H
