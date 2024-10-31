#include "test_view.h"

static view_handler_t* calling_view;

static lv_obj_t *test_view;
static view_handler_t test_view_handler;

static lv_obj_t *test_label;

static void test_input_handler(user_actions_t user_action)
{
}

static void test_view_clear()
{
    lvgl_port_lock(0);
    lv_obj_clean(test_view);
    lvgl_port_unlock();
}

static void test_view_draw(view_handler_t* _calling_view)
{
    calling_view = _calling_view;
    lvgl_port_lock(0);
    if(calling_view!=test_view_get_handler()){
      calling_view->clear_view();  
    }

    test_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(test_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(test_view, LV_ALIGN_TOP_LEFT, 0, 20);

    test_label = lv_label_create(test_view);
    lv_obj_align(test_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(test_label, "TEST");
    lvgl_port_unlock();
}

void test_view_init(void)
{
    test_view_handler.obj_view = test_view;
    test_view_handler.input_callback = test_input_handler;
    test_view_handler.draw_view = test_view_draw;
    test_view_handler.clear_view = test_view_clear;
}

view_handler_t* test_view_get_handler(void){
    return &test_view_handler;
}
