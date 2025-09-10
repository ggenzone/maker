idf_component_register(
    SRCS 
        "main.c"
        "hardware/hardware.c"
        "hardware/sd_spi.c"
    INCLUDE_DIRS 
        "."
        "hardware"
#    PRIV_REQUIRES 
#        spi_flash 
    REQUIRES 
        fatfs
        sdmmc
        driver
        esp_timer
)
