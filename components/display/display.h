#pragma once

// Common ESP-IDF Helpers
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

// SPI
#include "driver/gpio.h"
#include "driver/spi_master.h"

// ESP LCD
#include "esp_lcd_panel_io.h"

/* LCD definition */
#define LCD_H_RES (320)
#define LCD_V_RES (170)
#define LCD_SWAP_XY (1)
#define LCD_MIRROR_X (0)
#define LCD_MIRROR_Y (1)
#define LCD_DRAW_BUFF_SIZE (LCD_H_RES * 10) // *100
#define LCD_DRAW_BUFF_DOUBLE (1)
#define LCD_USE_DMA (1)
#define LCD_USE_SPIRAM (0)
#define LCD_BIGENDIAN (1)

esp_err_t display_init(spi_host_device_t spi_host, gpio_num_t cs, gpio_num_t dc,
                       gpio_num_t reset, gpio_num_t backlight);

esp_lcd_panel_io_handle_t display_get_io_handle();

esp_lcd_panel_handle_t display_get_panel_handle();

void display_backlight_on(void);
void display_backlight_off(void);
void display_backlight_toggle(void);
void display_backlight_intensity(uint8_t intensity);
void display_blacklight_set_default_intensity(uint8_t intensity);