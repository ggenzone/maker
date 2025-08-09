#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>

typedef struct {
    bool wifi_connected;
    bool wifi_enabled;
} app_state_t;

extern app_state_t app_state;

/**
 * Guarda el estado persistente del sistema en NVS.
 */
void app_state_save(void);

/**
 * Carga el estado persistente del sistema desde NVS.
 */
void app_state_load(void);

#endif // APP_STATE_H