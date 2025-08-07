#include "lvgl_helper.h"
#include "gui.h"
#include "lcd_helper.h"

void app_main(void)
{

    lcd_init();

    lvgl_task_init();

    begin_lvgl_procedure();
    create_gui();
    end_lvgl_procedure();
}
