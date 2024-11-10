/*
*
* https://github.com/adafruit/Adafruit-PN532
*
* https://github.com/zonque/pn532-espidf
*
*/

#pragma once

#include "driver/gpio.h"
#include "driver/i2c.h"
#include <string.h>

#define PN532_IRQ GPIO_NUM_17
#define PN532_RESET GPIO_NUM_45

#define PN532_ADDR 0x48

#define PN532_PACKBUFFSIZ 64

#define PN532_MIFARE_ISO14443A (0x00) ///< MiFare
#define PN532_UID_MAX_SIZE 7

/*
#define MIFARE1K_SECTORS 16
#define MIFARE1K_BLOCK_SIZE 16 
#define MIFARE1K_BLOCK_BY_SECTOR 4 
*/

typedef struct
{
    uint8_t ATQA;
    uint8_t SAK;
    uint8_t uid_length;
    uint8_t uid[PN532_UID_MAX_SIZE];
} pn532_record_t;

typedef void (*pn532_callback)(pn532_record_t* record);

void pn532_i2c_init(i2c_port_t _i2c_port, gpio_port_t irq, gpio_port_t reset, SemaphoreHandle_t i2c_lock);
esp_err_t pn532_SAMConfig(void);
esp_err_t pn532_send_command_check_ack(uint8_t *cmd, size_t cmd_lenght, uint16_t timeout);
uint32_t pn532_get_firmware_version(void);
esp_err_t pn532_set_passive_activation_retries(uint8_t max_retries);
const pn532_record_t *pn532_read_passive_targetID(uint8_t cardbaudrate, uint16_t timeout);
void pn532_background_read_passive_targetID(uint8_t cardbaudrate, uint16_t timeout, pn532_callback callback);
const pn532_record_t *pn532_get_last_record(void);