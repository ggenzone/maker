#include "gui_navigator.h"
#include "gui_screen_manager.h"
#include "esp_log.h"

static const char *TAG = "GUI_NAVIGATOR";

// Navigation stack for back functionality
#define MAX_NAVIGATION_STACK 10
static gui_screen_id_t navigation_stack[MAX_NAVIGATION_STACK];
static int stack_pointer = -1;
static gui_screen_id_t current_screen = GUI_SCREEN_MAIN;

void gui_navigator_init(void) {
    ESP_LOGI(TAG, "Navigation system initialized");
    stack_pointer = -1;
    current_screen = GUI_SCREEN_MAIN;
}

void gui_navigate_to(gui_screen_id_t screen_id, void *user_data) {
    ESP_LOGI(TAG, "Navigating to screen %d", screen_id);
    
    // Push current screen to stack (unless it's the same)
    if (screen_id != current_screen && stack_pointer < MAX_NAVIGATION_STACK - 1) {
        navigation_stack[++stack_pointer] = current_screen;
        ESP_LOGD(TAG, "Pushed screen %d to stack, stack pointer: %d", current_screen, stack_pointer);
    }
    
    current_screen = screen_id;
    gui_screen_manager_show(screen_id, user_data);
}

void gui_navigate_back(void) {
    if (stack_pointer >= 0) {
        gui_screen_id_t previous_screen = navigation_stack[stack_pointer--];
        current_screen = previous_screen;
        gui_screen_manager_show(previous_screen, NULL);
        ESP_LOGI(TAG, "Navigated back to screen %d, stack pointer: %d", previous_screen, stack_pointer);
    } else {
        ESP_LOGW(TAG, "Navigation stack is empty, staying on current screen");
    }
}

gui_screen_id_t gui_get_current_screen(void) {
    return current_screen;
}

void gui_navigate_clear_stack(void) {
    stack_pointer = -1;
    ESP_LOGI(TAG, "Navigation stack cleared");
}
