#pragma once

#include "bq27220.h"
#include "bq25896.h"

void battery_init(i2c_port_t i2c_port);
uint16_t battery_get_percent(void);
bool battery_get_charging_state(void);