#include "splash_view.h"

static lv_obj_t *splash_view;
static view_handler_t *calling_view;
static view_handler_t splash_view_handler;

//
// Splash Image
lv_obj_t *splash_image;

// LVGL images declaration
LV_IMG_DECLARE(klaus_112x85);
LV_IMG_DECLARE(klaus_afraid_104x85);
LV_IMG_DECLARE(klaus_bored_102x85);
LV_IMG_DECLARE(klaus_dab_126x85);
LV_IMG_DECLARE(klaus_fuck_100x85);

#define NB_IMAGES 5
static uint8_t current_image_index = 0;
static const void *image_list[NB_IMAGES] = {
    &klaus_112x85,
    &klaus_afraid_104x85,
    &klaus_bored_102x85,
    &klaus_dab_126x85,
    &klaus_fuck_100x85};

static void splash_view_clear()
{
    lvgl_port_lock(0);
    lv_obj_clean(splash_view);
    lvgl_port_unlock();
}

static void splash_input_handler(user_actions_t user_action)
{
    switch (user_action)
    {
    case WHEEL_UP:
        current_image_index++;
        if (current_image_index >= NB_IMAGES)
        {
            current_image_index = 0;
        }
        lvgl_port_lock(0);
        lv_image_set_src(splash_image, image_list[current_image_index]);
        lvgl_port_unlock();
        break;

    case WHEEL_DOWN:
        if (current_image_index > 0)
        {
            current_image_index--;
        }
        else
        {
            current_image_index = NB_IMAGES - 1;
        }
        lvgl_port_lock(0);
        lv_image_set_src(splash_image, image_list[current_image_index]);
        lvgl_port_unlock();
        break;

    default:
        break;
    }
}

static void splash_view_draw(view_handler_t *_calling_view)
{
    calling_view = _calling_view;
    if (calling_view != splash_view_get_handler())
    {
        calling_view->clear_view();
    }
    set_current_view_handler(splash_view_get_handler());
    lvgl_port_lock(0);
    splash_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(splash_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(splash_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(splash_view, get_background_style(), 0);

    splash_image = lv_image_create(splash_view);
    lv_image_set_src(splash_image, image_list[current_image_index]);
    lv_obj_align(splash_image, LV_ALIGN_CENTER, 0, 0);
    lvgl_port_unlock();
}

void splash_view_init(void)
{
    splash_view_handler.obj_view = splash_view;
    splash_view_handler.input_callback = splash_input_handler;
    splash_view_handler.draw_view = splash_view_draw;
    splash_view_handler.clear_view = splash_view_clear;
    current_image_index = 0;
}

view_handler_t *splash_view_get_handler()
{
    return &splash_view_handler;
}
