# Proyecto ESP32 - Control de Botones con Interrupciones

Este proyecto implementa el control de un módulo de **4 botones** utilizando **interrupciones GPIO** y **colas (xQueue)** de FreeRTOS para una gestión eficiente y no bloqueante de eventos.

## 📋 Características

- ✅ **Control de 4 botones** mediante interrupciones GPIO
- ✅ **Sistema anti-rebote (debounce)** por software (80ms)
- ✅ **xQueue de FreeRTOS** para comunicación entre ISR y tareas
- ✅ **Callbacks configurables** para responder a eventos de botones
- ✅ **Validación de estado** para evitar falsos positivos
- ✅ **Compatible con ESP32-WROVER-E**

## 🔌 Conexiones de Hardware

### Módulo de 4 Botones (ESP32-WROVER-E)

| Botón | GPIO ESP32-WROVER-E | Configuración |
|-------|---------------------|---------------|
| BTN 1 | GPIO 25             | Pull-up interno activado |
| BTN 2 | GPIO 26             | Pull-up interno activado |
| BTN 3 | GPIO 32             | Pull-up interno activado |
| BTN 4 | GPIO 33             | Pull-up interno activado |

**Nota Importante:** Los pines GPIO16 y GPIO17 **NO están disponibles** en ESP32-WROVER-E porque están conectados a la **PSRAM interna**. Los pines seleccionados (25, 26, 32, 33) son seguros para uso general.

**Conexión:** Los botones deben conectarse entre el pin GPIO y GND. Cuando se presionan, conectan el pin a tierra (lógica LOW = presionado).

### Diagrama de Conexión

```
ESP32-WROVER-E    Módulo de Botones
┌─────┐          ┌──────────────┐
│ 25  │──────────│ BTN1         │
│ 26  │──────────│ BTN2         │
│ 32  │──────────│ BTN3         │
│ 33  │──────────│ BTN4         │
│ GND │──────────│ GND          │
└─────┘          └──────────────┘
```

### ⚠️ Pines a EVITAR en ESP32-WROVER-E

| Pines | Razón |
|-------|-------|
| GPIO 6-11 | Conectados a flash SPI interna |
| GPIO 16-17 | Usados por PSRAM (no disponibles) |
| GPIO 12 | Strapping pin (configuración de voltaje) |

## 🏗️ Arquitectura del Sistema

### Flujo de Eventos

```
Botón Presionado
       ↓
   ISR (GPIO)
       ↓
   xQueueSend (desde ISR)
       ↓
  button_task (recibe evento)
       ↓
  Debounce + Validación
       ↓
  Callback registrado
       ↓
  Acción del usuario (actualizar OLED, etc.)
```

### Componentes Principales

#### 1. ISR (Interrupt Service Routine)
- Se ejecuta en **contexto de interrupción**
- **Mínimo procesamiento**: solo envía el número de botón a la cola
- Usa `xQueueSendFromISR()` para comunicación thread-safe

```c
static void IRAM_ATTR button_isr_handler(void* arg) {
    uint8_t button_num = (uint8_t)(uintptr_t)arg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(button_evt_queue, &button_num, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}
```

#### 2. button_task (Tarea FreeRTOS)
- Recibe eventos desde la cola
- Implementa **debounce temporal** (80ms entre pulsaciones)
- **Valida el estado** del botón antes de confirmar la pulsación
- Ejecuta el callback registrado

#### 3. xQueue (Cola FreeRTOS)
- Capacidad: **10 eventos**
- Almacena números de botón (`uint8_t`)
- Comunicación thread-safe entre ISR y tarea

## 🚀 Uso

### Inicialización

```c
#include "hardware/button_helper.h"

// Callback para eventos de botones
void on_button_pressed(uint8_t button_num) {
    printf("¡Botón %d presionado!\n", button_num);
}

void app_main(void) {
    // Inicializar botones
    esp_err_t ret = button_init();
    if (ret == ESP_OK) {
        // Registrar callback
        button_register_callback(on_button_pressed);
    }
}
```

### API Principal

| Función | Descripción |
|---------|-------------|
| `button_init()` | Inicializa el sistema de botones con interrupciones |
| `button_deinit()` | Limpia recursos (tarea, cola, ISR) |
| `button_is_ready()` | Verifica si el sistema está inicializado |
| `button_read(num)` | Lee el estado directo del botón (bypass de interrupciones) |
| `button_register_callback(cb)` | Registra función callback para eventos |

## ⚙️ Configuración

### Parámetros Ajustables

En `button_helper.c`:

