#pragma once

#include <driver/gpio.h>
#include <driver/i2c.h>
#include <driver/spi_master.h>

// CC1101 and WS2812 Power switch
#define POWER_SWITCH (GPIO_NUM_15)

// Inputs
#define KEY_BTN (GPIO_NUM_6)
#define KNOB_BTN (GPIO_NUM_0)
#define KNOB_A (GPIO_NUM_4)
#define KNOB_B (GPIO_NUM_5)

// I2C
#define SDA_PIN (GPIO_NUM_8)
#define SCL_PIN (GPIO_NUM_18)
#define I2C_PORT_NUM (I2C_NUM_0)
#define I2C_FREQ (100000)

// PN532
#define PN532_IRQ (GPIO_NUM_17)
#define PN532_RESET (GPIO_NUM_45)

// SPI
#define SPI_MIS0 (GPIO_NUM_10)
#define SPI_MOSI (GPIO_NUM_9)
#define SPI_CLK (GPIO_NUM_11)
#define SPI_NUM (SPI2_HOST)

// ST7789
#define LCD_CS (GPIO_NUM_41)
#define LCD_DC (GPIO_NUM_16)
#define LCD_RST (GPIO_NUM_40)
#define LCD_BACKLIGHT (GPIO_NUM_21)

// TF Card
#define SD_CS (GPIO_NUM_13)

// CC1101
// WS2812
// I2S
// IR
// Microphone