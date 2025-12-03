#include "button_task.h"
#include "drivers/input/button_helper.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "BTN_TASK";

// Estado interno de la tarea
static QueueHandle_t button_evt_queue = NULL;
static TaskHandle_t button_task_handle = NULL;
static button_press_callback_t press_callback = NULL;
static bool task_running = false;

// Configuración de debounce y task
#define DEBOUNCE_TIME_MS        80
#define STABILIZE_TIME_MS       20
#define BUTTON_QUEUE_SIZE       10
#define BUTTON_TASK_STACK_SIZE  3072
#define BUTTON_TASK_PRIORITY    5

// Task de procesamiento con debounce
static void button_processing_task(void* arg) {
    button_event_t evt;
    TickType_t last_press_time[4] = {0};  // Historial de última pulsación
    const TickType_t debounce_ticks = pdMS_TO_TICKS(DEBOUNCE_TIME_MS);

    ESP_LOGI(TAG, "Button processing task started");

    while (1) {
        // Esperar eventos de la cola (bloqueante)
        if (xQueueReceive(button_evt_queue, &evt, portMAX_DELAY)) {
            TickType_t current_time = xTaskGetTickCount();
            
            ESP_LOGD(TAG, "Event received: button_num=%d, timestamp=%lu", 
                     evt.button_num, (unsigned long)evt.timestamp);
            
            // Verificar debounce
            if ((current_time - last_press_time[evt.button_num]) > debounce_ticks) {
                
                // Esperar estabilización del hardware
                vTaskDelay(pdMS_TO_TICKS(STABILIZE_TIME_MS));
                
                // Confirmar que el botón sigue presionado (lectura directa)
                if (button_helper_read(evt.button_num + 1)) {
                    // Actualizar timestamp de última pulsación válida
                    last_press_time[evt.button_num] = current_time;
                    
                    ESP_LOGI(TAG, "Button %d PRESSED (confirmed)", evt.button_num + 1);
                    
                    // Llamar callback de aplicación si está registrado
                    if (press_callback != NULL) {
                        press_callback(evt.button_num + 1);
                    }
                } else {
                    ESP_LOGD(TAG, "Button %d: false trigger (not pressed anymore)", 
                             evt.button_num + 1);
                }
            } else {
                ESP_LOGD(TAG, "Button %d: debounce skip (too soon)", evt.button_num + 1);
            }
        }
    }
}

esp_err_t button_task_init(void) {
    if (task_running) {
        ESP_LOGW(TAG, "Task already running");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing button task...");

    // 1. Crear cola de eventos (gestionada por app layer)
    button_evt_queue = xQueueCreate(BUTTON_QUEUE_SIZE, sizeof(button_event_t));
    if (button_evt_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create event queue");
        return ESP_ERR_NO_MEM;
    }

    // 2. Inicializar hardware con ISR (pasándole nuestra cola)
    esp_err_t ret = button_helper_init_isr(button_evt_queue);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize button hardware: %s", esp_err_to_name(ret));
        vQueueDelete(button_evt_queue);
        button_evt_queue = NULL;
        return ret;
    }

    // 3. Crear tarea de procesamiento
    BaseType_t task_ret = xTaskCreate(
        button_processing_task,
        "button_task",
        BUTTON_TASK_STACK_SIZE,
        NULL,
        BUTTON_TASK_PRIORITY,
        &button_task_handle
    );

    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create processing task");
        button_helper_deinit();
        vQueueDelete(button_evt_queue);
        button_evt_queue = NULL;
        return ESP_ERR_NO_MEM;
    }

    task_running = true;
    ESP_LOGI(TAG, "Button task initialized successfully");
    return ESP_OK;
}

esp_err_t button_task_deinit(void) {
    if (!task_running) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing button task...");

    // 1. Eliminar tarea
    if (button_task_handle != NULL) {
        vTaskDelete(button_task_handle);
        button_task_handle = NULL;
    }

    // 2. Deinicializar hardware
    button_helper_deinit();

    // 3. Eliminar cola
    if (button_evt_queue != NULL) {
        vQueueDelete(button_evt_queue);
        button_evt_queue = NULL;
    }

    task_running = false;
    press_callback = NULL;
    
    ESP_LOGI(TAG, "Button task deinitialized");
    return ESP_OK;
}

esp_err_t button_task_register_callback(button_press_callback_t callback) {
    if (!task_running) {
        ESP_LOGE(TAG, "Task not running");
        return ESP_ERR_INVALID_STATE;
    }
    
    press_callback = callback;
    ESP_LOGI(TAG, "Callback registered");
    return ESP_OK;
}

bool button_task_is_running(void) {
    return task_running;
}
