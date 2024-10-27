#include "bq27220.h"

static const char *TAG = "bq27220";

static union battery_state bat_st;
static i2c_port_t i2c_port = -1;

// Write a specified number of bytes over I2C to a given subAddress
static esp_err_t bq27220_I2cWriteBytes(uint8_t subAddress, uint8_t *src, uint8_t count)
{
    // We write data incrementally
    // According to the datasheet this is only ok for f_scl up to 100khz
    // 66us idle time is required between commands
    // this seems to be built in and there are no communication problems
    // if needed in future could use eta_delay_us to busy wait for 66us
    if (i2c_port == -1)
    {
        return ESP_FAIL;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (bq27220_ADDR << 1) | I2C_MASTER_WRITE, 0x1);
    i2c_master_write_byte(cmd, subAddress, 0x1);
    for (int i = 0; i < count; i++)
    {
        i2c_master_write_byte(cmd, src[i], 0x1);
    }
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t bq27220_I2cReadBytes(uint8_t subAddress, uint8_t *dest, uint8_t count)
{
    if (i2c_port == -1)
    {
        return ESP_FAIL;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (bq27220_ADDR << 1) | I2C_MASTER_WRITE, 0x01);
    i2c_master_write_byte(cmd, subAddress, 0x01);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (bq27220_ADDR << 1) | I2C_MASTER_READ, 0x01);
    if (count > 1)
    {
        i2c_master_read(cmd, dest, count - 1, 0x0);
    }
    i2c_master_read_byte(cmd, dest + count - 1, 0X1);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 200 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
}

// Read a 16-bit command word from the bq27220_-G1A
static uint16_t bq27220_ReadWord(uint16_t subAddress)
{
    uint8_t data[2];
    bq27220_I2cReadBytes(subAddress, data, 2);
    return ((uint16_t)data[1] << 8) | data[0];
}

static uint16_t bq27220_ReadControlWord(uint16_t function)
{
    uint8_t subCommandMSB = (function >> 8);
    uint8_t subCommandLSB = (function & 0x00FF);
    uint8_t command[2] = {subCommandLSB, subCommandMSB};
    uint8_t data[2] = {0, 0};

    bq27220_I2cWriteBytes((uint8_t)0, command, 2);

    if (bq27220_I2cReadBytes((uint8_t)0, data, 2))
    {
        return ((uint16_t)data[1] << 8) | data[0];
    }

    return false;
}

esp_err_t bq27220_init(i2c_port_t _i2c_port)
{
    i2c_port = _i2c_port;
    printf("%s:\n \
    Device ID:0x%02x\n \
    Battery State:%d\n \
    Battery temp:%f°C\n \
    Charge State:%d\n \
    Remaining capacity:%dmAh\n \
    Full Capacity:%dmAh\n \
    State of Charge:%d%%\n \
    Voltage:%dV\n \
    Charging Voltage:%dV\n \
    Raw Voltage:%dV\n \
    Raw Current:%dmA\n \
    Instant current:%dmA\n \
    Standby Current:%dmA\n \
    Charging Current:%dmA\n \
    Average Current:%dmA\n",
           TAG,
           bq27220_get_device_id(),
           bq27220_get_battery_state(),
           bq27220_get_temperature(),
           bq27220_get_is_charging(),
           bq27220_get_remaining_capacity(),
           bq27220_get_full_charge_capacity(),
           bq27220_get_state_of_charge(),
           bq27220_get_voltage(VOLT),
           bq27220_get_voltage(VOLT_CHARGING),
           bq27220_get_voltage(VOLT_RAW),
           bq27220_get_current(CURRENT_RAW),
           bq27220_get_current(CURRENT_INSTANT),
           bq27220_get_current(CURRENT_STANDBY),
           bq27220_get_current(CURRENT_CHARGING),
           bq27220_get_current(CURRENT_AVERAGE));
    // Guidance from https://github.com/espressif/esp-idf/issues/11397
    printf("%d %d %d %d %d %d %d %d %d\n", bat_st.st.DSG, bat_st.st.SYSDWN, bat_st.st.TDA, bat_st.st.BATTPRES,
           bat_st.st.AUTH_GD, bat_st.st.OCVGD, bat_st.st.TCA, bat_st.st.RSVD, bat_st.st.FD);
    return i2c_set_timeout(i2c_port, 0x1f);
}

uint16_t bq27220_get_voltage(bq27220_voltage_mode_t mode)
{
    switch (mode)
    {
    case VOLT:
        return bq27220_ReadWord(BQ27220_COMMAND_VOLT);
        break;

    case VOLT_CHARGING:
        return bq27220_ReadWord(BQ27220_COMMAND_CHARGING_VOLT);
        break;

    case VOLT_RAW:
        return bq27220_ReadWord(BQ27220_COMMAND_RAW_VOLT);
        break;

    default:
        break;
    }
    return 0xFFFF;
}

uint16_t bq27220_get_current(bq27220_current_mode_t mode)
{
    switch (mode)
    {
    case CURRENT_RAW:
        return bq27220_ReadWord(BQ27220_COMMAND_RAW_CURR);
        break;

    case CURRENT_INSTANT:
        return bq27220_ReadWord(BQ27220_COMMAND_CURR);
        break;

    case CURRENT_STANDBY:
        return bq27220_ReadWord(BQ27220_COMMAND_STANDBY_CURR);
        break;

    case CURRENT_CHARGING:
        return bq27220_ReadWord(BQ27220_COMMAND_CHARGING_CURR);
        break;

    case CURRENT_AVERAGE:
        return bq27220_ReadWord(BQ27220_COMMAND_AVG_CURR);
        break;

    default:
        break;
    }
    return 0xFFFF;
}

uint16_t bq27220_get_battery_state()
{
    return bq27220_ReadWord(BQ27220_COMMAND_BAT_STA);
}

bool bq27220_get_is_charging()
{
    uint16_t ret = bq27220_ReadWord(BQ27220_COMMAND_BAT_STA);
    bat_st.full = ret;
    return !bat_st.st.DSG;
}

float bq27220_get_temperature()
{
    return ((float)(bq27220_ReadWord(BQ27220_COMMAND_TEMP) / 10) - 273.15);
}

uint16_t bq27220_get_remaining_capacity()
{
    return bq27220_ReadWord(BQ27220_COMMAND_REMAIN_CAPACITY);
}

uint16_t bq27220_get_full_charge_capacity()
{
    return bq27220_ReadWord(BQ27220_COMMAND_FCHG_CAPATICY);
}

uint16_t bq27220_get_state_of_charge()
{
    return bq27220_ReadWord(BQ27220_COMMAND_STATE_CHARGE);
}

uint16_t bq27220_get_device_id(void)
{
    return bq27220_ReadControlWord(0x01);
}
