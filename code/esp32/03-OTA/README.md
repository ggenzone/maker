# ESP32 + ILI9341 LCD + XPT2046 Touch - Ejemplo OTA (Over-The-Air Update)

Este es un ejemplo avanzado que demuestra cómo implementar actualizaciones OTA (Over-The-Air) en un ESP32 con una interfaz gráfica completa usando LCD ILI9341 de 2.4" con panel táctil XPT2046, WiFi, y LVGL.

## 🎯 Descripción

El proyecto implementa un sistema completo de actualización OTA con una interfaz gráfica moderna y navegable. Incluye conectividad WiFi, información detallada del sistema, y capacidades de actualización remota del firmware, todo controlado a través de una pantalla táctil intuitiva.

## 📋 Características Principales

### 🖥️ **Interfaz Gráfica**
- **Pantalla**: ILI9341 240x320 píxeles con retroiluminación controlable
- **Touch**: XPT2046 resistivo con navegación fluida
- **UI Framework**: LVGL 9.2+ con navegación por stack
- **Diseño**: Sistema modular de pantallas y widgets reutilizables

### 🌐 **Conectividad WiFi**
- Configuración WiFi integrada en la interfaz
- Conexión automática con credenciales almacenadas
- Indicador visual de estado de conexión

### 🔄 **Sistema OTA**
- **Actualización Over-The-Air** completa
- Interfaz gráfica para iniciar actualizaciones
- Verificación de conectividad antes de OTA
- Sistema de particiones duales para seguridad

### 📊 **Información del Sistema**
- **Información de firmware**: Versión, fecha de compilación, ESP-IDF
- **Información de hardware**: Chip, memoria flash, RAM libre
- **Estado de red**: IP, MAC, señal WiFi

### ⚙️ **Configuración Avanzada**
- Control de brillo de pantalla con auto-dim
- Configuraciones persistentes en NVS
- Sistema de navegación con historial
- Widgets modulares y reutilizables

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
03-OTA/
├── main/
│   ├── main.c                    # Función principal
│   ├── lvgl_helper.c/.h          # Configuración LVGL
│   ├── Kconfig                   # Configuración personalizable
│   ├── idf_component.yml         # Dependencias managed components
│   ├── CMakeLists.txt            # Configuración componente main
│   ├── gui/                      # Sistema de interfaz gráfica modular
│   │   ├── gui.c/.h             # Coordinador principal GUI
│   │   ├── gui_styles.c/.h      # Estilos globales
│   │   ├── gui_common.c/.h      # Componentes comunes
│   │   ├── navigation/          # Sistema de navegación
│   │   │   ├── gui_navigator.c/.h     # Control de navegación
│   │   │   └── gui_screen_manager.c/.h # Gestión de pantallas
│   │   ├── screens/             # Pantallas individuales
│   │   │   ├── gui_screen_main.c/.h          # Pantalla principal
│   │   │   ├── gui_screen_settings.c/.h     # Menú de configuraciones
│   │   │   ├── gui_screen_general.c/.h      # Configuraciones generales
│   │   │   ├── gui_screen_wifi.c/.h         # Configuraciones WiFi
│   │   │   └── gui_screen_settings_system.c/.h # Información del sistema
│   │   └── widgets/             # Widgets reutilizables
│   │       ├── gui_widget_header.c/.h       # Widget de header
│   │       └── gui_widget_brightness.c/.h   # Control de brillo
│   ├── hardware/                 # Drivers de hardware
│   │   ├── hardware.c/.h        # Inicialización general
│   │   ├── lcd_helper.c/.h      # Driver LCD y touch
│   │   └── touch_helper.c/.h    # Driver panel táctil
│   ├── network/                  # Sistema de red
│   │   ├── network.c/.h         # Gestión WiFi
│   │   ├── network_config.c/.h  # Configuración de red
│   │   └── ota_update.c/.h      # Sistema OTA
│   └── storage/                  # Almacenamiento persistente
│       ├── storage.c/.h         # Inicialización storage
│       └── app_config.c/.h      # Configuración de la aplicación
├── managed_components/           # Componentes ESP-IDF automáticos
├── CMakeLists.txt               # Configuración proyecto
└── README.md                    # Este archivo
```

## 📦 Dependencias (Managed Components)

El proyecto utiliza ESP-IDF Component Manager, por lo que las dependencias se instalan automáticamente:

```yaml
dependencies:
  espressif/esp_lcd_ili9341: ==1.0.0      # Driver LCD ILI9341
  lvgl/lvgl: ^9.2.2                       # LVGL Graphics Library v9.2+
  atanisoft/esp_lcd_touch_xpt2046: ^1.0.5 # Driver Touch XPT2046
  espressif/esp_lcd_touch: ^1.1.2         # Framework Touch genérico
