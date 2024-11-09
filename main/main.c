// Common ESP-IDF Helpers
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "KlausFirmware";

// I2C
#include <driver/i2c.h>
#define SDA_PIN (GPIO_NUM_8)
#define SCL_PIN (GPIO_NUM_18)
#define I2C_PORT_NUM I2C_NUM_0
#define I2C_FREQ 100000
SemaphoreHandle_t i2c_lock;

// SPI
#define SPI_MIS0 (GPIO_NUM_10)
#define SPI_MOSI (GPIO_NUM_9)
#define SPI_CLK (GPIO_NUM_11)
#define SPI_NUM (SPI2_HOST)

#include "userinputs.h"
#include "display.h"
#include "gui.h"
#include "battery.h"
#include "sd.h"
#include "wifi.h"
#include "clock.h"
#include "config.h"
klaus_config_t klaus_config;

#include "pn532.h"

static void i2c_init(void)
{
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = SCL_PIN;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FREQ;
    i2c_param_config(I2C_PORT_NUM, &conf);
    i2c_driver_install(I2C_PORT_NUM, conf.mode, 0, 0, 0);
}

static esp_err_t spi_init(void)
{
    const spi_bus_config_t spi_bus_cfg = {
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MIS0,
        .sclk_io_num = SPI_CLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 0,
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI_NUM, &spi_bus_cfg, SPI_DMA_CH_AUTO), TAG, "Initialize SPI bus failed");
    return ESP_OK;
}

void app_main(void)
{
    // wifi_init_apsta();
    //  Power LEDs and CC1101
    gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_15, 1);

    userinputs_init();

    spi_init();
    sd_init(SPI_NUM);
    config_parse_config(&klaus_config);

    i2c_init();

    pn532_i2c_init(I2C_PORT_NUM, PN532_IRQ, PN532_RESET);
    uint32_t pn532_ver = pn532_get_firmware_version();

    printf("PN5%2x, Ver.%d.%d\n",
           (uint8_t)((pn532_ver >> 24) & 0xFF),
           (uint8_t)((pn532_ver >> 16) & 0xFF),
           (uint8_t)((pn532_ver >> 8) & 0xFF));

    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
    uint8_t uidLength = 0;                 // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    if (pn532_read_passive_targetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 0) == ESP_OK)
    {
        printf("Found an ISO14443A card UID Length:%d, UID:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n", uidLength, uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
        if (uidLength == 4)
        {
            // We probably have a Mifare Classic card ...
            uint32_t cardid = uid[0];
            cardid <<= 8;
            cardid |= uid[1];
            cardid <<= 8;
            cardid |= uid[2];
            cardid <<= 8;
            cardid |= uid[3];
            printf("Seems to be a Mifare Classic card #%ld\n", cardid);
        }
    }

    battery_init(I2C_PORT_NUM);

    display_init(SPI_NUM);
    display_backlight_on();
    gui_init();

    wifi_connect(klaus_config.ssid, klaus_config.pass, klaus_config.hostname);
    clock_set(klaus_config.timezone);
}
