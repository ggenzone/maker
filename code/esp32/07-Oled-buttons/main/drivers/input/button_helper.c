#include "button_helper.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "BUTTON";
static bool button_initialized = false;
static button_callback_t button_callback = NULL;
static QueueHandle_t button_evt_queue = NULL;
static TaskHandle_t button_task_handle = NULL;

// Array con los pines de los botones
static const gpio_num_t button_pins[] = {
    BUTTON_1_PIN,
    BUTTON_2_PIN,
    BUTTON_3_PIN,
    BUTTON_4_PIN
};

#define NUM_BUTTONS (sizeof(button_pins) / sizeof(button_pins[0]))

// ISR handler
static void IRAM_ATTR button_isr_handler(void* arg) {
    uint8_t button_num = (uint8_t)(uintptr_t)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(button_evt_queue, &button_num, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

// Task para manejar eventos de botones (debounce)
static void button_task(void* arg) {
    uint8_t button_num;
    TickType_t last_press_time[NUM_BUTTONS] = {0};
    const TickType_t debounce_time = pdMS_TO_TICKS(80); // 80ms debounce

    ESP_LOGI(TAG, "Button task started");

    while (1) {
        ESP_LOGI(TAG, "Button task looping ...");
        if (xQueueReceive(button_evt_queue, &button_num, portMAX_DELAY)) {
            TickType_t current_time = xTaskGetTickCount();
            
            ESP_LOGI(TAG, "Button event received: %d", button_num);
            
            // Verificar debounce
            if (button_num < NUM_BUTTONS) {
                if ((current_time - last_press_time[button_num]) > debounce_time) {
                    
                    // Esperar un poco para estabilizar
                    vTaskDelay(pdMS_TO_TICKS(20));
                    


                    // Verificar que realmente está presionado (0 = presionado con pull-up)
                    int level = gpio_get_level(button_pins[button_num]);
                    ESP_LOGI(TAG, "Button %d GPIO level: %d", button_num + 1, level);
                    
                    if (level == 0) {

                        last_press_time[button_num] = current_time;

                        ESP_LOGI(TAG, "Button %d PRESSED (confirmed)", button_num + 1);
                        
                        // Llamar callback si está registrado
                        if (button_callback != NULL) {
                            button_callback(button_num + 1);
                        }
                    } else {
                        ESP_LOGW(TAG, "Button %d: false trigger (level=%d)", button_num + 1, level);
                    }
                } else {
                    ESP_LOGD(TAG, "Button %d: debounce skip", button_num + 1);
                }
            }
        }
    }
}

esp_err_t button_init(void) {
    if (button_initialized) {
        ESP_LOGW(TAG, "Buttons already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing buttons...");

    // Crear cola para eventos
    button_evt_queue = xQueueCreate(10, sizeof(uint8_t));
    if (button_evt_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create button event queue");
        return ESP_ERR_NO_MEM;
    }

    // Instalar servicio de ISR de GPIO
    esp_err_t ret = gpio_install_isr_service(0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        // ESP_ERR_INVALID_STATE significa que ya está instalado, lo cual está bien
        ESP_LOGE(TAG, "Failed to install GPIO ISR service: %s", esp_err_to_name(ret));
        vQueueDelete(button_evt_queue);
        button_evt_queue = NULL;
        return ret;
    }

    // Configurar cada botón
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE  // Interrupción en flanco descendente
    };

    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        io_conf.pin_bit_mask = (1ULL << button_pins[i]);
        
        ret = gpio_config(&io_conf);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to configure button %d: %s", i + 1, esp_err_to_name(ret));
            button_deinit();
            return ret;
        }

        // Instalar ISR handler
        ret = gpio_isr_handler_add(button_pins[i], button_isr_handler, (void*)(uintptr_t)i);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to add ISR handler for button %d: %s", i + 1, esp_err_to_name(ret));
            button_deinit();
            return ret;
        }

        ESP_LOGI(TAG, "Button %d configured on GPIO %d", i + 1, button_pins[i]);
    }

    // Crear tarea para manejar eventos
    BaseType_t task_ret = xTaskCreate(button_task, "button_task", 3072, NULL, 5, &button_task_handle);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create button task");
        button_deinit();
        return ESP_ERR_NO_MEM;
    }

    // Dar tiempo a que la tarea se inicie
    vTaskDelay(pdMS_TO_TICKS(100));

    button_initialized = true;
    ESP_LOGI(TAG, "Buttons initialized successfully (4 buttons on GPIOs: %d, %d, %d, %d)",
             button_pins[0], button_pins[1], button_pins[2], button_pins[3]);
    return ESP_OK;
}

esp_err_t button_deinit(void) {
    if (!button_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing buttons...");

    // Eliminar tarea
    if (button_task_handle != NULL) {
        vTaskDelete(button_task_handle);
        button_task_handle = NULL;
    }

    // Remover ISR handlers
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        gpio_isr_handler_remove(button_pins[i]);
    }

    // Eliminar cola
    if (button_evt_queue != NULL) {
        vQueueDelete(button_evt_queue);
        button_evt_queue = NULL;
    }

    button_initialized = false;
    button_callback = NULL;
    
    ESP_LOGI(TAG, "Buttons deinitialized");
    return ESP_OK;
}

bool button_is_ready(void) {
    return button_initialized;
}

bool button_read(uint8_t button_num) {
    if (button_num < 1 || button_num > NUM_BUTTONS) {
        ESP_LOGW(TAG, "Invalid button number: %d", button_num);
        return false;
    }
    
    // 0 = presionado (pull-up habilitado)
    int level = gpio_get_level(button_pins[button_num - 1]);
    // Solo log si está presionado para no saturar
    if (level == 0) {
        ESP_LOGI(TAG, "Button %d read: GPIO %d = PRESSED (0)", button_num, button_pins[button_num - 1]);
    }
    return level == 0;
}

esp_err_t button_register_callback(button_callback_t callback) {
    if (!button_initialized) {
        ESP_LOGE(TAG, "Buttons not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    button_callback = callback;
    ESP_LOGI(TAG, "Button callback registered");
    return ESP_OK;
}
