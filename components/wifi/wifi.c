#include "wifi.h"

#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "freertos/event_groups.h"

static const char *TAG = "WiFi";
static bool wifi_init = false;
static uint8_t original_mac_ap[6];

static wifi_ap_records_t ap_records;

static bool is_connected = false;
static bool is_connecting = false;
static bool scanning = false;

static esp_netif_t *netif_sta;
static esp_netif_t *netif_ap;

static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        if (scanning) // dont reparse if already done
        {
            ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_records.count, ap_records.records));
            scanning = false;
        }
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "STA Disconnect");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        is_connected = true;
        is_connecting = false;
    }
}

void wifi_init_apsta()
{
    if (wifi_init)
    {
        return;
    }

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());
    
    wifi_event_group = xEventGroupCreate();
	esp_event_loop_create_default();

    netif_ap = esp_netif_create_default_wifi_ap();
    netif_sta = esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // save original AP MAC address
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_AP, original_mac_ap));
    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_init = true;
}

void wifi_ap_start(wifi_config_t *wifi_config)
{
    ESP_LOGD(TAG, "Starting AP...");
    if (!wifi_init)
    {
        wifi_init_apsta();
    }
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, wifi_config));
    ESP_LOGI(TAG, "AP started with SSID=%s", wifi_config->ap.ssid);
}

void wifi_ap_stop()
{
    ESP_LOGD(TAG, "Stopping AP...");
    wifi_config_t wifi_config = {
        .ap = {
            .max_connection = 0},
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_LOGD(TAG, "AP stopped");
}

int wifi_get_rssi(void)
{
    int rssi = -100;
    if (wifi_is_connected() && !scanning)
    {
        esp_wifi_sta_get_rssi(&rssi);
    }
    return rssi;
}

bool wifi_is_connected(void)
{
    return is_connected;
}

bool wifi_is_connecting(void)
{
    return is_connecting;
}

const char *wifi_get_cipher_string(wifi_cipher_type_t cipher_type)
{
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
    ESP_LOGD(TAG, "Connecting STA to AP...");
    if (!wifi_init)
    {
        wifi_init_apsta();
    }
    esp_netif_set_hostname(netif_sta, hostname);
    wifi_config_t wifi_config;
    memset(&wifi_config,0,sizeof(wifi_config_t));
    strcpy((char *)wifi_config.sta.ssid, (char *)_ssid);
    strcpy((char *)wifi_config.sta.password, (char *)_pass);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());
    is_connecting = true;
    int bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
								   pdFALSE, pdTRUE, 100000 / portTICK_PERIOD_MS);
	ESP_LOGI(TAG, "bits=%x", bits);
	if (bits) {
		ESP_LOGI(TAG, "WIFI_MODE_STA connected.");
	} else {
		ESP_LOGI(TAG, "WIFI_MODE_STA can't connected.");
	}
	return (bits & CONNECTED_BIT) != 0;
}

void wifi_launch_scan(void)
{
    if (!wifi_init)
    {
        wifi_init_apsta();
    }
    ap_records.count = MAX_SCAN_SIZE;
    wifi_scan_config_t scan_config;
    memset(&scan_config, 0, sizeof(wifi_scan_config_t));
    scan_config.show_hidden = true;
    scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    esp_wifi_scan_start(&scan_config, false);
    scanning = true;
}

const wifi_ap_records_t *wifi_get_all_ap_records(void)
{
    if (scanning)
    { // Someone get the event before we parse the ap_records, parse them
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_records.count, ap_records.records));
        scanning = false;
    }
    return &ap_records;
}

const wifi_ap_record_t *wifi_get_one_ap_record(uint8_t index)
{
    if (index > ap_records.count)
    {
        ESP_LOGE(TAG, "Index out of bounds! %u records available, but %u requested", ap_records.count, index);
        return NULL;
    }
    return &ap_records.records[index];
}

void wifi_sta_disconnect(void)
{
    ESP_ERROR_CHECK(esp_wifi_disconnect());
}

void wifi_set_ap_mac(uint8_t *mac_ap)
{
    ESP_LOGD(TAG, "Changing AP MAC address...");
    ESP_ERROR_CHECK(esp_wifi_set_mac(WIFI_IF_AP, mac_ap));
}

void wifi_get_ap_mac(uint8_t *mac_ap)
{
    esp_wifi_get_mac(WIFI_IF_AP, mac_ap);
}

void wifi_restore_ap_mac(void)
{
    ESP_LOGD(TAG, "Restoring original AP MAC address...");
    ESP_ERROR_CHECK(esp_wifi_set_mac(WIFI_IF_AP, original_mac_ap));
}

void wifi_get_sta_mac(uint8_t *mac_sta)
{
    esp_wifi_get_mac(WIFI_IF_STA, mac_sta);
}

void wifi_set_channel(uint8_t channel)
{
    if ((channel == 0) || (channel > 14))
    {
        ESP_LOGE(TAG, "Channel out of range. Expected value from <1,13> but got %u", channel);
        return;
    }
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}