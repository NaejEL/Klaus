#include "clock.h"
#include "esp_netif_sntp.h"
#include "lwip/ip_addr.h"
#include "esp_sntp.h"
#include "wifi.h"

static const char *TAG = "SNTP";

static time_t now;
static struct tm timeinfo;

static char formated_time[64];

esp_err_t clock_set(const char *timezone)
{
    time(&now);
    setenv("TZ", timezone, 1);
    tzset();
    localtime_r(&now, &timeinfo);

    if (!wifi_get_state())
    {
        return ESP_FAIL;
    }

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    config.start = false;                     // start SNTP service explicitly (after connecting)
    config.server_from_dhcp = true;           // accept NTP offers from DHCP server, if any (need to enable *before* connecting)
    config.renew_servers_after_new_IP = true; // let esp-netif update configured SNTP server(s) after receiving DHCP lease
    config.index_of_first_server = 1;         // updates from server num 1, leaving server 0 (from DHCP) intact
    // config.ip_event_to_renew = IP_EVENT_STA_GOT_IP;
    esp_netif_sntp_init(&config);
    esp_netif_sntp_start();
    int retry = 0;
    const int retry_count = 15;
    while (esp_netif_sntp_sync_wait(2000 / portTICK_PERIOD_MS) == ESP_ERR_TIMEOUT && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
    }

    strftime(formated_time, sizeof(formated_time), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time is: %s", formated_time);
    return ESP_OK;
}

const char *clock_get_time(void)
{
    time(&now);
    localtime_r(&now, &timeinfo);
    sprintf(formated_time, "%02d/%02d %02d:%02d:%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return formated_time;
}