idf_component_register(
    SRCS "battery.c"
    INCLUDE_DIRS .
    REQUIRES driver
    PRIV_REQUIRES bq25896 bq27220
    )