#include "gui.h"
#include "navigation/gui_screen_manager.h"
#include "esp_log.h"

static const char *TAG = "GUI";

void gui_init(void) {
    ESP_LOGI(TAG, "Initializing GUI system");
    
    // Initialize styles first
    gui_styles_init();
    
    // Initialize navigation system
    gui_navigator_init();
    gui_screen_manager_init();
    
    // Show main menu
    gui_navigate_to(GUI_SCREEN_MAIN, NULL);
    
    // Initialize WiFi icon as disconnected
    gui_update_wifi_status(false);
    
    ESP_LOGI(TAG, "GUI system initialized successfully");
}

void gui_update_wifi_status(bool connected) {
    gui_update_wifi_icon(connected);
}
