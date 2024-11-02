#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define MAX_SCAN_SIZE 10

esp_err_t wifi_init(void);
esp_err_t wifi_connect(const char *ssid, const char *pass, const char *hostname);

bool wifi_get_state(void);
int wifi_get_rssi(void);
void wifi_launch_scan(void);
const char* wifi_get_auth_string(wifi_auth_mode_t authmode);
const char* wifi_get_cipher_string(wifi_cipher_type_t cipher_type);
