idf_component_register(
    SRCS "clock.c"
    INCLUDE_DIRS .
    PRIV_REQUIRES lwip wifi esp_netif
    )