```

### 📋 Componentes ESP-IDF Requeridos
```cmake
REQUIRES:
  esp_timer         # Timers para LVGL y sistema
  esp_wifi          # Conectividad WiFi
  nvs_flash         # Almacenamiento persistente
  esp_http_client   # Cliente HTTP para OTA
  app_update        # Sistema de actualización OTA
```

## ⚡ Configuración Rápida

### 1. Prerequisitos
- ESP-IDF 5.5 o superior instalado
- Hardware conectado según la tabla de conexiones

### 2. Compilar y Flashear
```bash
# Configurar el target (si es necesario)
idf.py set-target esp32

# Configurar pines y parámetros (opcional)
idf.py menuconfig

# Compilar el proyecto
idf.py build

# Flashear al ESP32
idf.py flash

# Monitorear salida serial
idf.py monitor
```

### 3. Configuración WiFi (Requerida para OTA)
Antes de usar OTA, configura las credenciales WiFi en `main/Kconfig`:

```bash
idf.py menuconfig
```

Navega a **"GMaker WiFi Configuration"** y configura:
- `GMAKER_WIFI_SSID` - Nombre de tu red WiFi
- `GMAKER_WIFI_PASSWORD` - Contraseña de tu red WiFi
- `GMAKER_OTA_URL` - URL del servidor OTA (requerida para actualizaciones)

#### 🌐 **Configuración del Servidor OTA**
La variable `GMAKER_OTA_URL` es **obligatoria** para el funcionamiento del sistema OTA. Debe apuntar a un servidor HTTP que contenga el archivo firmware (`.bin`).

**Ejemplo de configuración:**
```
GMAKER_OTA_URL = "http://192.168.1.100:8000/03-OTA.bin"
```

#### 🧪 **Testing Local con Python**
Para pruebas locales, puedes usar Python para servir el firmware desde la carpeta `build/`:

```bash
# Desde la carpeta del proyecto
cd build

# Servir archivos con Python (puerto 8000)
python3 -m http.server 8000

# O especificar un puerto diferente
python3 -m http.server 9000
```

Luego configura `GMAKER_OTA_URL` con tu IP local:
```
GMAKER_OTA_URL = "http://[TU_IP]:8000/03-OTA.bin"
```

**Ejemplo completo de testing:**
1. Compila el proyecto: `idf.py build`
2. Sirve archivos: `cd build && python3 -m http.server 8000`
3. Configura OTA URL: `http://192.168.1.100:8000/03-OTA.bin`
4. Flashea la versión inicial: `idf.py flash`
5. Modifica código, recompila, y testa OTA desde la GUI

### 4. Uso de la Interfaz

#### 🏠 **Menú Principal**
- **Settings**: Accede al menú de configuraciones

#### ⚙️ **Menú Settings**
- **General**: Control de brillo y configuraciones de pantalla
- **WiFi**: Configuración y estado de conectividad WiFi
- **System**: Información completa del sistema y actualizaciones OTA

#### 📊 **Pantalla System Info**
- **Información de Firmware**: Versión, fecha de compilación, ESP-IDF
- **Información de Hardware**: Chip, memoria, estado del sistema
- **Información de Red**: MAC, IP, señal WiFi
- **Estado en Tiempo Real**: RAM libre, uptime, conectividad
- **Update Firmware**: Botón para iniciar actualización OTA (requiere WiFi)

