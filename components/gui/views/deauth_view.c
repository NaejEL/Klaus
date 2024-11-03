#include "deauth_view.h"
#include "wifi.h"
#include <string.h>
#include "esp_timer.h"
#include "wsl_bypasser.h"

static view_handler_t *calling_view;

static lv_obj_t *deauth_view;
static view_handler_t deauth_view_handler;

static void deauth_input_handler(user_actions_t user_action)
{
    calling_view->draw_view(deauth_view_get_handler());
}

static void deauth_view_clear()
{
    lvgl_port_lock(0);
    lv_obj_clean(deauth_view);
    lvgl_port_unlock();
}

static void deauth_view_draw(view_handler_t *_calling_view)
{
    calling_view = _calling_view;
    if (calling_view != deauth_view_get_handler())
    {
        calling_view->clear_view();
    }
    set_current_view_handler(deauth_view_get_handler());
    lvgl_port_lock(0);
    deauth_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(deauth_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(deauth_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(deauth_view, get_background_style(), LV_PART_MAIN);
    lvgl_port_unlock();
}

void deauth_view_init(void)
{
    deauth_view_handler.obj_view = deauth_view;
    deauth_view_handler.input_callback = deauth_input_handler;
    deauth_view_handler.draw_view = deauth_view_draw;
    deauth_view_handler.clear_view = deauth_view_clear;
}

view_handler_t *deauth_view_get_handler(void)
{
    return &deauth_view_handler;
}
