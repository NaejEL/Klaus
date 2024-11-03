#pragma once

#include "esp_lvgl_port.h"
#include "style.h"
#include "view_commons.h"
#include "esp_wifi_types.h"

void deauth_view_init(void);

view_handler_t* deauth_view_get_handler(void);