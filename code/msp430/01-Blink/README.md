# MSP430 Blink Example

Proyecto básico de ejemplo para probar el toolchain de GCC de TI con la plataforma MSP430, utilizando VS Code como alternativa al editor oficial de Texas Instruments.

## Hardware

- **Placa**: MSP430 LaunchPad (MSP430G2553)
- **LED**: LED rojo integrado en P1.0

## Descripción

Este proyecto implementa el clásico ejemplo "Blink" que hace parpadear el LED rojo de la placa LaunchPad. El código está organizado en módulos separados para facilitar la comprensión y el mantenimiento.

### Estructura del Proyecto

```
.
├── main.c          # Programa principal
├── led.c           # Implementación de funciones del LED
├── led.h           # Declaraciones de funciones del LED
├── Makefile        # Script de compilación
└── build/          # Directorio de salida
    ├── bin/        # Binarios (.elf)
    └── obj/        # Archivos objeto (.o)
```

## Requisitos

### Toolchain

- **msp430-gcc**: Compilador GCC para MSP430 de Texas Instruments
  - Versión utilizada: 9.3.1.11
  - Ubicación esperada: `~/msp430/msp430-gcc-9.3.1.11_linux64`
  
### Herramientas de Programación

- **mspdebug**: Herramienta de depuración y programación para MSP430
  ```bash
  sudo apt install mspdebug
  ```

## Instalación del Toolchain

1. Descarga msp430-gcc desde el sitio oficial de TI
2. Extrae el archivo en `~/msp430/`
3. Verifica la instalación:
   ```bash
   ~/msp430/msp430-gcc-9.3.1.11_linux64/bin/msp430-elf-gcc --version
   ```

## Compilación

### Compilar el proyecto

```bash
make
```

El ejecutable se generará en `build/bin/blink.elf`

### Limpiar archivos generados

```bash
make clean
```

## Programación

Para flashear el programa en la placa LaunchPad:

```bash
make flash
```

Este comando utiliza `mspdebug` con el driver `rf2500` (apropiado para MSP430 LaunchPad con programador integrado).

## Funcionamiento

El programa:
1. Detiene el watchdog timer
2. Inicializa el LED (P1.0 como salida)
3. En un bucle infinito:
   - Alterna el estado del LED
   - Espera aproximadamente 10000 ciclos

## Notas

- El proyecto utiliza el MCU **MSP430G2553**
- Compilación optimizada para depuración (`-Og -g`)
- Flags de advertencia habilitados: `-Wall -Werror -Wshadow`


## Referencias 

[MSP430-GCC-OPENSOURCE Web](https://www.ti.com/tool/MSP430-GCC-OPENSOURCE)
[GCC for MSP430™ Microcontrollers | Quick Start Guide](https://www.ti.com/lit/ml/slau591c/slau591c.pdf)
