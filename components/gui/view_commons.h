#pragma once

#include "userinputs.h"

// LVGL
#include "esp_lvgl_port.h"

typedef struct view_handler_t view_handler_t;

typedef void (*init_view)(); // xTaskCreate
typedef void (*draw_view)(view_handler_t *calling_view);
typedef void (*clear_view)();

struct view_handler_t {
  lv_obj_t *obj_view;                 // lvgl view object
  userinputs_callback input_callback; // user inputs handler
  draw_view draw_view;                // vTaskResume
  clear_view clear_view;              // lv_obj_clean, vTaskSuspend
};

view_handler_t *get_current_view_handler(void);

void set_current_view_handler(view_handler_t *view_handler);