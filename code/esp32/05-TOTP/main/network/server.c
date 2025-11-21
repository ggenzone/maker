#include "server.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "totp/totp_storage.h"
#include "totp/totp_parser.h"
#include "totp/totp_engine.h"
#include <string.h>
#include <sys/time.h>
#include <cJSON.h>

static const char *TAG = "server";
static httpd_handle_t server = NULL;
static bool server_running = false;

// Embedded HTML file
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

// HTTP GET handler for root path
static esp_err_t root_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Serving root page");
    
    const size_t index_html_size = (index_html_end - index_html_start);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, index_html_size);
    
    return ESP_OK;
}

// Mock API: Get services list (JSON)
static esp_err_t api_services_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "API: Get services");
    
    // Use real storage
    char *json = totp_storage_list_json();
    if (json == NULL) {
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, "{\"error\":\"Failed to generate JSON\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    
    free(json);
    return ESP_OK;
}

// API: Add service (POST)
static esp_err_t api_services_post_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "API: Add service");
    
    // Allocate buffer dynamically to avoid stack overflow
    char *buf = malloc(512);
    if (buf == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory");
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, "{\"error\":\"Out of memory\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    int ret = httpd_req_recv(req, buf, 512 - 1);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        free(buf);
        return ESP_FAIL;
    }
    buf[ret] = '\0';
    
    ESP_LOGI(TAG, "Received: %s", buf);
    
    // Parse JSON to extract URI
    cJSON *root = cJSON_Parse(buf);
    free(buf); // Free buffer after parsing JSON
    
    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{\"error\":\"Invalid JSON\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    cJSON *uri_json = cJSON_GetObjectItem(root, "uri");
    if (uri_json == NULL || !cJSON_IsString(uri_json)) {
        ESP_LOGE(TAG, "URI field not found or invalid");
        cJSON_Delete(root);
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{\"error\":\"URI field required\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    const char *uri = uri_json->valuestring;
    ESP_LOGI(TAG, "Parsing URI: %s", uri);
    
    // Parse TOTP URI
    totp_service_t service;
    esp_err_t err = totp_parse_uri(uri, &service);
    
    // Clean up JSON before checking error
    cJSON_Delete(root);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to parse URI: %s", esp_err_to_name(err));
        httpd_resp_set_status(req, "400 Bad Request");
        char response[128];
        snprintf(response, sizeof(response), "{\"error\":\"Invalid URI: %s\"}", esp_err_to_name(err));
        httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    // Add service to storage
    err = totp_storage_add(&service);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add service: %s", esp_err_to_name(err));
        httpd_resp_set_status(req, "500 Internal Server Error");
        char response[128];
        snprintf(response, sizeof(response), "{\"error\":\"Failed to save: %s\"}", esp_err_to_name(err));
        httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    // Success response
    httpd_resp_set_status(req, "201 Created");
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"success\":true}", HTTPD_RESP_USE_STRLEN);
    
    ESP_LOGI(TAG, "Service added successfully: %s (%s)", service.issuer, service.account);
    return ESP_OK;
}

// API: Get TOTP code
static esp_err_t api_code_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "API: Get code");

    
    // Extract index from URI (e.g., /api/code/0)
    const char *uri = req->uri;
    const char *index_str = strrchr(uri, '/');
    if (index_str == NULL) {
        ESP_LOGE(TAG, "Invalid URI format");
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{\"error\":\"Invalid URI\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    int index = atoi(index_str + 1);
    ESP_LOGI(TAG, "Getting code for service index %d", index);
    
    // Get service from storage
    totp_service_t service;
    esp_err_t err = totp_storage_get(index, &service);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get service %d: %s", index, esp_err_to_name(err));
        httpd_resp_set_status(req, "404 Not Found");
        httpd_resp_send(req, "{\"error\":\"Service not found\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    // Generate real TOTP code
    uint32_t totp_code;
    ESP_LOGI(TAG, "Generating TOTP code for service: %s (%s)", service.issuer, service.account);
    ESP_LOGI(TAG, "Using secret: %s", service.secret);
    ESP_LOGI(TAG, "Using digits: %d, period: %lu", service.digits, service.period);
    err = totp_get_code(service.secret, service.digits, &totp_code);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to generate TOTP code: %s", esp_err_to_name(err));
        httpd_resp_set_status(req, "500 Internal Server Error");
        httpd_resp_send(req, "{\"error\":\"Failed to generate code\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    // Get remaining seconds
    uint32_t remaining = totp_get_remaining_seconds(service.period);
    
    char response[256];
    snprintf(response, sizeof(response), 
        "{\"code\":%lu,\"remaining\":%lu,\"service\":\"%s\"}", 
        totp_code, remaining, service.issuer);
    
    ESP_LOGI(TAG, "Sending TOTP code for %s: %0*lu (remaining: %lu)", 
             service.issuer, service.digits, totp_code, remaining);
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

// Mock API: Delete service
static esp_err_t api_services_delete_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "API: Delete service");
    
    // Extract index from URI (e.g., /api/services/0)
    const char *uri = req->uri;
    const char *index_str = strrchr(uri, '/');
    if (index_str == NULL) {
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_send(req, "{\"error\":\"Invalid URI\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_FAIL;
    }
    
    int index = atoi(index_str + 1);
    esp_err_t err = totp_storage_delete(index);
    
    if (err == ESP_OK) {
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"success\":true}", HTTPD_RESP_USE_STRLEN);
    } else {
        httpd_resp_set_status(req, "400 Bad Request");
        char response[64];
        snprintf(response, sizeof(response), "{\"error\":\"Delete failed: %s\"}", esp_err_to_name(err));
        httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    }
    
    return err;
}

bool api_code_uri_match(httpd_req_t *req, const char *uri, size_t uri_len) {
    const char *prefix = "/api/code/";

    // Si el path inicia con el prefijo, es válido
    if (strncmp(req->uri, prefix, strlen(prefix)) == 0) {
        return true;
    }

    // También aceptamos exactamente /api/code
    if (strcmp(req->uri, "/api/code") == 0) {
        return true;
    }

    return false;
}

// URI handler structures
static const httpd_uri_t root_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t api_services_get_uri = {
    .uri       = "/api/services",
    .method    = HTTP_GET,
    .handler   = api_services_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t api_services_post_uri = {
    .uri       = "/api/services",
    .method    = HTTP_POST,
    .handler   = api_services_post_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t api_code_uri = {
    .uri       = "/api/code/*",
    .method    = HTTP_GET,
    .handler   = api_code_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t api_services_delete_uri = {
    .uri       = "/api/services/*",
    .method    = HTTP_DELETE,
    .handler   = api_services_delete_handler,
    .user_ctx  = NULL
};

esp_err_t server_init(void) {
    if (server_running) {
        ESP_LOGW(TAG, "Server already running");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Starting HTTP server");

    // HTTP server configuration
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.max_open_sockets = 7;
    config.lru_purge_enable = true;
    config.max_uri_handlers = 10;
    config.stack_size = 6144;  // Increase stack size to avoid overflow
    config.uri_match_fn = httpd_uri_match_wildcard;

    // Start the HTTP server
    esp_err_t err = httpd_start(&server, &config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(err));
        return err;
    }

    // Register URI handlers
    httpd_register_uri_handler(server, &root_uri);
    httpd_register_uri_handler(server, &api_services_get_uri);
    httpd_register_uri_handler(server, &api_services_post_uri);
    httpd_register_uri_handler(server, &api_code_uri);
    httpd_register_uri_handler(server, &api_services_delete_uri);

    server_running = true;
    ESP_LOGI(TAG, "HTTP server started on port 80");
    
    return ESP_OK;
}

esp_err_t server_deinit(void) {
    if (!server_running || server == NULL) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Stopping HTTP server");

    esp_err_t err = httpd_stop(server);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop HTTP server: %s", esp_err_to_name(err));
        return err;
    }

    server = NULL;
    server_running = false;
    ESP_LOGI(TAG, "HTTP server stopped");

    return ESP_OK;
}

bool server_is_running(void) {
    return server_running;
}