```c
// Tiempo de debounce
const TickType_t debounce_time = pdMS_TO_TICKS(80);  // 80ms

// Capacidad de la cola
button_evt_queue = xQueueCreate(10, sizeof(uint8_t));

// Prioridad de la tarea
xTaskCreate(button_task, "button_task", 3072, NULL, 5, &button_task_handle);
```

### Cambiar Pines

Editar en `button_helper.h`:

```c
#define BUTTON_1_PIN    GPIO_NUM_25  // Seguro para WROVER-E
#define BUTTON_2_PIN    GPIO_NUM_26  // Seguro para WROVER-E
#define BUTTON_3_PIN    GPIO_NUM_32  // Seguro para WROVER-E
#define BUTTON_4_PIN    GPIO_NUM_33  // Seguro para WROVER-E
```

**Otros pines seguros para ESP32-WROVER-E:** GPIO 4, 5, 13, 14, 15, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33.

## 📊 Características Técnicas

### Anti-rebote (Debounce)
- **Temporal**: 80ms entre eventos consecutivos del mismo botón
- **Por validación**: verifica nivel GPIO después de delay de estabilización (20ms)
- **Doble protección**: evita falsos positivos por ruido eléctrico

### Gestión de Memoria
- **Cola**: 10 elementos × 1 byte = 10 bytes
- **Tarea**: 3072 bytes de stack
- **ISR**: código en IRAM para ejecución rápida
- **Compatible con PSRAM**: Los pines no interfieren con la PSRAM del WROVER-E

### Rendimiento
- **Latencia**: ~20-30ms desde pulsación hasta callback
- **Debounce**: 80ms de protección
- **Prioridad**: Tarea con prioridad 5 (ajustable)

## 🔍 Debugging

### Logs Disponibles

El sistema genera logs en diferentes niveles:

```
I (123) BUTTON: Initializing buttons...
I (124) BUTTON: Button 1 configured on GPIO 25
I (125) BUTTON: Button task started
I (200) BUTTON: Button event received: 0
I (220) BUTTON: Button 1 GPIO level: 0
I (221) BUTTON: Button 1 PRESSED (confirmed)
```

### Ajustar Nivel de Log

```c
esp_log_level_set("BUTTON", ESP_LOG_DEBUG);  // Más detalle
esp_log_level_set("BUTTON", ESP_LOG_INFO);   // Normal
esp_log_level_set("BUTTON", ESP_LOG_WARN);   // Solo advertencias
```

## 📂 Estructura del Proyecto

```
06-Buttons/
├── CMakeLists.txt
├── sdkconfig
├── README.md
└── main/
    ├── main.c                    # Aplicación principal
    ├── CMakeLists.txt
    └── hardware/
        ├── button_helper.h       # API pública de botones
        ├── button_helper.c       # Implementación con interrupciones
        ├── hardware.h/c          # Inicialización general
        ├── oled_helper.h/c       # Control del display OLED
        └── ...                   # Otros periféricos
```

## 🛠️ Compilación y Flash

```bash
# Configurar proyecto para ESP32
idf.py set-target esp32

# Compilar
idf.py build

# Flashear
idf.py -p /dev/ttyUSB0 flash

# Monitor serial
idf.py monitor

# Todo en uno
idf.py build flash monitor
```

## 🎯 Ventajas de este Diseño

1. **No bloqueante**: Las interrupciones permiten respuesta inmediata
2. **Eficiente**: El procesador no hace polling constante
3. **Robusto**: Sistema anti-rebote evita eventos espurios
4. **Escalable**: Fácil agregar más botones o cambiar pines
5. **Thread-safe**: Uso correcto de primitivas de FreeRTOS
6. **Modular**: API limpia y reutilizable
7. **Compatible PSRAM**: Los pines seleccionados no interfieren con PSRAM

## 📝 Notas Importantes

- Los botones usan **lógica inversa**: LOW (0) = presionado, HIGH (1) = suelto
- El **pull-up interno** está activado, no se necesitan resistencias externas
- La **ISR** debe ser lo más corta posible (solo envía a cola)
- El **procesamiento pesado** se hace en la tarea, no en la ISR
- El atributo `IRAM_ATTR` asegura que la ISR esté en RAM para ejecución rápida
- **ESP32-WROVER-E específico**: GPIO16/17 reservados para PSRAM (8MB)

## 🔗 Recursos

- [ESP-IDF GPIO API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)
- [FreeRTOS Queues](https://www.freertos.org/a00018.html)
- [ESP32 Interrupt Handling](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/general-notes.html#isr-handlers)
- [ESP32-WROVER-E Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-wrover-e_datasheet_en.pdf)

## 📄 Licencia

Este proyecto es parte de los ejemplos de aprendizaje ESP32.

