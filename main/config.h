#pragma once

// Common ESP-IDF Helpers
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

#include <cJSON.h>

typedef struct {
  char *ssid;
  char *pass;
  char *hostname;
  char *timezone;
} klaus_config_t;

esp_err_t config_parse_config(klaus_config_t *klaus_config);