#pragma once

#include "esp_lvgl_port.h"
#include "style.h"
#include "view_commons.h"

void rfid_menu_view_init(void);

view_handler_t *rfid_menu_view_get_handler(void);
