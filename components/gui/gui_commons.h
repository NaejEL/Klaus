#pragma once

// LVGL
#include "esp_lvgl_port.h"

lv_obj_t *common_screen;
lv_obj_t *main_screen;

// Splash screen
lv_obj_t *splash_screen;

#define BACKGROUND_COLOR (0x353D4D)
#define BACKGROUND_GRAD_COLOR (0x0AB2A0)

#define FOREGROUND_COLOR (0xE75F3F)
#define FOREGROUND_GRAD_COLOR (0xF8B234)

#define TEXT_COLOR (0XF9EDC6)