# ESP32 SD Card DataLogger

Un proyecto para aprender a usar una tarjeta SD con ESP32 y crear un sistema de registro de datos (DataLogger).

## Descripción

Este proyecto enseña cómo usar una tarjeta SD con ESP32 (ESP-IDF) y registrar datos en archivos. Incluye un logger simple que escribe líneas con timestamp en un fichero dentro de la SD.

## Estructura del Proyecto

```
04-SD/
├── CMakeLists.txt
├── README.md
└── main/
    ├── CMakeLists.txt
    ├── Kconfig
    ├── main.c
    └── hardware/
        ├── hardware.c
        ├── sd_spi.c        # Montaje SD + logger integrado
        └── sd_spi.h
```

## Requisitos

Hardware:
- ESP32 (ESP32-S3, ESP32-C6, o ESP32-C3)
- Módulo lector microSD (interfaz SPI)
- Tarjeta microSD (FAT32)
- Cables/jumpers

Software:
- ESP-IDF v5.5 o superior
- Toolchain configurada (idf.py)

## Componentes ESP-IDF Utilizados

- `fatfs`: Sistema de archivos FAT (usado por `esp_vfs_fat_sdspi_mount`)
- `sdmmc`: Funciones SD/MMC y capa SDSPI
- `driver`: Drivers SPI y GPIO (`spi_bus_initialize`, etc.)
- `esp_timer`: Timestamps en micro/milisegundos para el logger

## Configuración de Pines (menuconfig)

Archivo Kconfig permite definir:
- SPI MISO (por defecto GPIO12 en S3)
- SPI MOSI (GPIO11)
- SPI CLK  (GPIO13)
- SD CS    (GPIO10)

Cambiar con:  
`idf.py menuconfig` → GMaker Configuration

## Logger en SD

El driver `sd_spi.c` integra un logger sencillo que escribe en `/sdcard/log.txt`.

Macros disponibles (ya montada la SD):
```
SD_LOGI("APP", "Inicio ok");
SD_LOGW("SENSOR", "Valor fuera de rango=%d", v);
SD_LOGE("SD", "Error=%d", err);
SD_LOGD("DBG", "x=%u y=%u", x, y);
```

Formato de línea:
`[   12345 ms] I/APP: Mensaje`

Características actuales:
- Apertura automática tras montaje SD
- Flush inmediato (simple, sin buffering)
- Cierre en `sd_spi_deinit()`

## Conexiones de Hardware

### ESP32-S3-WROOM-1
| ESP32-S3 | SD Module |
|----------|-----------|
| GPIO12   | MISO      |
| GPIO11   | MOSI      |
| GPIO13   | CLK       |
| GPIO10   | CS        |
| 3.3V     | VCC (o 5V del módulo si tiene regulador y level shifting) |
| GND      | GND       |


## Compilación

```
idf.py set-target esp32s3   (o esp32c6 / esp32c3)
idf.py menuconfig
idf.py build
idf.py flash
idf.py monitor
```

## Funcionalidades Planeadas

- [x] Montaje SD vía SPI
- [x] Escritura básica de archivo
- [x] Logger simple (SD_LOGx)
- [ ] Registro con timestamps RTC
- [ ] Manejo de reconexión SD
- [ ] Buffer y flush configurable
- [ ] Mutex / thread safety
- [ ] Integración con ESP_LOG



## Notas de Desarrollo

- Frecuencia inicial reducida puede mejorar compatibilidad con módulos con level shifters
- Añadir delay tras power-up de módulo SD evita fallos de CMD8
- No alimentar módulo con 5V si NO tiene regulador / level shifting
- Formatear SD en FAT32 (cluster 32 KB recomendado)

## Recursos

- ESP-IDF SD Card Guide  
  https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/sdmmc.html
- Datasheets oficiales (S3, C6, C3)

