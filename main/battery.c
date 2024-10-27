#include "battery.h"

void battery_init(i2c_port_t i2c_port)
{
    bq25896_init(i2c_port);
    bq27220_init(i2c_port);
}

uint16_t battery_get_percent(void)
{
    return bq27220_get_state_of_charge();
}

bool battery_get_charging_state(void){
    return bq27220_get_is_charging();
}