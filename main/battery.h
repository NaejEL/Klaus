#pragma once

#include "bq27220.h"
#include "bq25896.h"
#include "display.h"

void battery_init(i2c_port_t i2c_port);
void battery_task(void *pvParameter);