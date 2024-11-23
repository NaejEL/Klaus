#include "config.h"

#include "sd.h"
static const char *TAG = "Config";
static const char *config_file = "klaus_config.json";

esp_err_t config_parse_config(klaus_config_t *klaus_config) {

  cJSON *json_config;
  json_config = cJSON_Parse(sd_get_file_content(config_file));

#define CONF_ITEM(_type, name, default_value)                                  \
  klaus_config->name = default_value;                                          \
  cJSON *name = cJSON_GetObjectItem(json_config, #name);                       \
  if (name) {                                                                  \
    if (name->type == cJSON_String) {                                          \
      klaus_config->name = name->valuestring;                                  \
    } else if (name->type == cJSON_Number) {                                   \
      klaus_config->name = name->valueint;                                     \
    }                                                                          \
  }
  CONF_ITEMS
#undef CONF_ITEM

  return ESP_OK;
}