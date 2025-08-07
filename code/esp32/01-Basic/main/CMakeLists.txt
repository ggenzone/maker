idf_component_register(SRCS "lvgl_helper.c" "lcd_helper.c" "main.c" "gui.c"
                       PRIV_REQUIRES spi_flash REQUIRES esp_timer
                       INCLUDE_DIRS ".")
