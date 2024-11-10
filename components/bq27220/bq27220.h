/*
Based on:

- https://github.com/JMare/esp-idf-bq27441
- https://github.com/Xinyuan-LilyGO/T-Embed-CC1101/blob/master/examples/factory_test/peripheral/bq27220.h

*/

#pragma once

// Common ESP-IDF Helpers
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

// I2C
#include <driver/i2c.h>

// Commands
#define bq27220_ADDR 0x55
#include "bq27220_registers.h"

typedef enum
{
    CURRENT_RAW,
    CURRENT_INSTANT,
    CURRENT_STANDBY,
    CURRENT_CHARGING,
    CURRENT_AVERAGE
} bq27220_current_mode_t;

typedef enum
{
    VOLT,
    VOLT_CHARGING,
    VOLT_RAW
} bq27220_voltage_mode_t;

union battery_state
{
    struct __st
    {
        uint16_t DSG : 1;
        uint16_t SYSDWN : 1;
        uint16_t TDA : 1;
        uint16_t BATTPRES : 1;
        uint16_t AUTH_GD : 1;
        uint16_t OCVGD : 1;
        uint16_t TCA : 1;
        uint16_t RSVD : 1;
        uint16_t CHGING : 1;
        uint16_t FC : 1;
        uint16_t OTD : 1;
        uint16_t OTC : 1;
        uint16_t SLEEP : 1;
        uint16_t OCVFALL : 1;
        uint16_t OCVCOMP : 1;
        uint16_t FD : 1;
    } st;
    uint16_t full;
};

esp_err_t bq27220_init(i2c_port_t i2c_port, SemaphoreHandle_t _i2c_lock);

uint16_t bq27220_get_device_id(void);
uint16_t bq27220_get_battery_state(void);
float bq27220_get_temperature(void);
bool bq27220_get_is_charging(void);
uint16_t bq27220_get_remaining_capacity(void);
uint16_t bq27220_get_full_charge_capacity(void);
uint16_t bq27220_get_state_of_charge(void);
uint16_t bq27220_get_voltage(bq27220_voltage_mode_t mode);
uint16_t bq27220_get_current(bq27220_current_mode_t mode);