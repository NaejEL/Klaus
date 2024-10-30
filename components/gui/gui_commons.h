#pragma once

// LVGL
#include "esp_lvgl_port.h"

/*
typedef struct {
view_t view;    //
userinputs_callback();              // user inputs handler
init_view();                        // xTaskCreate
draw_view(view_t calling view);     // vTaskResume
clear_view();                       // lv_obj_clean, vTaskSuspend
} view_handler_t;
*/

typedef enum
{
    SPLASH_VIEW,
    MAIN_MENU_VIEW
} view_t;

// Screens
lv_obj_t *common_screen;
lv_obj_t *main_screen;

// Navigation handler
lv_obj_t *current_view = NULL;
view_t active_view;

// Views
lv_obj_t *splash_view;
lv_obj_t *main_menu_view;

#define MAIN_SCREEN_HEIGHT (150)
#define MAIN_SCREEN_WIDTH (320)

#define BACKGROUND_COLOR (0x353D4D)
#define BACKGROUND_GRAD_COLOR (0x0AB2A0)

#define FOREGROUND_COLOR (0xE75F3F)
#define FOREGROUND_GRAD_COLOR (0xF8B234)

#define TEXT_COLOR (0XF9EDC6)