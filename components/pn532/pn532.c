#include "pn532.h"
#include "pn532_registers.h"

// Common ESP-IDF Helpers
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "pn532";

static gpio_port_t irq = -1;
static gpio_port_t reset = -1;
static i2c_port_t i2c_port = -1;
static SemaphoreHandle_t i2c_lock = NULL;

static uint8_t pn532ack[] = {0x00, 0x00, 0xFF,
                             0x00, 0xFF, 0x00}; ///< ACK message from PN532
static uint8_t pn532_packetbuffer[PN532_PACKBUFFSIZ];

static pn532_record_t pn532_last_record;

static uint8_t cardbaudrate;
static uint16_t timeout;
static pn532_callback callback;

static esp_err_t pn532_write_command(uint8_t *cmd, size_t cmd_length);
static void pn532_hardware_reset(void);
static esp_err_t pn532_read_data(uint8_t *buffer, size_t buffer_size);

static void pn532_launch_read_task(void *pvParams);

static bool pn532_is_ready(void);
static bool pn532_wait_ready(uint16_t timeout);
static bool pn532_read_ack(void);

static void pn532_launch_read_task(void *pvParams)
{
    callback(pn532_read_passive_targetID(cardbaudrate, timeout));
    vTaskDelete(NULL);
}

static esp_err_t pn532_read_data(uint8_t *buff, size_t buffer_size)
{
    i2c_cmd_handle_t i2ccmd;
    uint8_t *buffer = (uint8_t *)malloc(buffer_size + 3);

    while (xSemaphoreTake(i2c_lock, portMAX_DELAY) != pdPASS)
    {
        ESP_LOGI(TAG, "Read cannot take the lock");
        return ESP_FAIL;
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
    bzero(buffer, buffer_size + 3);
    bzero(buff, buffer_size);

    i2ccmd = i2c_cmd_link_create();
    i2c_master_start(i2ccmd);
    i2c_master_write_byte(i2ccmd, PN532_I2C_READ_ADDRESS, true);
    for (uint8_t i = 0; i < (buffer_size + 2); i++)
        i2c_master_read_byte(i2ccmd, &buffer[i], I2C_MASTER_ACK);

    i2c_master_read_byte(i2ccmd, &buffer[buffer_size + 2], I2C_MASTER_LAST_NACK);
    i2c_master_stop(i2ccmd);

    if (i2c_master_cmd_begin(i2c_port, i2ccmd, I2C_READ_TIMEOUT / portTICK_PERIOD_MS) != ESP_OK)
    {
        // Reset i2c bus
        i2c_cmd_link_delete(i2ccmd);
        xSemaphoreGive(i2c_lock);
        free(buffer);
        return ESP_FAIL;
    };

    i2c_cmd_link_delete(i2ccmd);
    xSemaphoreGive(i2c_lock);

    memcpy(buff, buffer + 1, buffer_size);

    // Start read (n+1 to take into account leading 0x01 with I2C)
    // esp_log_buffer_hex("PN532 read", buffer, buffer_size + 3);
    free(buffer);
    return ESP_OK;
}

static bool pn532_read_ack(void)
{
    uint8_t ackbuff[6];
    pn532_read_data(ackbuff, 6);
    return (0 == memcmp((char *)ackbuff, (char *)pn532ack, 6));
}

static esp_err_t pn532_write_command(uint8_t *cmd, size_t cmd_length)
{
    // I2C command write.
    uint8_t checksum;

    // Create the command
    uint8_t *command = (uint8_t *)malloc(cmd_length + 9);
    bzero(command, cmd_length + 9);

    vTaskDelay(10 / portTICK_PERIOD_MS);
    checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;

    command[0] = PN532_I2C_ADDRESS;
    command[1] = PN532_PREAMBLE;
    command[2] = PN532_PREAMBLE;
    command[3] = PN532_STARTCODE2;
    command[4] = (cmd_length + 1);
    command[5] = ~(cmd_length + 1) + 1;
    command[6] = PN532_HOSTTOPN532;
    checksum += PN532_HOSTTOPN532;

    for (uint8_t i = 0; i < cmd_length; i++)
    {
        command[i + 7] = cmd[i];
        checksum += cmd[i];
    }

    command[(cmd_length - 1) + 8] = ~checksum;
    command[(cmd_length - 1) + 9] = PN532_POSTAMBLE;

    while (xSemaphoreTake(i2c_lock, portMAX_DELAY) != pdPASS)
    {
        ESP_LOGI(TAG, "Write cannot take the lock");
        return ESP_FAIL;
    }

    i2c_cmd_handle_t i2ccmd = i2c_cmd_link_create();
    i2c_master_start(i2ccmd);
    i2c_master_write_byte(i2ccmd, command[0], true);

    for (uint8_t i = 1; i < cmd_length + 9; i++)
        i2c_master_write_byte(i2ccmd, command[i], true);

    i2c_master_stop(i2ccmd);
    // esp_log_buffer_hex("PN532 write", command, cmd_length + 9);

    esp_err_t result = ESP_OK;
    result = i2c_master_cmd_begin(i2c_port, i2ccmd, I2C_WRITE_TIMEOUT / portTICK_PERIOD_MS);

    if (result != ESP_OK)
    {
        const char *resultText = NULL;
        switch (result)
        {
        case ESP_ERR_INVALID_ARG:
            resultText = "Parameter error";
            break;
        case ESP_FAIL:
            resultText = "Sending command error, slave doesn’t ACK the transfer.";
            break;
        case ESP_ERR_INVALID_STATE:
            resultText = "I2C driver not installed or not in master mode.";
            break;
        case ESP_ERR_TIMEOUT:
            resultText = "Operation timeout because the bus is busy. ";
            break;
        }

        ESP_LOGE("PN532", "%s I2C write failed: %s", __func__, resultText);
    }

    i2c_cmd_link_delete(i2ccmd);
    xSemaphoreGive(i2c_lock);

    free(command);
    return result;
}

esp_err_t pn532_send_command_check_ack(uint8_t *cmd, size_t cmd_lenght, uint16_t timeout)
{
    pn532_write_command(cmd, cmd_lenght);
    if (!pn532_wait_ready(timeout))
    {
        ESP_LOGD(TAG, "Not ready in time");
        return ESP_FAIL;
    }
    if (!pn532_read_ack())
    {
        ESP_LOGD(TAG, "No ACK Frame received");
        return ESP_FAIL;
    }
    return ESP_OK;
}

static bool pn532_is_ready()
{
    return (gpio_get_level(PN532_IRQ) == 0x00);
}

static bool pn532_wait_ready(uint16_t timeout)
{
    uint16_t timer = 0;
    while (!pn532_is_ready())
    {
        if (timeout != 0)
        {
            timer += 10;
            if (timer > timeout)
            {
                ESP_LOGI(TAG, "Wait ready timeout");
                return false;
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    return true;
}

static void pn532_hardware_reset()
{
    gpio_set_level(reset, 1);
    gpio_set_level(reset, 0);
    vTaskDelay(400 / portTICK_PERIOD_MS);
    gpio_set_level(reset, 1);
    vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay required before taking other actions after reset.
                                         //	 See timing diagram on page 209 of the datasheet, section 12.23.
}

void pn532_i2c_init(i2c_port_t _i2c_port, gpio_port_t _irq, gpio_port_t _reset, SemaphoreHandle_t _i2c_lock)
{
    i2c_lock = _i2c_lock;
    i2c_port = _i2c_port;
    irq = _irq;
    reset = _reset;

    gpio_set_direction(irq, GPIO_MODE_INPUT);
    gpio_set_direction(reset, GPIO_MODE_OUTPUT);

    pn532_hardware_reset();
    vTaskDelay(10 / portTICK_PERIOD_MS);
    i2c_set_timeout(i2c_port, 0x1f);
    pn532_SAMConfig();
}

uint32_t pn532_get_firmware_version()
{
    static const uint8_t pn532response_firmwarevers[] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};
    uint32_t response;

    pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

    if (pn532_send_command_check_ack(pn532_packetbuffer, 1, I2C_WRITE_TIMEOUT) != ESP_OK)
    {
        ESP_LOGI(TAG, "get firmware version send command check ack failed");
        return 0;
    }

    pn532_read_data(pn532_packetbuffer, 12);

    // check some basic stuff
    if (0 != strncmp((char *)pn532_packetbuffer, (char *)pn532response_firmwarevers, 6))
    {
        ESP_LOGD(TAG, "Basic stuff failed");
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

esp_err_t pn532_SAMConfig()
{
    pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01; // normal mode;
    pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    pn532_packetbuffer[3] = 0x01; // use IRQ pin!

    if (!pn532_send_command_check_ack(pn532_packetbuffer, 4, 200))
    {
        return ESP_FAIL;
    }
    pn532_read_data(pn532_packetbuffer, 9);
    uint8_t offset = 6;
    return (pn532_packetbuffer[offset] == 0x15);
}

esp_err_t pn532_set_passive_activation_retries(uint8_t max_retries)
{
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
    pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
    pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
    pn532_packetbuffer[4] = max_retries;

    if (!pn532_send_command_check_ack(pn532_packetbuffer, 5, I2C_WRITE_TIMEOUT))
    {
        return ESP_FAIL;
    }
    return ESP_OK;
}

const pn532_record_t *pn532_read_passive_targetID(uint8_t cardbaudrate, uint16_t timeout)
{
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1; // max 1 cards at once (we can set this to 2 later)
    pn532_packetbuffer[2] = cardbaudrate;

    if (pn532_send_command_check_ack(pn532_packetbuffer, 3, timeout) != ESP_OK)
    {
        ESP_LOGI(TAG, "No card(s) read");
        return NULL;
    }

    if (!pn532_wait_ready(timeout))
    {
        return NULL;
    }

    // read data packet
    if (pn532_read_data(pn532_packetbuffer, 20) != ESP_OK)
    {
        ESP_LOGI(TAG, "Read Data issue");
        return NULL;
    }

    /* ISO14443A card response should be in the following format:

    byte            Description
    -------------   ------------------------------------------
    b0..6           Frame header and preamble
    b7              Tags Found
    b8              Tag Number (only one used in this example)
    b9..10          SENS_RES
    b11             SEL_RES
    b12             NFCID Length
    b13..NFCIDLen   NFCID                                      */

    ESP_LOGD(TAG, "Found %d tags", pn532_packetbuffer[7]);

    if (pn532_packetbuffer[7] != 1)
    {
        ESP_LOGI(TAG, "Eighth packetbuffer not ok:0x%02x\n", pn532_packetbuffer[7]);
        return NULL;
    }

    uint16_t sens_res = pn532_packetbuffer[9];
    sens_res <<= 8;
    sens_res |= pn532_packetbuffer[10];

    pn532_last_record.ATQA = sens_res;
    pn532_last_record.SAK = pn532_packetbuffer[11];

    /* Card appears to be Mifare Classic */
    pn532_last_record.uid_length = pn532_packetbuffer[12];

    for (uint8_t i = 0; i < pn532_packetbuffer[12]; i++)
    {
        pn532_last_record.uid[i] = pn532_packetbuffer[13 + i];
    }
    return &pn532_last_record;
}

const pn532_record_t *pn532_get_last_record()
{
    return &pn532_last_record;
}

void pn532_background_read_passive_targetID(uint8_t _cardbaudrate, uint16_t _timeout, pn532_callback _callback)
{
    cardbaudrate = _cardbaudrate;
    timeout = _timeout;
    callback = _callback;
    xTaskCreate(pn532_launch_read_task, "PN532ReadTask", 4096, NULL, 4, NULL);
}