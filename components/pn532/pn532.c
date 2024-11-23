#include "pn532.h"
#include "pn532_registers.h"

// Common ESP-IDF Helpers
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "pn532";

static uint8_t uid[PN532_UID_MAX_SIZE]; // ISO14443A uid
static uint8_t uidLen;
static uint8_t key[MFC_KEY_SIZE];

static gpio_port_t irq = -1;
static gpio_port_t reset = -1;
static i2c_port_t i2c_port = -1;
static SemaphoreHandle_t i2c_lock = NULL;

static uint8_t pn532ack[] = {0x00, 0x00, 0xFF,
                             0x00, 0xFF, 0x00}; ///< ACK message from PN532

static uint8_t pn532_packetbuffer[PN532_PACKBUFFSIZ];
static pn532_record_t pn532_last_record;
static pn532_general_status_t pn532_last_status;

static uint8_t cardbaudrate;
static uint16_t timeout;
static pn532_callback callback;
static TaskHandle_t read_task = NULL;

static void pn532_hardware_reset(void);
static esp_err_t pn532_write_command(uint8_t *cmd, size_t cmd_length);
static esp_err_t pn532_read_data(uint8_t *buffer, size_t buffer_size);
static bool pn532_read_ack(void);

static void pn532_launch_read_task(void *pvParams);
static const pn532_record_t *pn532_read_passive_targetID(uint8_t cardbaudrate,
                                                         uint16_t timeout);

static bool pn532_is_ready(void);
static bool pn532_wait_ready(uint16_t timeout);

static void pn532_hardware_reset() {
  gpio_set_level(reset, 1);
  gpio_set_level(reset, 0);
  vTaskDelay(400 / portTICK_PERIOD_MS);
  gpio_set_level(reset, 1);
  // https://www.nxp.com/docs/en/user-guide/141520.pdf page 50 T_osc_start
  vTaskDelay(10 / portTICK_PERIOD_MS);
}

static esp_err_t pn532_write_command(uint8_t *cmd, size_t cmd_length) {

  esp_err_t result = ESP_OK;

  uint8_t *command = (uint8_t *)malloc(cmd_length + 9);
  memset(command, 0, cmd_length + 9);

  uint8_t checksum;
  checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;

  // https://www.nxp.com/docs/en/user-guide/141520.pdf page 65
  command[0] = PN532_I2C_ADDRESS;
  command[1] = PN532_PREAMBLE;
  command[2] = PN532_PREAMBLE;
  command[3] = PN532_STARTCODE2;
  command[4] = (cmd_length + 1);
  command[5] = ~(cmd_length + 1) + 1;
  command[6] = PN532_HOSTTOPN532;
  checksum += PN532_HOSTTOPN532;

  for (uint8_t i = 0; i < cmd_length; i++) {
    command[i + 7] = cmd[i];
    checksum += cmd[i];
  }

  command[(cmd_length - 1) + 8] = ~checksum;
  command[(cmd_length - 1) + 9] = PN532_POSTAMBLE;

  xSemaphoreTake(i2c_lock, portMAX_DELAY);

  i2c_cmd_handle_t i2ccmd = i2c_cmd_link_create();
  result = i2c_master_start(i2ccmd);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot start i2c master", __func__);
    free(command);
    i2c_cmd_link_delete(i2ccmd);
    xSemaphoreGive(i2c_lock);
    return result;
  }

  for (uint8_t i = 0; i < cmd_length + 9; i++) {
    result = i2c_master_write_byte(i2ccmd, command[i], true);
    if (result != ESP_OK) {
      ESP_LOGE(TAG, "%s: Cannot write byte: 0x%02x", __func__, command[i]);
      free(command);
      i2c_cmd_link_delete(i2ccmd);
      xSemaphoreGive(i2c_lock);
      return result;
    }
  }

  result = i2c_master_stop(i2ccmd);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot stop i2c master", __func__);
    free(command);
    i2c_cmd_link_delete(i2ccmd);
    xSemaphoreGive(i2c_lock);
    return result;
  }

  result = i2c_master_cmd_begin(i2c_port, i2ccmd,
                                I2C_WRITE_TIMEOUT / portTICK_PERIOD_MS);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot begin i2c master", __func__);
    free(command);
    i2c_cmd_link_delete(i2ccmd);
    xSemaphoreGive(i2c_lock);
    return result;
  }

  i2c_cmd_link_delete(i2ccmd);
  xSemaphoreGive(i2c_lock);
  free(command);
  return result;
}

