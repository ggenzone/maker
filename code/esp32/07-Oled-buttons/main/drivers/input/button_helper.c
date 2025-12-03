#include "button_helper.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "BTN_HW";
static bool button_initialized = false;
static bool isr_mode = false;
static QueueHandle_t button_event_queue = NULL;

// Array con los pines de los botones
static const gpio_num_t button_pins[] = {
    BUTTON_1_PIN,
    BUTTON_2_PIN,
    BUTTON_3_PIN,
    BUTTON_4_PIN
};

#define NUM_BUTTONS (sizeof(button_pins) / sizeof(button_pins[0]))

// ISR handler - SOLO envía evento crudo a la cola
static void IRAM_ATTR button_isr_handler(void* arg) {
    uint8_t button_idx = (uint8_t)(uintptr_t)arg;
    
    if (button_event_queue != NULL) {
        button_event_t evt = {
            .button_num = button_idx,
            .timestamp = xTaskGetTickCountFromISR()
        };
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(button_event_queue, &evt, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}

esp_err_t button_helper_init(void) {
    if (button_initialized) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing button hardware (polling mode)...");

    // Configurar cada botón SIN interrupciones
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE  // Sin interrupciones
    };

    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        io_conf.pin_bit_mask = (1ULL << button_pins[i]);
        
        esp_err_t ret = gpio_config(&io_conf);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to configure GPIO %d: %s", button_pins[i], esp_err_to_name(ret));
            button_helper_deinit();
            return ret;
        }

        ESP_LOGI(TAG, "Button %d: GPIO %d configured (polling mode)", i + 1, button_pins[i]);
    }

    button_initialized = true;
    isr_mode = false;
    ESP_LOGI(TAG, "Button hardware initialized (%d buttons, polling mode)", NUM_BUTTONS);
    return ESP_OK;
}

esp_err_t button_helper_init_isr(QueueHandle_t event_queue) {
    if (button_initialized) {
        ESP_LOGW(TAG, "Already initialized");
        return ESP_OK;
    }

    if (event_queue == NULL) {
        ESP_LOGE(TAG, "Event queue is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Initializing button hardware (ISR mode)...");
    
    button_event_queue = event_queue;

    // Instalar servicio de ISR de GPIO
    esp_err_t ret = gpio_install_isr_service(0);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install GPIO ISR service: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configurar cada botón CON interrupciones
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
            ESP_LOGE(TAG, "Failed to configure GPIO %d: %s", button_pins[i], esp_err_to_name(ret));
            button_helper_deinit();
            return ret;
        }

        // Instalar ISR handler
        ret = gpio_isr_handler_add(button_pins[i], button_isr_handler, (void*)(uintptr_t)i);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to add ISR for GPIO %d: %s", button_pins[i], esp_err_to_name(ret));
            button_helper_deinit();
            return ret;
        }

        ESP_LOGI(TAG, "Button %d: GPIO %d configured (ISR mode)", i + 1, button_pins[i]);
    }

    button_initialized = true;
    isr_mode = true;
    ESP_LOGI(TAG, "Button hardware initialized (%d buttons, ISR mode)", NUM_BUTTONS);
    return ESP_OK;
}

esp_err_t button_helper_deinit(void) {
    if (!button_initialized) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Deinitializing button hardware...");

    // Remover ISR handlers si estaban instalados
    if (isr_mode) {
        for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
            gpio_isr_handler_remove(button_pins[i]);
        }
    }

    button_initialized = false;
    isr_mode = false;
    button_event_queue = NULL;
    
    ESP_LOGI(TAG, "Button hardware deinitialized");
    return ESP_OK;
}

bool button_helper_is_ready(void) {
    return button_initialized;
}

bool button_helper_read(uint8_t button_num) {
    if (button_num < 1 || button_num > NUM_BUTTONS) {
        return false;
    }
    
    // 0 = presionado (pull-up habilitado)
    return gpio_get_level(button_pins[button_num - 1]) == 0;
}

uint8_t button_helper_get_count(void) {
    return NUM_BUTTONS;
}

bool button_helper_is_isr_mode(void) {
    return isr_mode;
}
