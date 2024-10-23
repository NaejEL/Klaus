#pragma once
#include "display.h"
#include "iot_button.h"

#define KEY_BTN (GPIO_NUM_6)
#define KEY_BTN_LONG_CLICK 2000
#define KEY_BTN_SHORT_CLICK 50

void keybtn_init(void);