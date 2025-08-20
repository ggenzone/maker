# Módulo de Storage - ESP32 02-WiFi

El módulo de storage proporciona una interfaz unificada para gestionar datos persistentes usando NVS (Non-Volatile Storage) de ESP-IDF.

## Estructura del Módulo

```
storage/
├── storage.h        # API de bajo nivel para NVS
├── storage.c        # Implementación del sistema NVS
├── app_config.h     # API de configuración de aplicación
└── app_config.c     # Gestión de configuraciones específicas
```

## Características Principales

### Sistema Storage Base (`storage.h/c`)
- **Inicialización automática de NVS**: Maneja la configuración y errores de NVS
- **Tipos de datos soportados**: bool, uint8_t, uint16_t, uint32_t, string, blob
- **Gestión de errores**: Valores por defecto automáticos si las claves no existen
- **Operaciones atómicas**: Commit manual para optimizar escrituras

### Configuración de Aplicación (`app_config.h/c`)
- **Configuraciones categorizadas**: WiFi, Display, Sistema, Touch, Red, GUI
- **Valores por defecto**: Sistema robusto con defaults para todas las configuraciones
- **API conveniente**: Getters/setters individuales para cada configuración
- **Persistencia automática**: Guarda cambios al sistema de storage

## Configuraciones Disponibles

### WiFi
- `wifi_enabled`: Habilitar/deshabilitar WiFi

### Display
- `lcd_brightness`: Brillo LCD (0-100%)
- `screen_timeout_sec`: Timeout de pantalla en segundos
- `auto_dim_enabled`: Auto-dimming antes de apagar
- `dim_brightness`: Nivel de brillo para dimming

### Sistema
- `debug_logging`: Habilitar logging verbose
- `system_volume`: Volumen del sistema (0-100%)

### Touch
- `touch_calibrated`: Estado de calibración del touch
- `touch_cal_x_offset/y_offset`: Offsets de calibración
- `touch_cal_x_scale/y_scale`: Factores de escala

### Red
- `connection_timeout_ms`: Timeout de conexión de red
- `auto_reconnect`: Auto-reconexión en pérdida de red

### GUI
- `theme_mode`: Modo de tema (0=light, 1=dark, 2=auto)
- `animations_enabled`: Habilitar animaciones GUI
- `font_size`: Tamaño de fuente (0=small, 1=normal, 2=large)

## Ejemplos de Uso

### Inicialización
```c
#include "storage/storage.h"
#include "storage/app_config.h"

void app_main(void) {
    // Inicializar sistema de storage
    ESP_ERROR_CHECK(storage_init());
    ESP_ERROR_CHECK(app_config_init());
    ESP_ERROR_CHECK(app_config_load());
    
    // Usar configuraciones...
}
```

### Gestión de Configuración
```c
// Obtener configuraciones
uint8_t brightness = app_config_get_lcd_brightness();
bool wifi_enabled = app_config_get_wifi_enabled();

// Modificar configuraciones
app_config_set_lcd_brightness(80);
app_config_set_wifi_enabled(true);

// Guardar cambios
app_config_save();
```

### Uso Directo del Storage
```c
// Guardar datos personalizados
storage_set_string("device_name", "Mi ESP32");
storage_set_u32("boot_count", 42);

// Leer datos con defaults
char device_name[32];
storage_get_string("device_name", device_name, sizeof(device_name), "ESP32-Default");

uint32_t boot_count;
storage_get_u32("boot_count", &boot_count, 0);
```

## Integración con el Sistema

### En main.c
El sistema se inicializa antes que otros módulos para que puedan usar las configuraciones:

```c
void app_main(void) {
    // 1. Storage primero
    ESP_ERROR_CHECK(storage_init());
    ESP_ERROR_CHECK(app_config_init());
    ESP_ERROR_CHECK(app_config_load());
    
    // 2. Hardware con configuraciones
    ESP_ERROR_CHECK(hardware_init());
    lcd_set_brightness(app_config_get_lcd_brightness());
    
    // 3. Resto del sistema...
}
```

### En GUI
Los controles del GUI se sincronizan automáticamente con el storage:

```c
// Slider de brillo en gui_settings.c
static void brightness_slider_cb(lv_event_t *e) {
    uint8_t brightness = lv_slider_get_value(slider);
    
    // Aplicar inmediatamente
    lcd_set_brightness(brightness);
    
    // Guardar persistentemente
    app_config_set_lcd_brightness(brightness);
    app_config_save();
}
```

## Ventajas del Diseño

1. **Separación de responsabilidades**: Storage de bajo nivel vs configuración de aplicación
2. **Tipo-seguro**: APIs específicas para cada tipo de dato
3. **Defaults robustos**: Siempre hay valores válidos aunque no existan en NVS
4. **Eficiencia**: Commit manual para reducir escrituras a flash
5. **Extensibilidad**: Fácil agregar nuevas configuraciones
6. **Mantenibilidad**: Código organizado y bien documentado

## Migración desde app_state.c

El sistema anterior de `app_state.c` ha sido refactorizado para usar el nuevo sistema:

```c
// Antes (app_state.c viejo)
void app_state_save(void) {
    nvs_handle_t handle;
    nvs_open("app_config", NVS_READWRITE, &handle);
    nvs_set_u8(handle, "wifi_enabled", app_state.wifi_enabled ? 1 : 0);
    nvs_commit(handle);
    nvs_close(handle);
}

// Ahora (app_state.c nuevo)
void app_state_save(void) {
    app_config_set_wifi_enabled(app_state.wifi_enabled);
    app_config_save();
}
```

Este diseño mantiene la compatibilidad hacia atrás mientras proporciona una base sólida para futuras configuraciones.
