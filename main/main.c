// Common ESP-IDF Helpers
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "battery.h"
#include "clock.h"
#include "config.h"
#include "display.h"
#include "gui.h"
#include "peripherals.h"
#include "pn532.h"
#include "sd.h"
#include "userinputs.h"
#include "wifi.h"

static const char *TAG = "KlausFirmware";

SemaphoreHandle_t i2c_lock = NULL;
klaus_config_t klaus_config;

static esp_err_t i2c_init(void) {
  i2c_config_t conf = {};
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = SDA_PIN;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = SCL_PIN;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_FREQ;
  esp_err_t result = i2c_param_config(I2C_PORT_NUM, &conf);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot configure i2c: %s", __func__,
             esp_err_to_name(result));
    return result;
  }
  result = i2c_driver_install(I2C_PORT_NUM, conf.mode, 0, 0, 0);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot install i2c driver: %s", __func__,
             esp_err_to_name(result));
    return result;
  }
  i2c_lock = xSemaphoreCreateBinary();
  xSemaphoreGive(i2c_lock);
  return ESP_OK;
}

static esp_err_t spi_init(void) {
  const spi_bus_config_t spi_bus_cfg = {
      .mosi_io_num = SPI_MOSI,
      .miso_io_num = SPI_MIS0,
      .sclk_io_num = SPI_CLK,
      .quadwp_io_num = GPIO_NUM_NC,
      .quadhd_io_num = GPIO_NUM_NC,
      .max_transfer_sz = 0,
  };
  ESP_RETURN_ON_ERROR(
      spi_bus_initialize(SPI_NUM, &spi_bus_cfg, SPI_DMA_CH_AUTO), TAG,
      "Initialize SPI bus failed");
  return ESP_OK;
}

void app_main(void) {

  ESP_LOGI(TAG, "Restart reason:%d", esp_reset_reason());
  //  Power LEDs and CC1101
  gpio_set_direction(POWER_SWITCH, GPIO_MODE_OUTPUT);
  gpio_set_level(POWER_SWITCH, 1);

  if (userinputs_init(KEY_BTN, KNOB_BTN, KNOB_A, KNOB_B) != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot initialize user inputs", __func__);
    esp_restart();
  }

  if (i2c_init() != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot initialize I2C", __func__);
    esp_restart();
  }
  pn532_i2c_init(I2C_PORT_NUM, PN532_IRQ, PN532_RESET, i2c_lock);
  battery_init(I2C_PORT_NUM, i2c_lock);

  if (spi_init() != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot initialize SPI", __func__);
    esp_restart();
  }

  if (display_init(SPI_NUM, LCD_CS, LCD_DC, LCD_RST, LCD_BACKLIGHT) != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot initialize display", __func__);
    esp_restart();
  }

  if (gui_init() != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot start GUI", __func__);
    esp_restart();
  }
  display_backlight_on();

  if (sd_init(SPI_NUM, SD_CS) != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot initialise TF Card", __func__);
  }

  if (sd_is_present()) {
    config_parse_config(&klaus_config);
    display_blacklight_set_default_intensity(klaus_config.backlight);
    wifi_connect(klaus_config.ssid, klaus_config.pass, klaus_config.hostname);
    clock_set(klaus_config.timezone);
  }
}
