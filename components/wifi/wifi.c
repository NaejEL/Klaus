#include "wifi.h"

#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "freertos/event_groups.h"

static const char *TAG = "WiFi";

static int s_retry_num = 0;
static bool is_connected = false;
static bool scanning = false;

static EventGroupHandle_t s_wifi_event_group;
static esp_netif_t *netif;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        scanning = false;
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        is_connected = false;
        if (s_retry_num < 10)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        is_connected = true;
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

int wifi_get_rssi(void)
{
    int rssi = -100;
    if (wifi_get_state() && !scanning)
    {
        esp_wifi_sta_get_rssi(&rssi);
    }
    return rssi;
}

esp_err_t wifi_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_RETURN_ON_ERROR(nvs_flash_init(), TAG, "Cannot init NVS");
    }

    s_wifi_event_group = xEventGroupCreate();
    ESP_RETURN_ON_ERROR(esp_netif_init(), TAG, "Cannot init netif");

    esp_event_loop_create_default();
    netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    return ESP_OK;
}

bool wifi_get_state(void)
{
    return is_connected;
}

const char* wifi_get_cipher_string(wifi_cipher_type_t cipher_type){
    switch (cipher_type)
    {
    case WIFI_CIPHER_TYPE_NONE:
        return "NONE";
        break;

    case WIFI_CIPHER_TYPE_WEP40:
        return "WEP 40";
        break;

    case WIFI_CIPHER_TYPE_WEP104:
        return "WEP 104";
        break;

    case WIFI_CIPHER_TYPE_TKIP:
        return "TKIP";
        break;

    case WIFI_CIPHER_TYPE_CCMP:
        return "CCMP";
        break;

    case WIFI_CIPHER_TYPE_TKIP_CCMP:
        return "TKIP CCMP";
        break;

    case WIFI_CIPHER_TYPE_AES_CMAC128:
        return "AES-CMAC-128";
        break;

    case WIFI_CIPHER_TYPE_SMS4:
        return "SMS4";
        break;

    case WIFI_CIPHER_TYPE_GCMP:
        return "GCMP";
        break;

    case WIFI_CIPHER_TYPE_GCMP256:
        return "GCMP-256";
        break;

    case WIFI_CIPHER_TYPE_AES_GMAC128:
        return "AES-GMAC-128";
        break;

    case WIFI_CIPHER_TYPE_AES_GMAC256:
        return "AES-GMAC-256";
        break;
    
    default:
        return "UNKNOW";
        break;
    }
}

const char *wifi_get_auth_string(wifi_auth_mode_t authmode)
{
    switch (authmode)
    {
    case WIFI_AUTH_OPEN:
        return "OPEN";
        break;

    case WIFI_AUTH_WEP:
        return "WEP";
        break;

    case WIFI_AUTH_WPA_PSK:
        return "WPA PSK";
        break;

    case WIFI_AUTH_WPA2_PSK:
        return "WPA2 PSK";
        break;

    case WIFI_AUTH_WPA_WPA2_PSK:
        return "WPA WPA2 PSK";
        break;
    case WIFI_AUTH_ENTERPRISE:
        return "EAP";
        break;

    case WIFI_AUTH_WPA3_PSK:
        return "WPA3 PSK";
        break;

    case WIFI_AUTH_WPA2_WPA3_PSK:
        return "WPA2 WPA3 PSK";
        break;

    case WIFI_AUTH_WAPI_PSK:
        return "WAPI PSK";
        break;

    case WIFI_AUTH_OWE:
        return "OWE";
        break;

    case WIFI_AUTH_WPA3_ENT_192:
        return "WPA3 ENT 192";
        break;

     case WIFI_AUTH_WPA3_EXT_PSK:
        return "WPA3 EXT PSK";
        break;   

    case WIFI_AUTH_WPA3_EXT_PSK_MIXED_MODE:
        return "WPA3 EXT PSK MIXED MODE";
        break;

    case WIFI_AUTH_DPP:
        return "DPP";
        break;

    default:
        return "UNKNOW";
        break;
    }
}

esp_err_t wifi_connect(const char *_ssid, const char *_pass, const char *hostname)
{
    esp_netif_set_hostname(netif, hostname);
    wifi_config_t wifi_config;
    strcpy((char *)wifi_config.sta.ssid, (char *)_ssid);
    strcpy((char *)wifi_config.sta.password, (char *)_pass);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    return ESP_OK;
}

void wifi_launch_scan(void)
{
    wifi_scan_config_t scan_config;
    memset(&scan_config, 0, sizeof(wifi_scan_config_t));
    scan_config.show_hidden = true;
    scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    esp_wifi_scan_start(&scan_config, false);
    scanning = true;
}