#### 🔄 **Proceso OTA**
1. Conecta a WiFi desde Settings → WiFi
2. Ve a Settings → System 
3. Toca "Update Firmware"
4. El sistema buscará e instalará actualizaciones automáticamente

## 🔧 Configuración Personalizada

### Configuración mediante menuconfig (Recomendado)
El proyecto incluye un sistema de configuración Kconfig que permite personalizar los pines sin modificar el código:

```bash
idf.py menuconfig
```

Navega a: **"GMaker ILI9341 + XPT2046 Configuration"** donde puedes configurar:

#### LCD Pins Configuration:
- `GMAKER_LCD_CS_PIN` - LCD Chip Select (default: 5)
- `GMAKER_LCD_DC_PIN` - LCD Data/Command (default: 4) 
- `GMAKER_LCD_RST_PIN` - LCD Reset (default: 22)
- `GMAKER_LCD_BACKLIGHT_PIN` - LCD Backlight (default: 15)

#### Touch Pins Configuration:
- `GMAKER_TOUCH_CS_PIN` - Touch Chip Select (default: 14)

#### SPI Configuration:
- `GMAKER_SPI_SCLK_PIN` - SPI Clock (default: 18)
- `GMAKER_SPI_MOSI_PIN` - SPI MOSI (default: 23)
- `GMAKER_SPI_MISO_PIN` - SPI MISO (default: 19)

### Cambiar Pines Manualmente (Método Alternativo)
Si prefieres editar directamente el código, modifica las definiciones en `main/lcd_helper.c`:

```c
#define PIN_NUM_SCLK           CONFIG_GMAKER_SPI_SCLK_PIN
#define PIN_NUM_MOSI           CONFIG_GMAKER_SPI_MOSI_PIN
#define PIN_NUM_MISO           CONFIG_GMAKER_SPI_MISO_PIN
#define PIN_NUM_LCD_DC         CONFIG_GMAKER_LCD_DC_PIN
#define PIN_NUM_LCD_RST        CONFIG_GMAKER_LCD_RST_PIN
#define PIN_NUM_LCD_CS         CONFIG_GMAKER_LCD_CS_PIN
#define PIN_NUM_BK_LIGHT       CONFIG_GMAKER_LCD_BACKLIGHT_PIN
#define PIN_NUM_TOUCH_CS       CONFIG_GMAKER_TOUCH_CS_PIN
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

### 🎯 **Core System**
- **`main.c`**: Punto de entrada. Inicializa hardware, WiFi, GUI y sistemas
- **`main/Kconfig`**: Configuración personalizable de pines GPIO y WiFi

### 🖥️ **Hardware Layer**
- **`hardware/lcd_helper.c`**: Driver LCD ILI9341 con configuración SPI
- **`hardware/touch_helper.c`**: Driver panel táctil XPT2046
- **`lvgl_helper.c`**: Configuración e inicialización de LVGL

### 🎨 **GUI System (Modular)**
- **`gui/gui.c`**: Coordinador principal del sistema GUI
- **`gui/gui_styles.c`**: Estilos globales y temas visuales
- **`gui/gui_common.c`**: Componentes UI comunes (headers, botones, items)

#### 🧭 **Navigation System**
- **`gui/navigation/gui_navigator.c`**: Control de navegación con stack
- **`gui/navigation/gui_screen_manager.c`**: Gestión y cambio de pantallas

#### 📱 **Screens (Pantallas)**
- **`gui/screens/gui_screen_main.c`**: Pantalla principal con menú
- **`gui/screens/gui_screen_settings.c`**: Menú de configuraciones
- **`gui/screens/gui_screen_general.c`**: Configuraciones generales (brillo, auto-dim)
- **`gui/screens/gui_screen_wifi.c`**: Configuración y estado WiFi
- **`gui/screens/gui_screen_settings_system.c`**: Información sistema y OTA

#### 🧩 **Widgets (Reutilizables)**
- **`gui/widgets/gui_widget_header.c`**: Widget header con navegación
- **`gui/widgets/gui_widget_brightness.c`**: Control de brillo y auto-dim
- **`gui/widgets/gui_widget_system_info.c`**: Info sistema en tiempo real

### 🌐 **Network Layer**
- **`network/network.c`**: Gestión WiFi y conectividad
- **`network/network_config.c`**: Configuración y credenciales de red
- **`network/ota_update.c`**: Sistema completo de actualización OTA

### 💾 **Storage Layer**
- **`storage/app_config.c`**: Configuración persistente en NVS (brillo, WiFi, etc.)
- **`storage/storage.c`**: Inicialización y gestión del almacenamiento

## 🐛 Solución de Problemas

### 🖥️ **Problemas de Pantalla**
- **Pantalla en blanco**: Verifica VCC, GND, backlight y pines GPIO
- **Touch no responde**: Verifica T_CS (GPIO 14) y pines SPI compartidos
- **Colores incorrectos**: Revisa conexiones DC y configuración ILI9341

### 🌐 **Problemas de WiFi**
- **No conecta**: Verifica SSID y contraseña en menuconfig
- **Conexión intermitente**: Revisa calidad de señal y estabilidad de red
- **No aparece IP**: Verifica que el router asigne DHCP correctamente

### 🔄 **Problemas de OTA**
- **"Enable WiFi to update firmware"**: Activa WiFi desde Settings → WiFi
- **Falla la descarga**: Verifica conexión a internet y servidor OTA
- **Error de verificación**: Asegúrate de que el firmware sea compatible
- **No inicia OTA**: Verifica que haya suficiente espacio en partición OTA

### 🔧 **Problemas de Compilación**
- **Errores de build**: Verifica ESP-IDF 5.5+ y ejecuta `idf.py clean && idf.py build`
- **Componentes no encontrados**: Conecta a internet y ejecuta `idf.py reconfigure`
- **Errores de memoria**: Ajusta configuración de particiones si es necesario

### 💾 **Problemas de Configuración**
- **Configuraciones no se guardan**: Verifica inicialización NVS
- **Brillo no cambia**: Verifica conexión de pin backlight (GPIO 15)
- **Auto-dim no funciona**: Revisa configuración y guardado en NVS

## 🧪 Testing y Desarrollo OTA

### 📝 **Configuración Paso a Paso para Testing**

#### 1. **Preparación del Entorno**
```bash
# Verifica que tengas Python 3 instalado
python3 --version

