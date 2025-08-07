# ESP32 + ILI9341 LCD + XPT2046 Touch - Ejemplo Básico

Este es un ejemplo básico que demuestra cómo utilizar una pantalla LCD ILI9341 de 2.4" con panel táctil XPT2046 en un ESP32 usando ESP-IDF 5.5+ y LVGL.

## 🎯 Descripción

El proyecto implementa una interfaz gráfica básica con navegación por menús usando LVGL (Light and Versatile Graphics Library) sobre una pantalla ILI9341 con capacidades táctiles. La aplicación muestra un menú principal con navegación a submenús usando el panel táctil.

## 📋 Características

- **Pantalla**: ILI9341 240x320 píxeles
- **Touch**: XPT2046 resistivo
- **Interfaz**: LVGL con menús navegables
- **Comunicación**: SPI para pantalla y touch
- **Compatible**: ESP-IDF 5.5+

## 🔧 Hardware Requerido

- ESP32 (cualquier variante)
- Pantalla ILI9341 2.4" con touch XPT2046
- Cables de conexión

## 📐 Conexiones

### Pantalla ILI9341
| Señal LCD | Pin ESP32 | Descripción |
|-----------|-----------|-------------|
| VCC       | 3.3V      | Alimentación |
| GND       | GND       | Tierra |
| CS        | GPIO 5    | Chip Select |
| RESET     | GPIO 22   | Reset |
| DC        | GPIO 4    | Data/Command |
| SDI/MOSI  | GPIO 23   | Master Out Slave In |
| SCK       | GPIO 18   | Clock |
| LED       | GPIO 15   | Backlight |
| SDO/MISO  | GPIO 19   | Master In Slave Out |

### Touch XPT2046
| Señal Touch | Pin ESP32 | Descripción |
|-------------|-----------|-------------|
| T_CS        | GPIO 14   | Touch Chip Select |
| T_CLK       | GPIO 18   | Clock (compartido) |
| T_DIN       | GPIO 23   | Data In (compartido) |
| T_DO        | GPIO 19   | Data Out (compartido) |
| T_IRQ       | NC        | Interrupt (no usado) |

## 🏗️ Estructura del Proyecto

```
01-Basic/
├── main/
│   ├── main.c              # Función principal
│   ├── lcd_helper.c/.h     # Configuración LCD y touch
│   ├── lvgl_helper.c/.h    # Configuración LVGL
│   ├── gui.c/.h            # Interfaz gráfica
│   ├── idf_component.yml   # Dependencias managed components
│   └── CMakeLists.txt      # Configuración componente main
├── CMakeLists.txt          # Configuración proyecto
└── README.md              # Este archivo
```

## 📦 Dependencias (Managed Components)

El proyecto utiliza ESP-IDF Component Manager, por lo que las dependencias se instalan automáticamente:

```yaml
dependencies:
  espressif/esp_lcd_ili9341: ==1.0.0    # Driver LCD ILI9341
  lvgl/lvgl: ^9.2.2                     # LVGL Graphics Library
  atanisoft/esp_lcd_touch_xpt2046: ^1.0.5  # Driver Touch XPT2046
```

## ⚡ Configuración Rápida

### 1. Prerequisitos
- ESP-IDF 5.5 o superior instalado
- Hardware conectado según la tabla de conexiones

### 2. Compilar y Flashear
```bash
# Configurar el target (si es necesario)
idf.py set-target esp32

# Compilar el proyecto
idf.py build

# Flashear al ESP32
idf.py flash

# Monitorear salida serial
idf.py monitor
```

### 3. Uso
- Al iniciar, verás el menú principal en la pantalla
- Toca "Settings" para navegar al submenú
- Toca "Back" para regresar al menú principal

## 🔧 Configuración Personalizada

### Cambiar Pines de Conexión
Edita las definiciones en `main/lcd_helper.c`:

```c
#define PIN_NUM_SCLK           18  // Clock SPI
#define PIN_NUM_MOSI           23  // MOSI SPI
#define PIN_NUM_MISO           19  // MISO SPI
#define PIN_NUM_LCD_DC         4   // Data/Command LCD
#define PIN_NUM_LCD_RST        22  // Reset LCD
#define PIN_NUM_LCD_CS         5   // Chip Select LCD
#define PIN_NUM_BK_LIGHT       15  // Backlight
#define PIN_NUM_TOUCH_CS       14  // Chip Select Touch
```

### Ajustar Resolución o Orientación
En `main/lcd_helper.c`:

```c
#define LCD_H_RES              240  // Resolución horizontal
#define LCD_V_RES              320  // Resolución vertical
```

### Personalizar la Interfaz
Modifica `main/gui.c` para cambiar la apariencia y funcionalidad de los menús.

## 📁 Archivos Principales

### `main.c`
Punto de entrada de la aplicación. Inicializa la pantalla, LVGL y crea la interfaz gráfica.

### `lcd_helper.c`
- Configuración del bus SPI
- Inicialización del driver ILI9341
- Configuración del panel táctil XPT2046
- Callbacks para LVGL

### `lvgl_helper.c`
- Inicialización de LVGL
- Configuración del task de LVGL
- Gestión del timer de LVGL

### `gui.c`
- Implementación de la interfaz gráfica
- Navegación entre menús
- Callbacks de eventos táctiles

## 🐛 Solución de Problemas

### Pantalla en blanco
- Verifica las conexiones, especialmente VCC, GND, y backlight
- Asegúrate de que los pines estén correctamente definidos

### Touch no responde
- Verifica la conexión del pin T_CS (GPIO 14)
- Asegúrate de que los pines SPI compartidos estén bien conectados

### Errores de compilación
- Verifica que tengas ESP-IDF 5.5 o superior
- Ejecuta `idf.py clean` y luego `idf.py build`

### Componentes no encontrados
- Asegúrate de tener conexión a internet para descargar managed components
- Ejecuta `idf.py reconfigure` si hay problemas con dependencias

## 📖 Recursos Adicionales

- [Documentación ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [LVGL Documentation](https://docs.lvgl.io/)
- [ILI9341 Datasheet](https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf)
- [XPT2046 Touch Controller](https://ldm-systems.ru/f/doc/catalog/HY-TFT-2,8/XPT2046.pdf)

## 🔄 Próximos Pasos

Este ejemplo básico puede extenderse con:
- Más pantallas y funcionalidades
- Conectividad WiFi/Bluetooth
- Sensores adicionales
- Almacenamiento de configuración
- Animaciones LVGL más complejas
