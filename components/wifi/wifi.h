#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

esp_err_t wifi_init(void);
esp_err_t wifi_connect(const char* ssid, const char* pass, const char* hostname);

bool wifi_get_state(void);
int wifi_get_rssi(void);