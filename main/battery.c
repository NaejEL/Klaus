#include "battery.h"

void battery_init(i2c_port_t i2c_port)
{
    bq25896_init(i2c_port);
    bq27220_init(i2c_port);
}
void battery_task(void *pvParameter)
{
    while (1)
    {
        setBatteryValue((int)bq27220_get_state_of_charge());
        vTaskDelay(BATTERY_BAR_REFRESH_RATE / portTICK_PERIOD_MS);
    }
}