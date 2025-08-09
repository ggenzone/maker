#include "wifi.h"
#include "app_state.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "wifi";

static wifi_status_cb_t status_cb = NULL;
static bool is_connected = false;

#define WIFI_SSID CONFIG_GMAKER_WIFI_SSID
#define WIFI_PASSWORD CONFIG_GMAKER_WIFI_PASSWORD

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (app_state.wifi_enabled == false) {
        ESP_LOGI(TAG, "WiFi is disabled, ignoring event");
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "WIFI_EVENT_STA_START: Connecting...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Disconnected. Retrying...");
        wifi_event_sta_disconnected_t* disconn = (wifi_event_sta_disconnected_t*) event_data;
        ESP_LOGW(TAG, "Disconnected. Reason code: %d", disconn->reason);
        app_state.wifi_connected = false;
        is_connected = false;
        if (status_cb) status_cb(false);
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        app_state.wifi_connected = true;
        is_connected = true;
        if (status_cb) status_cb(true);
    }
}

void wifi_register_status_callback(wifi_status_cb_t cb) {
    status_cb = cb;
}

void wifi_init(void) {
    esp_log_level_set("wifi", ESP_LOG_VERBOSE);


    // Init stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Registrar manejador de eventos WiFi + IP
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Configurar WiFi STA
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false,
            },
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi init done");

    if (app_state.wifi_enabled) {
        ESP_LOGI(TAG, "WiFi was enabled → Connecting...");
        esp_wifi_connect();
    }
}

void wifi_enable(void) {
    app_state.wifi_enabled = true;

    if (!is_connected) {
        ESP_LOGI(TAG, "wifi_enable(): Connecting...");
        esp_wifi_connect();
    }

    app_state_save(); 
}

void wifi_disable(void) {
    app_state.wifi_enabled = false;

    if (is_connected) {
        ESP_LOGI(TAG, "wifi_disable(): Disconnecting...");
        esp_wifi_disconnect();
        is_connected = false;
        app_state.wifi_connected = false;
        if (status_cb) status_cb(false);
    }

    app_state_save(); 
}
