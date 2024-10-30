#pragma once

#include "userinputs.h"

// LVGL
#include "esp_lvgl_port.h"

typedef struct view_handler_t view_handler_t;

typedef void (*init_view)();
typedef void (*draw_view)(view_handler_t* calling_view);
typedef void (*clear_view)();

//init_view init_view;                // xTaskCreate
struct view_handler_t
{
    lv_obj_t *obj_view; // lvgl view object
    userinputs_callback input_callback; // user inputs handler
    draw_view draw_view;                // vTaskResume
    clear_view clear_view;              // lv_obj_clean, vTaskSuspend
};

#define MAIN_SCREEN_HEIGHT (150)
#define MAIN_SCREEN_WIDTH (320)

#define BACKGROUND_COLOR (0x353D4D)
#define BACKGROUND_GRAD_COLOR (0x0AB2A0)

#define FOREGROUND_COLOR (0xE75F3F)
#define FOREGROUND_GRAD_COLOR (0xF8B234)

#define TEXT_COLOR (0XF9EDC6)