# Encuentra tu IP local
ip addr show | grep "inet 192"  # Linux
ifconfig | grep "inet 192"      # macOS
ipconfig                         # Windows
```

#### 2. **Configuración del Proyecto**
```bash
# Configura las variables necesarias
idf.py menuconfig
```

En **"GMaker WiFi Configuration"** configura:
- `GMAKER_WIFI_SSID`: Tu red WiFi (ejemplo: "MiWiFi")  
- `GMAKER_WIFI_PASSWORD`: Contraseña de tu WiFi
- `GMAKER_OTA_URL`: URL completa del servidor (ejemplo: "http://192.168.1.100:8000/03-OTA.bin")

#### 3. **Primera Compilación y Flash**  
```bash
# Limpia y compila
idf.py clean && idf.py build

# Flashea la versión inicial
idf.py flash monitor
```

#### 4. **Preparar Servidor OTA Local**
```bash
# En una nueva terminal, ve a la carpeta build
cd build/

# Inicia servidor HTTP con Python
python3 -m http.server 8000

# Verifica que el archivo esté disponible
# Abre http://[TU_IP]:8000 en un navegador
# Deberías ver 03-OTA.bin en la lista
```

#### 5. **Testing del Flujo OTA**
1. **Conecta WiFi**: Desde Settings → WiFi, conecta a tu red
2. **Verifica conexión**: Ve a Settings → System, verifica que muestre tu IP
3. **Haz cambios**: Modifica algo en el código (ej: cambiar texto en pantalla)
4. **Recompila**: `idf.py build` (sin flashear)
5. **Testa OTA**: Desde Settings → System, toca "Update Firmware"

#### 6. **Verificación del Update**
- El ESP32 descargará y aplicará el nuevo firmware automáticamente
- Verifica que los cambios sean visibles después del reinicio
- Revisa la nueva versión en Settings → System

### 🔄 **Workflow de Desarrollo Recomendado**

```bash
# Terminal 1: Servidor HTTP permanente
cd build/ && python3 -m http.server 8000

