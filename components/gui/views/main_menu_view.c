#include "main_menu_view.h"
#include "wifi_menu_view.h"
#include "splash_view.h"

static view_handler_t *calling_view;

static lv_obj_t *main_menu_view;
static view_handler_t main_menu_view_handler;

typedef enum
{
    MAIN_MENU_WIFI,
    MAIN_MENU_RFID,

    MAIN_MENU_SIZE
} main_menu_items_t;

static main_menu_items_t current_item;

static lv_obj_t *current_item_label;

// Menu images
LV_IMG_DECLARE(wifi_107x85);
LV_IMG_DECLARE(rfid_85);
static lv_obj_t *main_menu_image;
static const void *image_list[MAIN_MENU_SIZE] = {
    &wifi_107x85, &rfid_85};

static const char *main_menu_texts[MAIN_MENU_SIZE] = {
    "WIFI", "RFID"};

static void main_menu_input_handler(user_actions_t user_action)
{
    switch (user_action)
    {
    case KEY_CLICK_SHORT:
        splash_view_get_handler()->draw_view(main_menu_view_get_handler());
        break;
    case WHEEL_UP:
        current_item++;
        if (current_item >= MAIN_MENU_SIZE)
        {
            current_item = 0;
        }
        lvgl_port_lock(0);
        lv_label_set_text(current_item_label, main_menu_texts[current_item]);
        lv_image_set_src(main_menu_image, image_list[current_item]);
        lvgl_port_unlock();
        break;
    case WHEEL_CLICK_SHORT:
        if (current_item == MAIN_MENU_WIFI)
        {
            wifi_menu_view_get_handler()->draw_view(get_current_view_handler());
        }
    default:
        break;
    }
}

static void main_menu_view_clear()
{
    lvgl_port_lock(0);
    lv_obj_clean(main_menu_view);
    lvgl_port_unlock();
}

static void main_menu_view_draw(view_handler_t *_calling_view)
{
    calling_view = _calling_view;
    if (calling_view != main_menu_view_get_handler())
    {
        calling_view->clear_view();
    }
    set_current_view_handler(main_menu_view_get_handler());
    lvgl_port_lock(0);
    main_menu_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(main_menu_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(main_menu_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(main_menu_view, get_background_style(), LV_PART_MAIN);

    main_menu_image = lv_image_create(main_menu_view);
    lv_image_set_src(main_menu_image, image_list[current_item]);
    lv_obj_align(main_menu_image, LV_ALIGN_CENTER, 0, -15);

    current_item_label = lv_label_create(main_menu_view);
    lv_label_set_text(current_item_label, main_menu_texts[current_item]);
    lv_obj_align(current_item_label, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_style(current_item_label, get_bigfont_style(), LV_PART_MAIN);

    lvgl_port_unlock();
}

void main_menu_view_init(void)
{
    main_menu_view_handler.obj_view = main_menu_view;
    main_menu_view_handler.input_callback = main_menu_input_handler;
    main_menu_view_handler.draw_view = main_menu_view_draw;
    main_menu_view_handler.clear_view = main_menu_view_clear;
    current_item = MAIN_MENU_WIFI;
    wifi_menu_view_init();
}

view_handler_t *main_menu_view_get_handler(void)
{
    return &main_menu_view_handler;
}