static esp_err_t pn532_read_data(uint8_t *buff, size_t buffer_size) {

  esp_err_t result = ESP_OK;
  // vTaskDelay(10 / portTICK_PERIOD_MS);

  uint8_t *buffer = (uint8_t *)malloc(buffer_size + 3);
  memset(buffer, 0, buffer_size + 3);
  memset(buff, 0, buffer_size);

  xSemaphoreTake(i2c_lock, portMAX_DELAY);

  i2c_cmd_handle_t i2ccmd = i2c_cmd_link_create();
  result = i2c_master_start(i2ccmd);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot start i2c master", __func__);
    i2c_cmd_link_delete(i2ccmd);
    xSemaphoreGive(i2c_lock);
    free(buffer);
    return result;
  }

  result = i2c_master_write_byte(i2ccmd, PN532_I2C_READ_ADDRESS, true);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot write byte:0x%02x", __func__,
             PN532_I2C_READ_ADDRESS);
    i2c_cmd_link_delete(i2ccmd);
    xSemaphoreGive(i2c_lock);
    free(buffer);
    return result;
  }

  for (uint8_t i = 0; i < (buffer_size + 2); i++) {
    result = i2c_master_read_byte(i2ccmd, &buffer[i], I2C_MASTER_ACK);
    if (result != ESP_OK) {
      ESP_LOGE(TAG, "%s: Cannot read byte:%d", __func__, i);
      i2c_cmd_link_delete(i2ccmd);
      xSemaphoreGive(i2c_lock);
      free(buffer);
      return result;
    }
  }

  result = i2c_master_read_byte(i2ccmd, &buffer[buffer_size + 2],
                                I2C_MASTER_LAST_NACK);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot read byte:%d", __func__, buffer_size + 2);
    i2c_cmd_link_delete(i2ccmd);
    xSemaphoreGive(i2c_lock);
    free(buffer);
    return result;
  }

  result = i2c_master_stop(i2ccmd);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot stop i2c master", __func__);
    i2c_cmd_link_delete(i2ccmd);
    xSemaphoreGive(i2c_lock);
    free(buffer);
    return result;
  }

  result = i2c_master_cmd_begin(i2c_port, i2ccmd,
                                I2C_READ_TIMEOUT / portTICK_PERIOD_MS);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: Cannot begin i2c master", __func__);
    i2c_cmd_link_delete(i2ccmd);
    xSemaphoreGive(i2c_lock);
    free(buffer);
    return result;
  }

  i2c_cmd_link_delete(i2ccmd);
  xSemaphoreGive(i2c_lock);
  memcpy(buff, buffer + 1, buffer_size);
  free(buffer);
  return result;
}

static bool pn532_read_ack(void) {
  uint8_t ackbuff[6];
  esp_err_t result = pn532_read_data(ackbuff, 6);
  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: error:0x%02x %s", __func__, result,
             esp_err_to_name(result));
    return false;
  }
  if (memcmp((char *)ackbuff, (char *)pn532ack, 6) != 0) {
    ESP_LOGE(TAG, "%s: received ack:0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",
             __func__, ackbuff[0], ackbuff[1], ackbuff[2], ackbuff[3],
             ackbuff[4], ackbuff[5]);
    return false;
  }
  return true;
}

esp_err_t pn532_send_command_check_ack(uint8_t *cmd, size_t cmd_lenght,
                                       uint16_t timeout) {
  esp_err_t result = ESP_OK;

  result = pn532_write_command(cmd, cmd_lenght);
  vTaskDelay(1 / portTICK_PERIOD_MS);

  if (result != ESP_OK) {
    ESP_LOGE(TAG, "%s: error:0x%02x %s", __func__, result,
             esp_err_to_name(result));
    return result;
  }

  if (!pn532_wait_ready(timeout)) {
    ESP_LOGE(TAG, "%s Not ready in time", __func__);
    return ESP_FAIL;
  }

  if (!pn532_read_ack()) {
    ESP_LOGE(TAG, "%s: No ACK Frame received", __func__);
    return ESP_FAIL;
  }
  vTaskDelay(1 / portTICK_PERIOD_MS);

  if (!pn532_wait_ready(timeout)) {
    ESP_LOGE(TAG, "%s Not ready in time 2", __func__);
    return ESP_FAIL;
  }

  return ESP_OK;
}

static void pn532_launch_read_task(void *pvParams) {
  const pn532_record_t *to_return =
      pn532_read_passive_targetID(cardbaudrate, timeout);
  if (to_return == NULL) {
    callback(pn532_get_last_record());
  } else {
    callback(to_return);
  }
  vTaskDelete(read_task);
  read_task = NULL;
}

