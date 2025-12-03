#ifndef BUTTON_TASK_H
#define BUTTON_TASK_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

// Callback cuando se confirma una pulsación (después de debounce)
typedef void (*button_press_callback_t)(uint8_t button_num);

/**
 * @brief Inicializa la tarea de procesamiento de botones
 * 
 * Crea la cola de eventos, configura el hardware con ISR y 
 * crea la task que procesa eventos con debounce.
 * 
 * @return ESP_OK on success
 */
esp_err_t button_task_init(void);

/**
 * @brief Detiene y limpia la tarea de botones
 * 
 * Elimina la task, desinicializa el hardware y libera recursos.
 * 
 * @return ESP_OK on success
 */
esp_err_t button_task_deinit(void);

/**
 * @brief Registra un callback para eventos de botones confirmados
 * 
 * El callback se llama desde la task después de aplicar debounce
 * y confirmar que el botón sigue presionado.
 * 
 * @param callback Función que se llama cuando se confirma una pulsación
 *                 Recibe el número de botón (1-4)
 * @return ESP_OK on success
 */
esp_err_t button_task_register_callback(button_press_callback_t callback);

/**
 * @brief Verifica si la tarea está corriendo
 * 
 * @return true si la task está activa
 */
bool button_task_is_running(void);

#endif // BUTTON_TASK_H
