#include "gui_styles.h"
#include "esp_log.h"

static const char *TAG = "GUI_STYLES";

// Global styles
lv_style_t gui_clean_style;

void gui_styles_init(void) {
    ESP_LOGI(TAG, "Initializing GUI styles");
    
    // Initialize clean style for containers
    lv_style_init(&gui_clean_style);
    lv_style_set_radius(&gui_clean_style, 0);           // No rounded corners
    lv_style_set_border_width(&gui_clean_style, 0);     // No border
    lv_style_set_outline_width(&gui_clean_style, 0);    // No outline
    lv_style_set_shadow_width(&gui_clean_style, 0);     // No shadow
    
    ESP_LOGI(TAG, "GUI styles initialized successfully");
}

lv_obj_t* gui_create_clean_container(lv_obj_t *parent) {
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_add_style(obj, &gui_clean_style, 0);
    return obj;
}

void gui_apply_clean_style(lv_obj_t *obj) {
    if (obj == NULL) return;
    
    lv_obj_set_style_radius(obj, 0, 0);           // No rounded corners
    lv_obj_set_style_border_width(obj, 0, 0);     // No border
    lv_obj_set_style_outline_width(obj, 0, 0);    // No outline
    lv_obj_set_style_shadow_width(obj, 0, 0);     // No shadow
}
