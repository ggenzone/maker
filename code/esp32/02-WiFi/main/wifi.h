#ifndef WIFI_H
#define WIFI_H

#include "esp_wifi.h"
#include <stdbool.h>

typedef void (*wifi_status_cb_t)(bool connected);

void wifi_init(void);
void wifi_enable(void);
void wifi_disable(void);
void wifi_register_status_callback(wifi_status_cb_t cb);

#endif
