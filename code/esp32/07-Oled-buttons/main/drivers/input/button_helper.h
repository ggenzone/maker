#ifndef BUTTON_HELPER_H
#define BUTTON_HELPER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "driver/gpio.h"

// Definir los pines de los botones (ESP32-WROVER-E safe pins)
#define BUTTON_1_PIN    GPIO_NUM_25
#define BUTTON_2_PIN    GPIO_NUM_26
#define BUTTON_3_PIN    GPIO_NUM_32
#define BUTTON_4_PIN    GPIO_NUM_33


// Callback type for button events
typedef void (*button_callback_t)(uint8_t button_num);

/**
 * @brief Initialize buttons with interrupts and debounce
 * @return ESP_OK on success
 */
esp_err_t button_init(void);

/**
 * @brief Deinitialize buttons
 * @return ESP_OK on success
 */
esp_err_t button_deinit(void);

/**
 * @brief Check if buttons are ready
 * @return true if buttons are initialized
 */
bool button_is_ready(void);

/**
 * @brief Read button state directly (bypasses interrupt system)
 * @param button_num Button number (1-4)
 * @return true if button is pressed
 */
bool button_read(uint8_t button_num);

/**
 * @brief Register callback for button press events
 * @param callback Function to call when a button is pressed
 * @return ESP_OK on success
 */
esp_err_t button_register_callback(button_callback_t callback);

#endif // BUTTON_HELPER_H
