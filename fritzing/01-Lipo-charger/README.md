# Alimentar un ESP32 con batería LiPo usando TP4056 y regulador buck

## 🔋 ¿Qué batería estamos usando?

- **Tipo:** LiPo plana
- **Modelo:** 103450
- **Voltaje nominal:** 3.7V
- **Capacidad:** 2000mAh
- **Rango de voltaje útil:** ~3.0V a 4.2V

---

## 🧰 Componentes necesarios

| Componente | Descripción |
|-----------|-------------|
| TP4056 con protección | Módulo con USB-C para cargar celdas Li-Ion o LiPo de una celda (1S) |
| Batería LiPo 3.7V 103450 | Fuente de energía recargable |
| Regulador MINI560 | Convierte el voltaje de la batería a 3.3V (salida fija) |
| LED + resistencia 220Ω | Para pruebas básicas |
| S1 Battery Indicator (opcional) | Muestra visualmente el estado de carga |
| Cable USB-C | Para alimentar el TP4056 |
| Protoboard o cables dupont | Para montar las conexiones |

---

## ⚙️ Esquema de conexión

El flujo general de energía es:

[USB-C 5V] → [TP4056] → [Batería LiPo 3.7V] → [MINI560 a 3.3V] → [LED con resistencia] → GND


> ⚠️ Asegurate de respetar las polaridades de conexión. No conectes la batería al revés o podrías dañar el módulo.

Voy a agregar un diagrama visual más adelante para que se entienda mejor.

---

## 🔌 Detalles de conexión

### TP4056
- `IN+ / IN-`: entrada de 5V desde USB
- `B+ / B-`: salida hacia la batería
- `OUT+ / OUT-`: salida protegida hacia el buck

### MINI560
- Entrada: `OUT+ / OUT-` del TP4056
- Salida: 3.3V estables

### LED
- Ánodo (positivo) → salida 3.3V del MINI560
- Cátodo → resistencia de 220Ω → GND

### Battery Indicator S1 (opcional)
- `+` → directamente al terminal `B+` de la batería
- `–` → al `B-`
- Algunos modelos tienen un pin "K" para activarlo (podés conectarlo temporalmente a GND)

---

## ✅ Verificación paso a paso

1. Conectá el TP4056 al USB para cargar la batería.
2. Medí el voltaje de salida del MINI560 — debe ser 3.3V.
3. Conectá el LED con resistencia a la salida del MINI560.
4. (Opcional) Conectá el indicador S1 para ver el estado de carga.





## ⚙️ Otros esquemas de conexión


```ascii
        [USB-C 5V]
             │
       ┌─────▼──────┐
       │  TP4056    │
       │ (protección)│
       └─────┬──────┘
             │ B+ / B-
       ┌─────▼───────┐
       │  Batería    │  → LiPo 3.7V
       └─────┬───────┘
             │
             ▼
       [Buck Converter]
       (MP1584 / MINI560)
             │ (3.3V)
             ▼
          [ESP32]
```



```ascii
                    +-------------+
USB-C 5V ---------> |   TP4056    | <------> Batería LiPo 3.7V
                    |  (con protección)
                    +------+------+      
                           |                            
                           | (OUT+ / OUT-)                         
                    +------v-------+
                    |   MINI560    |  → salida 3.3V
                    +------+-------+
                           |
                  +--------+---------+
                  |                  |
            +-----v----+       +-----v------+
            |   LED    |       | S1 Battery |
            | + Resistor|       | Indicator  |
            +----------+       +------------+
```


