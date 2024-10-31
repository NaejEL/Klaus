# 🐡 Klaus

A cool firmware for the Lilygo T-Embed CC1101 based on ESP-IDF

[Lilygo T-Embed Shop](https://www.lilygo.cc/products/t-embed-cc1101)

[Lilygo T-Embed Official Github](https://github.com/Xinyuan-LilyGO/T-Embed-CC1101)

## 👷 HardwareTodo

- [x] Key buttons [Component](https://components.espressif.com/components/espressif/button/versions/3.3.2)
- [x] Rotary encoder and key [PCNT](https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32/api-reference/peripherals/pcnt.html)
- [x] Display [esp_lcd](https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32/api-reference/peripherals/lcd/index.html)
- [ ] WS2812 [Component](https://components.espressif.com/components/espressif/led_strip/versions/2.5.5)
- [x] SD SPI [sd_spi](https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32/api-reference/peripherals/sdspi_host.html)
- [x] Battery PPM
- [x] Wifi
- [ ] PN532
- [ ] IR [RMT](https://docs.espressif.com/projects/esp-idf/en/v5.3.1/esp32/api-reference/peripherals/rmt.html)
- [ ] CC1101 [Component](https://components.espressif.com/components/jgromes/radiolib/versions/7.0.2)
- [ ] Speaker & Microphone [Component](https://components.espressif.com/components/espressif/esp_codec_dev/versions/1.3.1)
- [ ] Portable Firmware [esp-bsp](https://github.com/espressif/esp-bsp/)

## 👷 Software ToDo

- [x] LVGL [Component](https://components.espressif.com/components/espressif/esp_lvgl_port/versions/2.4.1)

- [x] Better GUI LVGL screen based

- [x] Config from SD

- [ ] Audio Recoder

- [ ] Audio player

- [x] NTP Clock

- [ ] Wifi Pentest

- [ ] IR Pentest

- [ ] BLE Pentest

- [ ] RFID Pentest

## Components

### MCU

ESP32s3 16MB Flash, 8MB PSRAM

Battery voltage detection: GPIO4
Key Button: GPIO6

### Power Management Unit

BQ27220 -> Battery fuel gauge

Address: 0x55

BQ25896 -> Battery charge management

Address: 0x6B

### Screen

https://components.espressif.com/components/espressif/esp_lvgl_port/versions/2.3.3

ST7789V TFT LCD 1.9 inch 170*320

Pins:

- Backlight: GPIO21
- DC: GPIO16
- CS: GPIO41
- CLK: GPIO11
- MOSI: GPIO9
- RESET: GPIO40

### LoRa

https://components.espressif.com/components/jgromes/radiolib/versions/7.0.2

CC1101

Pins:

- CS: GPIO12
- SCK: GPIO11
- MOSI: GPIO9
- MISO: GPIO10
- IO2: GPIO38
- IO0: GPIO3
- SW1: GPIO47
- SW0 GPIO48

| SW1 | SW2 | Freq |
|-----|-----|------|
|  L  |  L  | 315  |
|  L  |  H  | 868  |
|  H  |  H  | 433  |

### RGB Leds

8 * WS2812/APA102

Pins:

- DataIn: GPIO14

### Speaker

I2S

Pins:

- BCLK: GPIO46
- WCLK: GPIO40
- DOUT: GPIO7

### Microphone

Pins:

- DATA: GPIO42
- CLK: GPIO39

### Encoder

Pins:

- A: GPIO4
- B: GPIO5

### RFID

PN532

Address: 0x24

Pins:

- SCL: GPIO18
- SDA: GPIO8
- IRQ: GPIO17
- RF_RES: GPIO45

### TF Card

Pins:

- CS: GPIO13
- SCLK: GPIO11
- MOSI: GPIO9
- MISO: GPIO10

### Infrared

Pins:

- ENABLE: GPIO2
- RX: GPIO1