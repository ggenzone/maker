#include "lvgl.h"
#include <string.h>
#include "esp_log.h"


static const char *TAG = "GUI";


// Variables globales opcionales para navegación
static lv_obj_t *main_menu;
static lv_obj_t *settings_menu;

static void show_main_menu();
static void show_settings_menu();



// Callback para el botón "Settings"
static void settings_btn_event_handler(lv_event_t *e) {
    show_settings_menu();
}

// Callback para el botón "Back" desde Settings
static void back_btn_event_handler(lv_event_t *e) {
    show_main_menu();
}


// Menú principal
static void show_main_menu() {
    lv_obj_clean(lv_scr_act());  // Limpiar pantalla actual

    main_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_menu, lv_pct(100), lv_pct(100));
    lv_obj_center(main_menu);

    lv_obj_t *label = lv_label_create(main_menu);
    lv_label_set_text(label, "Main Menu");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t *settings_btn = lv_btn_create(main_menu);
    lv_obj_set_width(settings_btn, lv_pct(80));
    lv_obj_align(settings_btn, LV_ALIGN_CENTER, 0, -20);
    lv_obj_add_event_cb(settings_btn, settings_btn_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *settings_label = lv_label_create(settings_btn);
    lv_label_set_text(settings_label, "Settings");
    lv_obj_center(settings_label);
}

// Submenú "Settings"
static void show_settings_menu() {
    lv_obj_clean(lv_scr_act());

    settings_menu = lv_obj_create(lv_scr_act());
    lv_obj_set_size(settings_menu, lv_pct(100), lv_pct(100));
    lv_obj_center(settings_menu);

    lv_obj_t *label = lv_label_create(settings_menu);
    lv_label_set_text(label, "Settings");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);


    // Botón Back
    lv_obj_t *back_btn = lv_btn_create(settings_menu);
    lv_obj_set_width(back_btn, lv_pct(80));
    lv_obj_align(back_btn, LV_ALIGN_CENTER, 0, 80);
    lv_obj_add_event_cb(back_btn, back_btn_event_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);
}

// Función principal que llama al menú inicial
void create_gui() {

    show_main_menu();

}