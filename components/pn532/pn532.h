/*
 *
 * https://github.com/adafruit/Adafruit-PN532
 *
 * https://github.com/zonque/pn532-espidf
 *
 * https://www.nxp.com/docs/en/user-guide/141520.pdf
 *
 */

#pragma once

#include "driver/gpio.h"
#include "driver/i2c.h"
#include <string.h>

#define PN532_IRQ GPIO_NUM_17
#define PN532_RESET GPIO_NUM_45

#define PN532_PACKBUFFSIZ 64

#define PN532_MIFARE_ISO14443A (0x00) // MiFare
#define PN532_FELICA_212 (0x01)       // Felica 212kbps baudrate
#define PN532_FELICA_424 (0x02)       // Felica 424kbps baudrate
#define PN532_ISO14443_3B (0x03)      // 106kbps type B
#define PN532_INNOVISION_JEWEL (0x04) // 106kbps Innovision Jewel Tag

#define PN532_UID_MAX_SIZE 10
#define PN532_UID_4 4
#define PN532_UID_7 7
#define PN532_UID_10 10

#define PN532_MAX_CTRLED_TAGS 2

#define MIFARE1K_SECTORS 16
#define MFC_BLOCK_BY_SECTOR 4
#define MFC_BLOCK_SIZE 16

#define MFC_KEY_SIZE 6

typedef enum {
  TYPE_UNKNOWN,
  TYPE_NOT_COMPLETE,
  TYPE_MIFARE_MINI,
  TYPE_MIFARE_1K,
  TYPE_MIFARE_4K,
  TYPE_MIFARE_UL,
  TYPE_MIFARE_PLUS,
  TYPE_TNP3XXX,
  TYPE_ISO_14443_4,
  TYPE_ISO_18092,
  TYPE_SIZE
} pn532_type_t;

typedef struct {
  uint16_t ATQA;
  uint8_t SAK;
  uint8_t uid_length;
  uint8_t uid[PN532_UID_MAX_SIZE];
} pn532_record_t;

/* General Status
 * https://www.nxp.com/docs/en/user-guide/141520.pdf page 74
 */

typedef struct {
  uint8_t tag_index;
  uint8_t rx_bitrate;
  uint8_t tx_bitrate;
  uint8_t modulation_type;
} pn532_tag_status_t;

typedef struct {
  uint8_t last_error;
  uint8_t field_present;
  uint8_t nb_tags; // number of tags currently controlled
  pn532_tag_status_t tag_status[PN532_MAX_CTRLED_TAGS];
  uint8_t SAM_status;
} pn532_general_status_t;

/* SAM Config
 * https://www.nxp.com/docs/en/user-guide/141520.pdf page 89
 */

typedef enum {
  SAM_NORMAL_MODE = 0x01,
  SAM_VIRTUAL_CARD = 0x02,
  SAM_WIRED_CARD = 0x03,
  SAM_DUAL_CARD = 0x04,
} pn532_SAM_mode_t;

typedef void (*pn532_callback)(const pn532_record_t *record);

void pn532_i2c_init(i2c_port_t _i2c_port, gpio_port_t irq, gpio_port_t reset,
                    SemaphoreHandle_t i2c_lock);

// Command 0x00 Diagnose
/* TBD */

// Command 0x02 GetFirmwareVersion
uint32_t pn532_get_firmware_version(void);

// Command 0x04 GetGeneralStatus
const pn532_general_status_t *pn532_get_general_status(void);

// Command 0x06 ReadRegister
/* TBD */

// Command 0x08 WriteRegister
/* TBD */

// Command 0x0C ReadGPIO
/* TBD */

// Command 0x0E WriteGPIO
/* TBD */

// Command 0x10 SetSerialBaudrate
/* TBD */

// Command 0x12 SetParameters
/* TBD */

// Command 0x14 SAMConfiguration
/// @brief Set the SAM config
/// @param mode SAM mode
/// @param timeout value = timeout*50ms 0 = infinite timeout
/// @param irq Use IRQ pin or not
/// @return ESP_OK if OK ESP_FAIL instead
esp_err_t pn532_SAM_config(pn532_SAM_mode_t mode, uint8_t timeout, bool irq);

esp_err_t pn532_set_passive_activation_retries(uint8_t max_retries);

void pn532_background_read_passive_targetID(uint8_t cardbaudrate,
                                            uint16_t timeout,
                                            pn532_callback callback);
esp_err_t pn532_send_command_check_ack(uint8_t *cmd, size_t cmd_lenght,
                                       uint16_t timeout);

esp_err_t pn532_mfc_authenticate_block(uint8_t *uid, uint8_t uidLen,
                                       uint32_t blockNumber, uint8_t keyNumber,
                                       const uint8_t *keyData);
esp_err_t pn532_mfc_read_data_block(uint8_t blockNumber, uint8_t *data);

void pn532_cancel_read_task(void);
const pn532_record_t *pn532_get_last_record(void);
void pn532_get_last_uid_string(char *buffer);

pn532_type_t pn532_get_type(const pn532_record_t *record);
pn532_type_t pn532_get_last_type(void);
void pn532_get_type_string(const pn532_record_t *record, char *buffer);
void pn532_get_last_type_string(char *buffer);