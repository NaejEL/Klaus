/**
 * 
 * All credit to risinek (risinek@gmail.com)
 * https://github.com/risinek/esp32-wifi-penetration-tool
 * 
 */

#pragma once

#include "wifi.h"

void wifi_attack_method_broadcast(const wifi_ap_record_t *ap_record, unsigned period_sec);
void wifi_attack_method_broadcast_stop();
void wifi_attack_method_rogueap(const wifi_ap_record_t *ap_record);

void wifi_attack_dos_start(const wifi_ap_record_t *ap_record);
void wifi_attack_dos_stop();