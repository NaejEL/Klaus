idf_component_register(
    SRCS "display.c"
    INCLUDE_DIRS .
    REQUIRES esp_lcd
    PRIV_REQUIRES esp_driver_spi esp_driver_gpio
    )