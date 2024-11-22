#include "battery.h"

#include "bq25896.h"
#include "bq27220.h"

void battery_init(i2c_port_t i2c_port, SemaphoreHandle_t i2c_lock) {
  bq25896_init(i2c_port, i2c_lock);
  bq27220_init(i2c_port, i2c_lock);
}

uint16_t battery_get_percent(void) { return bq27220_get_state_of_charge(); }

bool battery_get_charging_state(void) { return bq27220_get_is_charging(); }