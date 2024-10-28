#include "config.h"

#include "sd.h"
static const char *TAG = "Config";
static const char *config_file = "klaus_config.json";

esp_err_t config_parse_config(klaus_config_t *klaus_config)
{
    klaus_config->ssid = TAG;
    klaus_config->pass = TAG;
    klaus_config->hostname = TAG;
    klaus_config->timezone = "UTC+1";

    cJSON *json_config;
    json_config = cJSON_Parse(sd_get_file_content(config_file));
    printf("Parsed Json:%s", cJSON_Print(json_config));

    cJSON *ssid = cJSON_GetObjectItem(json_config, "ssid");
    cJSON *pass = cJSON_GetObjectItem(json_config, "pass");
    cJSON *hostname = cJSON_GetObjectItem(json_config, "hostname");
    cJSON *timezone = cJSON_GetObjectItem(json_config, "timezone");
    
    if (ssid)
    {
        klaus_config->ssid = ssid->valuestring;
    }
    if (pass)
    {
        klaus_config->pass = pass->valuestring;
    }
    if (hostname)
    {
        klaus_config->hostname = hostname->valuestring;
    }
    if (timezone)
    {
        klaus_config->timezone = timezone->valuestring;
    }
    return ESP_OK;
}