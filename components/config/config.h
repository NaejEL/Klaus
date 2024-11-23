#pragma once

// Common ESP-IDF Helpers
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

#define CONF_ITEMS                                                             \
  CONF_ITEM(char *, ssid, "None")                                              \
  CONF_ITEM(char *, pass, "")                                                  \
  CONF_ITEM(char *, hostname, "Klaus")                                         \
  CONF_ITEM(char *, timezone, "UTC0")                                          \
  CONF_ITEM(uint8_t, backlight, 50)

typedef struct {
#define CONF_ITEM(type, name, default_value) type name;
  CONF_ITEMS
#undef CONF_ITEM
} klaus_config_t;

esp_err_t config_parse_config(klaus_config_t *klaus_config);