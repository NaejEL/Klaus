/*
Based on: https://github.com/lewisxhe/XPowersLib
*/

#pragma once

#include "driver/i2c.h"
#include <string.h>

#define BQ25896_ADDR (0X6B)

#define BQ25896_VOLT_STEP (100) // Power off threshold 100mV steps
#define BQ25896_VOLT_MIN (3000) // Power off min voltage 3.0V
#define BQ25896_VOLT_MAX (3700) // Power off max voltage 3.7V

#define BQ25896_CURR_STEP (50)  // Input current limit 50mA step
#define BQ25896_CURR_MAX (3250) // Input current limit Max 3.25A
#define BQ25896_CURR_MIN (100)  // Input current limit Min 100mA

#define BQ25896_CHARGE_VOLT_STEP (16) // Charge target voltage 16mV steps
#define BQ25896_CHARGE_VOLT_MAX (4608) // Charge target voltage max 4.608V
#define BQ25896_CHARGE_VOLT_MIN (3840) // Charge target voltage min 3.84V

#define BQ25896_PRECHARGE_CURR_STEP (64) // Precharge current 64mA steps
#define BQ25896_PRECHARGE_CURR_MAX (1024) // Precharge current max 1024mA
#define BQ25896_PRECHARGE_CURR_MIN (64) // Precharge current min 128mA

#define BQ25896_CHARGE_CONST_CURR_STEP (64)  // Charge constant current step 64mA
#define BQ25896_CHARGE_CONST_CURR_MAX (3008)  // Charge constant current max 3008mA, min 0mA

#define BQ25896_PWOFF_TRESHOLD (3300) // We want to power off under 3.3V
#define BQ25896_CHARGE_VOLT_DEFAULT (4208)// We want to charge at 4.208V
#define BQ25896_CHARGE_CONST_CURR_DEFAULT (832)// We want fast charge at 832mA

typedef enum
{
    NO_CHARGE,
    PRE_CHARGE,
    FAST_CHARGE,
    CHARGE_DONE,
    CHARGE_UNKNOW
} bq25896_charge_status_t;

typedef enum
{
   TIMEOUT_40SEC,
   TIMEOUT_80SEC,
   TIMEOUT_160SEC,
} bq25896_watchdog_timeout_t;

void bq25896_init(i2c_port_t _i2c_port);

void bq25896_set_input_current_limit(uint16_t milliamps);
uint16_t bq25896_get_input_current_limit(void);

void bq25896_disable_current_limit_pin(void);
void bq25896_enable_current_limit_pin(void);
bool bq25896_get_current_limit_pin_state(void);

void bq25896_set_charge_target_voltage(uint16_t millivolts);
uint16_t bq25896_get_charge_target_voltage(void);

void bq25896_set_precharge_current(uint16_t milliamps);
uint16_t bq25896_get_precharge_current(void);

void bq25896_set_charger_constant_current(uint16_t milliamps);
uint16_t bq25896_get_charger_constant_current(void);

void bq25896_enable_adc(void);
void bq25896_disable_adc(void);

void bq25896_enable_charge(void);
void bq25896_disable_charge(void);
bool bq25896_get_charge_state(void);

void bq25896_enable_watchdog(bq25896_watchdog_timeout_t timeout);
void bq25896_disable_watchdog(void);

void bq25896_set_power_off_voltage(uint16_t threshold_millivolts);
uint16_t bq25896_get_power_off_voltage(void);

uint8_t bq25896_get_devices_id(void);
bq25896_charge_status_t bq25896_get_charge_status(void);

void bq25896_reset_registers(void);