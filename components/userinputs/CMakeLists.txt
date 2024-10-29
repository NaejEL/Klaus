idf_component_register(
    SRCS "userinputs.c"
    INCLUDE_DIRS .
    PRIV_REQUIRES esp_driver_gpio esp_driver_pcnt freertos
    )