void pn532_cancel_read_task() {
  if (read_task != NULL) {
    vTaskDelete(read_task);
    read_task = NULL;
  }
}

static bool pn532_is_ready() { return (gpio_get_level(irq) == 0x00); }

static bool pn532_wait_ready(uint16_t timeout) {
  uint16_t timer = 0;
  while (!pn532_is_ready()) {
    if (timeout != 0) {
      timer += 10;
      if (timer > timeout) {
        ESP_LOGE(TAG, "%s: Wait ready timeout", __func__);
        return false;
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  return true;
}

void pn532_i2c_init(i2c_port_t _i2c_port, gpio_port_t _irq, gpio_port_t _reset,
                    SemaphoreHandle_t _i2c_lock) {
  i2c_lock = _i2c_lock;
  i2c_port = _i2c_port;
  irq = _irq;
  reset = _reset;
  memset(&pn532_last_record, 0, sizeof(pn532_record_t));

  gpio_set_direction(irq, GPIO_MODE_INPUT);
  gpio_set_direction(reset, GPIO_MODE_OUTPUT);

  pn532_hardware_reset();
  vTaskDelay(10 / portTICK_PERIOD_MS);
  i2c_set_timeout(i2c_port, 0x1f);
  pn532_SAM_config(SAM_NORMAL_MODE, 0x14, true);
}

uint32_t pn532_get_firmware_version() {
  static const uint8_t pn532response_firmwarevers[] = {0x00, 0xFF, 0x06,
                                                       0xFA, 0xD5, 0x03};
  uint32_t response;

  pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

  if (pn532_send_command_check_ack(pn532_packetbuffer, 1, I2C_WRITE_TIMEOUT) !=
      ESP_OK) {
    ESP_LOGE(TAG, "%s: send command check ack failed", __func__);
    return 0;
  }

  if (pn532_read_data(pn532_packetbuffer, 12) != ESP_OK) {
    ESP_LOGE(TAG, "%s: Unable to read data", __func__);
    return 0;
  }

  // check some basic stuff
  if (0 != strncmp((char *)pn532_packetbuffer,
                   (char *)pn532response_firmwarevers, 6)) {
    ESP_LOGE(TAG, "%s: Basic stuff failed", __func__);
    return 0;
  }

  int offset = 7; // Skip a response byte when using I2C to ignore extra data.
  response = pn532_packetbuffer[offset++];
  response <<= 8;
  response |= pn532_packetbuffer[offset++];
  response <<= 8;
  response |= pn532_packetbuffer[offset++];
  response <<= 8;
  response |= pn532_packetbuffer[offset++];

  return response;
}

esp_err_t pn532_SAM_config(pn532_SAM_mode_t mode, uint8_t timeout, bool irq) {
  pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
  pn532_packetbuffer[1] = mode;
  pn532_packetbuffer[2] = timeout;      // timeout * 50ms
  pn532_packetbuffer[3] = (uint8_t)irq; // use IRQ pin?

  if (pn532_send_command_check_ack(pn532_packetbuffer, 4, 200) != ESP_OK) {
    ESP_LOGE(TAG, "%s: send command check ack failed", __func__);
    return ESP_FAIL;
  }

  if (pn532_read_data(pn532_packetbuffer, 9) != ESP_OK) {
    ESP_LOGE(TAG, "%s: Unable to read data", __func__);
    return ESP_FAIL;
  }
  uint8_t offset = 6;
  if (pn532_packetbuffer[offset] == 0x15) {
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t pn532_set_passive_activation_retries(uint8_t max_retries) {
  pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
  pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
  pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
  pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
  pn532_packetbuffer[4] = max_retries;

  if (pn532_send_command_check_ack(pn532_packetbuffer, 5, I2C_WRITE_TIMEOUT) !=
      ESP_OK) {
    ESP_LOGE(TAG, "%s: send command check ack failed", __func__);
    return ESP_FAIL;
  }
  return ESP_OK;
}

static const pn532_record_t *pn532_read_passive_targetID(uint8_t cardbaudrate,
                                                         uint16_t timeout) {
  pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
  pn532_packetbuffer[1] = 1; // max 1 cards at once (we can set this to 2 later)
  pn532_packetbuffer[2] = cardbaudrate;
  if (pn532_send_command_check_ack(pn532_packetbuffer, 3, timeout) != ESP_OK) {
    ESP_LOGE(TAG, "%s: send command check ack failed", __func__);
    return NULL;
  }

  if (!pn532_wait_ready(timeout)) {
    ESP_LOGE(TAG, "%s: Wait ready failed", __func__);
    return NULL;
  }

  // read data packet
  if (pn532_read_data(pn532_packetbuffer, 20) != ESP_OK) {
    ESP_LOGE(TAG, "%s: Unable to read data", __func__);
    return NULL;
  }

  ESP_LOGI(TAG, "%s: Found %d tags", __func__, pn532_packetbuffer[7]);

  if (pn532_packetbuffer[7] != 1) {
    ESP_LOGE(TAG, "%s: Response not ok:0x%02x\n", __func__,
             pn532_packetbuffer[7]);
    return NULL;
  }

  uint16_t sens_res = pn532_packetbuffer[9];
  sens_res <<= 8;
  sens_res |= pn532_packetbuffer[10];

  pn532_last_record.ATQA = sens_res;
  pn532_last_record.SAK = pn532_packetbuffer[11];
  pn532_last_record.uid_length = pn532_packetbuffer[12];

  for (uint8_t i = 0; i < pn532_packetbuffer[12]; i++) {
    pn532_last_record.uid[i] = pn532_packetbuffer[13 + i];
  }
  return &pn532_last_record;
}

const pn532_record_t *pn532_get_last_record() { return &pn532_last_record; }

void pn532_background_read_passive_targetID(uint8_t _cardbaudrate,
                                            uint16_t _timeout,
                                            pn532_callback _callback) {
  cardbaudrate = _cardbaudrate;
  timeout = _timeout;
  callback = _callback;
  xTaskCreate(pn532_launch_read_task, "PN532ReadTask", 4096, NULL, 4,
              &read_task);
}

esp_err_t pn532_mfc_authenticate_block(uint8_t *_uid, uint8_t _uidLen,
                                       uint32_t blockNumber, uint8_t keyNumber,
                                       const uint8_t *keyData) {
  memcpy(key, keyData, 6);
  memcpy(uid, _uid, _uidLen);
  uidLen = _uidLen;

  // Prepare the authentication command //
  pn532_packetbuffer[0] =
      PN532_COMMAND_INDATAEXCHANGE; /* Data Exchange Header */
  pn532_packetbuffer[1] = 1;        /* Max card numbers */
  pn532_packetbuffer[2] = (keyNumber) ? MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
  pn532_packetbuffer[3] =
      blockNumber; /* Block Number (1K = 0..63, 4K = 0..255 */
  memcpy(pn532_packetbuffer + 4, key, 6);
  for (uint8_t i = 0; i < uidLen; i++) {
    pn532_packetbuffer[10 + i] = uid[i]; /* 4 byte card ID */
  }

  if (pn532_send_command_check_ack(pn532_packetbuffer, 10 + uidLen,
                                   I2C_WRITE_TIMEOUT) != ESP_OK) {
    ESP_LOGE(TAG, "%s: send command check ack failed\n", __func__);
    return ESP_FAIL;
  }

  if (pn532_read_data(pn532_packetbuffer, 12) != ESP_OK) {
    ESP_LOGE(TAG, "%s: Unable to read data", __func__);
    return ESP_FAIL;
  }

  if (pn532_packetbuffer[7] != 0x00) {
    ESP_LOGE(TAG, "%s: bad key:%d block:%ld checkbyte: 0x%02x\n", __func__,
             keyNumber, blockNumber, pn532_packetbuffer[7]);
    return ESP_FAIL;
  }
  return ESP_OK;
}

esp_err_t pn532_mfc_read_data_block(uint8_t blockNumber, uint8_t *data) {
  pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = 1;               /* Card number */
  pn532_packetbuffer[2] = MIFARE_CMD_READ; /* Mifare Read command = 0x30 */
  pn532_packetbuffer[3] =
      blockNumber; /* Block Number (0..63 for 1K, 0..255 for 4K) */

  if (pn532_send_command_check_ack(pn532_packetbuffer, 4, I2C_WRITE_TIMEOUT) !=
      ESP_OK) {
    ESP_LOGE(TAG, "%s: send command check ack failed", __func__);
    return ESP_FAIL;
  }

  if (pn532_read_data(pn532_packetbuffer, 26) != ESP_OK) {
    ESP_LOGE(TAG, "%s: Unable to read data", __func__);
    return ESP_FAIL;
  }

  uint8_t check_byte = pn532_packetbuffer[7];
  if (check_byte != 0x00) {
    ESP_LOGE(TAG, "%s: %d\terror byte:0x%02x\tNAD:%d\tMI:%d", __func__,
             blockNumber, check_byte, (check_byte && 0x80),
             (check_byte && 0x20));
    return ESP_FAIL;
  }
  /* Copy the 16 data bytes to the output buffer        */
  /* Block content starts at byte 9 of a valid response */
  memcpy(data, pn532_packetbuffer + 8, MFC_BLOCK_SIZE);

  return ESP_OK;
}

const pn532_general_status_t *pn532_get_general_status(void) {

  pn532_packetbuffer[0] = PN532_COMMAND_GETGENERALSTATUS;
  if (pn532_send_command_check_ack(pn532_packetbuffer, 1, I2C_WRITE_TIMEOUT) !=
      ESP_OK) {
    ESP_LOGE(TAG, "%s: send command check ack failed", __func__);
    return NULL;
  }

  if (pn532_read_data(pn532_packetbuffer, 22) != ESP_OK) {
    ESP_LOGE(TAG, "%s: Unable to read data", __func__);
    return NULL;
  }

  int offset = 7; // Skip a response byte when using I2C to ignore extra data.
  pn532_last_status.last_error = pn532_packetbuffer[offset++];
  pn532_last_status.field_present = pn532_packetbuffer[offset++];
  pn532_last_status.nb_tags = pn532_packetbuffer[offset++];
  for (size_t i = 0; i < pn532_last_status.nb_tags; i++) {
    pn532_last_status.tag_status[i].tag_index = pn532_packetbuffer[offset++];
    pn532_last_status.tag_status[i].rx_bitrate = pn532_packetbuffer[offset++];
    pn532_last_status.tag_status[i].tx_bitrate = pn532_packetbuffer[offset++];
    pn532_last_status.tag_status[i].modulation_type =
        pn532_packetbuffer[offset++];
  }

  pn532_last_status.SAM_status = pn532_packetbuffer[offset];
  return &pn532_last_status;
}

void pn532_get_last_uid_string(char *buffer) {
  size_t nb_char = 0;
  char temp[] = "00";
  for (size_t i = 0; i < pn532_last_record.uid_length; i++) {
    sprintf(temp, "%02x", pn532_last_record.uid[i]);
    strcat(buffer, temp);
    nb_char += sizeof(temp) - 1;
  }
  buffer[nb_char] = '\0';
  return;
}

pn532_type_t pn532_get_type(const pn532_record_t *record) {
  uint8_t sak = (record->SAK & 0x7F);
  switch (sak) {
  case 0x04:
    return TYPE_NOT_COMPLETE; // UID not complete
  case 0x09:
    return TYPE_MIFARE_MINI;
  case 0x08:
    return TYPE_MIFARE_1K;
  case 0x18:
    return TYPE_MIFARE_4K;
  case 0x00:
    return TYPE_MIFARE_UL;
  case 0x10:
  case 0x11:
    return TYPE_MIFARE_PLUS;
  case 0x01:
    return TYPE_TNP3XXX;
  case 0x20:
    return TYPE_ISO_14443_4;
  case 0x40:
    return TYPE_ISO_18092;
  default:
    return TYPE_UNKNOWN;
    break;
  }
}

pn532_type_t pn532_get_last_type(void) {
  return pn532_get_type(&pn532_last_record);
}

void pn532_get_type_string(const pn532_record_t *record, char *buffer) {
  char type[20];
  switch (pn532_get_type(record)) {
  case TYPE_NOT_COMPLETE:
    sprintf(type, "NOT COMPLETE");
    break;

  case TYPE_MIFARE_MINI:
    sprintf(type, "MIFARE MINI");
    break;

  case TYPE_MIFARE_1K:
    sprintf(type, "MIFARE 1K");
    break;

  case TYPE_MIFARE_4K:
    sprintf(type, "MIFARE 4K");
    break;

  case TYPE_MIFARE_UL:
    sprintf(type, "MIFARE UTRALIGHT");
    break;

  case TYPE_MIFARE_PLUS:
    sprintf(type, "MIFARE PLUS");
    break;

  case TYPE_TNP3XXX:
    sprintf(type, "TNP3XXX");
    break;

  case TYPE_ISO_14443_4:
    sprintf(type, "ISO14443_4");
    break;

  case TYPE_ISO_18092:
    sprintf(type, "ISO18092");
    break;

  default:
    sprintf(type, "UNKNOWN");
    break;
  }
  sprintf(buffer, "%s", type);
}

void pn532_get_last_type_string(char *buffer) {
  pn532_get_type_string(&pn532_last_record, buffer);
}