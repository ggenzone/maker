idf_component_register(
    SRCS 
        "main.c"
        "hardware/hardware.c"
        "hardware/button_helper.c"
#        "hardware/sd_spi.c"
    INCLUDE_DIRS 
        "."
        "hardware"
)
