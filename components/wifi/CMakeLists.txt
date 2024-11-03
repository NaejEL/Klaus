idf_component_register(
    SRCS "wifi.c" "wifi_attacks.c"
    INCLUDE_DIRS .
    REQUIRES esp_wifi
    PRIV_REQUIRES nvs_flash wsl_bypasser esp_timer
    )