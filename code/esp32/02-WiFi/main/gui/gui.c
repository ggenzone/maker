#include "gui.h"
#include "esp_log.h"

static const char *TAG = "GUI";

void gui_init(void) {
    ESP_LOGI(TAG, "Initializing GUI system");
    
    // Initialize styles first
    gui_styles_init();
    
    // Show main menu
    gui_show_main_menu();
    
    // Initialize WiFi icon as disconnected
    gui_update_wifi_status(false);
    
    ESP_LOGI(TAG, "GUI system initialized successfully");
}

void gui_update_wifi_status(bool connected) {
    gui_update_wifi_icon(connected);
}