# Terminal 2: Desarrollo iterativo  
idf.py build                    # Recompila cambios
# Testa OTA desde la GUI        # Sin necesidad de flashear cable
```

### 📋 **Variables de Configuración OTA**

| Variable | Descripción | Ejemplo |
|----------|-------------|---------|
| `GMAKER_OTA_URL` | URL completa del servidor OTA | `http://192.168.1.100:8000/03-OTA.bin` |
| `GMAKER_WIFI_SSID` | Nombre de red WiFi | `MiWiFi_2.4GHz` |
| `GMAKER_WIFI_PASSWORD` | Contraseña WiFi | `mipassword123` |

### ⚠️ **Consideraciones de Desarrollo**

- **Tamaño del firmware**: El archivo `03-OTA.bin` debe ser accesible vía HTTP
- **Red local**: ESP32 y computadora deben estar en la misma red WiFi  
- **Firewall**: Asegúrate de que el puerto 8000 esté abierto
- **Estabilidad**: Para producción, usa un servidor web real (nginx, Apache)

## 📖 Recursos Adicionales

- [Documentación ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [LVGL Documentation](https://docs.lvgl.io/)
- [ILI9341 Datasheet](https://cdn-shop.adafruit.com/datasheets/ILI9341.pdf)
- [XPT2046 Touch Controller](https://ldm-systems.ru/f/doc/catalog/HY-TFT-2,8/XPT2046.pdf)

## � Funcionalidades Avanzadas

### 📋 **Sistema de Navegación**
- **Stack de navegación**: Mantiene historial para botón "Back"
- **Navegación fluida**: Transiciones suaves entre pantallas
- **Gestión de memoria**: Limpieza automática de pantallas

### 🎨 **Sistema de UI Modular**
- **Widgets reutilizables**: Componentes que se pueden usar en múltiples pantallas
- **Estilos consistentes**: Sistema de estilos globales
- **Responsive design**: Adaptación automática al contenido

### 📊 **Monitoreo en Tiempo Real**
- **Timer automático**: Actualización cada 5 segundos
- **Información dinámica**: RAM, IP, uptime
- **Indicadores visuales**: Códigos de color para estados

### 🔐 **Sistema OTA Seguro**
- **Particiones duales**: Rollback automático en caso de falla
- **Verificación**: Checksums y validación de firmware
- **Estados visuales**: Indicación del progreso de actualización

## 🔄 Extensiones Posibles

Este ejemplo OTA completo puede extenderse con:

### 🌟 **Funcionalidades Adicionales**
- **Servidor web integrado**: Para configuración remota
- **Bluetooth**: Conectividad adicional
- **Sensores**: Temperatura, humedad, presión
- **Actuadores**: Control de dispositivos externos
- **Logging**: Sistema de logs con timestamps

### 📱 **Mejoras de UI**
- **Animaciones**: Transiciones y efectos visuales
- **Temas**: Modo oscuro/claro
- **Gráficos**: Charts y visualizaciones de datos
- **Notificaciones**: Alertas y mensajes del sistema

### 🔧 **Mejoras de Sistema**
- **Configuración remota**: Parámetros vía web/app
- **Telemetría**: Envío de datos a servidor
- **Scheduling**: Tareas programadas
- **Backup/Restore**: Respaldo de configuraciones

---

## 📚 Serie de Ejemplos

Este es el **Ejemplo 03 - OTA** de una serie de ejemplos progresivos:

1. **01-Basic**: LCD + Touch básico
2. **02-WiFi**: Conectividad WiFi + GUI
3. **03-OTA**: Sistema OTA completo (este ejemplo)
4. **04-Sensors**: Integración con sensores
5. **05-IoT**: Plataforma IoT completa
