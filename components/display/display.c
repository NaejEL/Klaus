#include "display.h"

static const char *TAG = "Display";

// ST7789 LCD
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_types.h"

/* LCD color formats */
#define ESP_LCD_COLOR_FORMAT_RGB565 (1)
#define ESP_LCD_COLOR_FORMAT_RGB888 (2)

/* LCD display color format */
#define LCD_COLOR_FORMAT (ESP_LCD_COLOR_FORMAT_RGB565)

/* LCD display color bits */
#define LCD_BITS_PER_PIXEL (16)
/* LCD display color space */
#define LCD_COLOR_SPACE (ESP_LCD_COLOR_SPACE_RGB)

#define LCD_Y_GAP (35)
#define LCD_X_GAP (0)
#define LCD_INVERT_COLOR (1)

#define LCD_PIXEL_CLOCK_HZ SPI_MASTER_FREQ_80M
#define LCD_SPI_NUM (SPI2_HOST)

#define LCD_QUEUE_SIZE (100)

// Bit number used to represent command and parameter
#define LCD_CMD_BITS (8)
#define LCD_PARAM_BITS (8)

static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;
static gpio_num_t backlight_pin;

static bool backlight_state = false;

static esp_err_t display_backlight_init(void) {
  return gpio_set_direction(backlight_pin, GPIO_MODE_OUTPUT);
}

esp_err_t display_init(spi_host_device_t spi_host, gpio_num_t cs, gpio_num_t dc,
                       gpio_num_t reset, gpio_num_t backlight) {
  backlight_pin = backlight;
  const esp_lcd_panel_io_spi_config_t io_config = {
      .cs_gpio_num = cs,
      .dc_gpio_num = dc,
      .spi_mode = 0,
      .pclk_hz = LCD_PIXEL_CLOCK_HZ,
      .trans_queue_depth = LCD_QUEUE_SIZE,
      .lcd_cmd_bits = LCD_CMD_BITS,
      .lcd_param_bits = LCD_PARAM_BITS,
  };
  ESP_RETURN_ON_ERROR(
      esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)spi_host, &io_config,
                               &io_handle),
      TAG, "LCD new panel IO SPI failed");

  const esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = reset,
      .color_space = LCD_COLOR_SPACE,
      .bits_per_pixel = LCD_BITS_PER_PIXEL,
  };
  ESP_RETURN_ON_ERROR(
      esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle), TAG,
      "Create LCD panel failed");

  ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel_handle), TAG,
                      "LCD Panel Reset failed");
  ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel_handle), TAG,
                      "LCD panel init failed");
  ESP_RETURN_ON_ERROR(esp_lcd_panel_swap_xy(panel_handle, LCD_SWAP_XY), TAG,
                      "LCD panel swap X/Y failed");
  ESP_RETURN_ON_ERROR(
      esp_lcd_panel_mirror(panel_handle, LCD_MIRROR_X, LCD_MIRROR_Y), TAG,
      "LCD panel mirror failed");
  ESP_RETURN_ON_ERROR(
      esp_lcd_panel_invert_color(panel_handle, LCD_INVERT_COLOR), TAG,
      "LCD panel invert color failed");
  ESP_RETURN_ON_ERROR(esp_lcd_panel_set_gap(panel_handle, LCD_X_GAP, LCD_Y_GAP),
                      TAG, "LCD panel set gap failed");
  ESP_RETURN_ON_ERROR(display_backlight_init(), TAG,
                      "LCD backlight init failed");
  ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel_handle, true), TAG,
                      "LCD Panel turn on failed");
  display_backlight_init();
  return ESP_OK;
}

esp_lcd_panel_io_handle_t display_get_io_handle() { return io_handle; }

esp_lcd_panel_handle_t display_get_panel_handle() { return panel_handle; }

void display_backlight_on(void) {
  gpio_set_level(backlight_pin, 1);
  backlight_state = true;
}

void display_backlight_off(void) {
  gpio_set_level(backlight_pin, 0);
  backlight_state = false;
}

void display_backlight_toggle(void) {
  backlight_state ? display_backlight_off() : display_backlight_on();
}