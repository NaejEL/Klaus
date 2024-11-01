#pragma once

#include "esp_err.h"
#include <stdbool.h>
typedef enum
{
    WHEEL_UP,
    WHEEL_DOWN,
    WHEEL_CLICK_SHORT,
    WHEEL_CLICK_LONG,
    KEY_CLICK_SHORT,
    KEY_CLICK_LONG
} user_actions_t;

typedef void (*userinputs_callback)(user_actions_t user_action);

esp_err_t userinputs_init(void);

void userinputs_register_callback(userinputs_callback callback);
void userinputs_set_ignore(bool ignore);