// Common ESP-IDF Helpers
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "KlausFirmware";

// GUI
#include "display.h"

// Key Button
#include "keybtn.h"

// knob
#include "knob.h"

// I2C
#define SDA_PIN (GPIO_NUM_8)
#define SCL_PIN (GPIO_NUM_18)
#define I2C_PORT_NUM I2C_NUM_0
#define I2C_FREQ 400000

// SPI
#define SPI_MIS0 (GPIO_NUM_10)
#define SPI_MOSI (GPIO_NUM_9)
#define SPI_CLK (GPIO_NUM_11)
#define SPI_NUM (SPI2_HOST)

// Battery
#include "battery.h"

// SD
#include "sd.h"

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
    ESP_LOGD(TAG, "Initialize SPI bus");
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI_NUM, &spi_bus_cfg, SPI_DMA_CH_AUTO), TAG, "Initialize SPI bus failed");
    return ESP_OK;
}

void app_main(void)
{
    // Power LEDs and CC1101
    gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_15, 1);

    spi_init();
    sd_init(SPI_NUM);

    i2c_init();
    battery_init(I2C_PORT_NUM);

    display_init(SPI_NUM);
    display_backlight_on();
    lvgl_init();
    start_gui();
    keybtn_init();
    xTaskCreate(knob_task, "knob_task", KNOB_TASK_STACK_SIZE, NULL, 2, NULL);
}