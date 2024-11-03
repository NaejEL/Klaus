#include "wifi_menu_view.h"
#include "scan_view.h"
#include "deauth_view.h"
static view_handler_t *calling_view;

static lv_obj_t *wifi_menu_view;
static view_handler_t wifi_menu_view_handler;

typedef enum
{
    WIFI_MENU_SCAN,
    WIFI_MENU_DEAUTH,

    WIFI_MENU_SIZE
} wifi_menu_items_t;

static wifi_menu_items_t current_item;
static lv_obj_t *current_item_label;
LV_IMG_DECLARE(scan_85);
LV_IMG_DECLARE(nowifi_85);

static lv_obj_t *wifi_menu_image;
static const void *image_list[WIFI_MENU_SIZE] = {
    &scan_85,
    &nowifi_85};

static const char *wifi_menu_texts[WIFI_MENU_SIZE] = {
    "SCAN",
    "DEAUTH"};

static void wifi_menu_input_handler(user_actions_t user_action)
{
    switch (user_action)
    {
    case KEY_CLICK_SHORT:
        calling_view->draw_view(wifi_menu_view_get_handler());
        break;
    case WHEEL_CLICK_SHORT:
        if (current_item == WIFI_MENU_SCAN)
        {
            scan_view_get_handler()->draw_view(calling_view);
        }
        break;
    case WHEEL_UP:
        current_item++;
        if (current_item >= WIFI_MENU_SIZE)
        {
            current_item = 0;
        }
        lvgl_port_lock(0);
        lv_label_set_text(current_item_label, wifi_menu_texts[current_item]);
        lv_image_set_src(wifi_menu_image, image_list[current_item]);
        lvgl_port_unlock();
        break;
    default:
        break;
    }
}

static void wifi_menu_view_clear()
{
    lvgl_port_lock(0);
    lv_obj_clean(wifi_menu_view);
    lvgl_port_unlock();
}

static void wifi_menu_view_draw(view_handler_t *_calling_view)
{
    calling_view = _calling_view;
    if (calling_view != wifi_menu_view_get_handler())
    {
        calling_view->clear_view();
    }
    set_current_view_handler(wifi_menu_view_get_handler());
    lvgl_port_lock(0);
    wifi_menu_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(wifi_menu_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(wifi_menu_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(wifi_menu_view, get_background_style(), LV_PART_MAIN);

    wifi_menu_image = lv_image_create(wifi_menu_view);
    lv_image_set_src(wifi_menu_image, image_list[current_item]);
    lv_obj_align(wifi_menu_image, LV_ALIGN_CENTER, 0, -15);

    current_item_label = lv_label_create(wifi_menu_view);
    lv_label_set_text(current_item_label, wifi_menu_texts[current_item]);
    lv_obj_align(current_item_label, LV_ALIGN_CENTER, 0, 50);
    lv_obj_add_style(current_item_label, get_bigfont_style(), LV_PART_MAIN);

    lvgl_port_unlock();
}

void wifi_menu_view_init(void)
{
    wifi_menu_view_handler.obj_view = wifi_menu_view;
    wifi_menu_view_handler.input_callback = wifi_menu_input_handler;
    wifi_menu_view_handler.draw_view = wifi_menu_view_draw;
    wifi_menu_view_handler.clear_view = wifi_menu_view_clear;
    scan_view_init();
}

view_handler_t *wifi_menu_view_get_handler(void)
{
    return &wifi_menu_view_handler;
}