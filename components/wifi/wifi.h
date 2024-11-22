#pragma once

/*
 Largely inspired by https://github.com/risinek/esp32-wifi-penetration-tool
*/

#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include <stdbool.h>

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define MAX_SCAN_SIZE 10
#define MAX_PHY_SIZE 15

typedef struct {
  uint16_t count;
  wifi_ap_record_t records[MAX_SCAN_SIZE];
} wifi_ap_records_t;

void wifi_init_apsta(void);

void wifi_ap_start(wifi_config_t *wifi_config);
void wifi_ap_stop(void);
void wifi_sta_disconnect(void);
void wifi_set_ap_mac(const uint8_t *mac_ap);
void wifi_get_ap_mac(uint8_t *mac_ap);
void wifi_restore_ap_mac(void);
void wifi_get_sta_mac(uint8_t *mac_sta);
void wifi_set_channel(uint8_t channel);

void wifi_launch_scan(void);
const wifi_ap_records_t *wifi_get_all_ap_records(void);
const wifi_ap_record_t *wifi_get_one_ap_record(uint8_t record_index);
const char *wifi_get_auth_string(wifi_auth_mode_t authmode);
const char *wifi_get_cipher_string(wifi_cipher_type_t cipher_type);
void wifi_get_bssid_string_from_record(uint8_t record_index, char *buffer);
void wifi_get_phy_from_record(uint8_t record_index, char *buffer);

esp_err_t wifi_connect(const char *ssid, const char *pass,
                       const char *hostname);
bool wifi_is_connected(void);
bool wifi_is_connecting(void);
int wifi_get_rssi(void);
