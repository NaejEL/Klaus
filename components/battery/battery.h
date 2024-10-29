#pragma once

// I2C
#include <driver/i2c.h>

void battery_init(i2c_port_t i2c_port);
uint16_t battery_get_percent(void);
bool battery_get_charging_state(void);