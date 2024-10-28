#pragma once

// Common ESP-IDF Helpers
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

// ST7789 LCD
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_types.h"

// Backlight
#include "driver/gpio.h"

// LVGL
#include "esp_lvgl_port.h"

/* LCD color formats */
#define ESP_LCD_COLOR_FORMAT_RGB565 (1)
#define ESP_LCD_COLOR_FORMAT_RGB888 (2)

/* LCD display color format */
#define LCD_COLOR_FORMAT (ESP_LCD_COLOR_FORMAT_RGB565)

/* LCD display color bytes endianess */
#define LCD_BIGENDIAN (1)
/* LCD display color bits */
#define LCD_BITS_PER_PIXEL (16)
/* LCD display color space */
#define LCD_COLOR_SPACE (ESP_LCD_COLOR_SPACE_RGB)
/* LCD definition */
#define LCD_H_RES (320)
#define LCD_V_RES (170)
#define LCD_Y_GAP (35)
#define LCD_X_GAP (0)
#define LCD_SWAP_XY (1)
#define LCD_MIRROR_X (0)
#define LCD_MIRROR_Y (1)
#define LCD_INVERT_COLOR (1)

#define LCD_SPI_CS (GPIO_NUM_41)
#define LCD_DC (GPIO_NUM_16)
#define LCD_RST (GPIO_NUM_40)
#define LCD_BACKLIGHT (GPIO_NUM_21)

#define LCD_PIXEL_CLOCK_HZ SPI_MASTER_FREQ_80M
#define LCD_SPI_NUM (SPI2_HOST)

#define LCD_DRAW_BUFF_SIZE (LCD_H_RES * 20) // *100
#define LCD_DRAW_BUFF_DOUBLE (1)

#define LCD_USE_DMA (1)
#define LCD_USE_SPIRAM (0)

#define LCD_QUEUE_SIZE (100)

// Bit number used to represent command and parameter
#define LCD_CMD_BITS (8)
#define LCD_PARAM_BITS (8)

// My Colors
#define BACKGROUND_COLOR (0x9BCDEF)
#define FOREGROUND_COLOR (0xF79347)

#define BATTERY_BAR_REFRESH_RATE 300

#define SD_LOGO_REFRESH_RATE 3000

#define WIFI_LOGO_REFRESH_RATE 1000

#define CLOCK_REFRESH_RATE 100


typedef enum
{
    WHEEL_UP,
    WHEEL_DOWN,
    WHEEL_CLICK_SHORT,
    WHEEL_CLICK_LONG,
    KEY_CLICK_SHORT,
    KEY_CLICK_LONG
} user_actions_t;

esp_err_t display_init(spi_host_device_t spi_host);

esp_err_t lvgl_init(void);

void display_backlight_on(void);

void display_backlight_off(void);

void display_backlight_toggle(void);

void start_gui(void);

void user_action(user_actions_t action);