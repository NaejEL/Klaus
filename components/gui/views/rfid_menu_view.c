#include "rfid_menu_view.h"
#include "pn532.h"

static view_handler_t *calling_view;

static lv_obj_t *rfid_menu_view;
static view_handler_t rfid_menu_view_handler;

typedef enum
{
    RFID_MENU_READ,

    RFID_MENU_SIZE
} rfid_menu_items_t;

static const char *rfid_menu_texts[RFID_MENU_SIZE] = {
    "READ"};

rfid_menu_items_t current_menu_item;

typedef enum
{
    RFID_VIEW_MAIN,
    RFID_VIEW_READ,
    RFID_VIEW_TAG,

    RFID_VIEWS_SIZE
} rfid_menu_views_t;

rfid_menu_views_t current_view;

lv_obj_t *spinner;

static void rfid_menu_draw_read_view(void);
static void rfid_menu_draw_main_view(void);
static void rfid_menu_draw_tag_view(pn532_record_t *record);

static void rfid_menu_input_handler(user_actions_t user_action)
{
    if (user_action == KEY_CLICK_SHORT)
    {
        calling_view->draw_view(rfid_menu_view_get_handler());
    }
    else if (user_action == WHEEL_CLICK_SHORT && current_view == RFID_VIEW_MAIN)
    {
        rfid_menu_draw_read_view();
    }
}

static void rfid_menu_draw_tag_view(pn532_record_t *record)
{
    current_view = RFID_VIEW_TAG;
    lvgl_port_lock(0);

    lv_obj_clean(rfid_menu_view);
    rfid_menu_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(rfid_menu_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(rfid_menu_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(rfid_menu_view, get_background_style(), LV_PART_MAIN);

    lv_obj_t *tag_info_lbl = lv_label_create(rfid_menu_view);
    lv_obj_align(tag_info_lbl, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_label_set_text_fmt(tag_info_lbl,"ATQA:0x%02x SAK:0x%02x UID:%02X:%02X:%02X:%02X",
    record->ATQA,
    record->SAK,
    record->uid[0],
    record->uid[1],
    record->uid[2],
    record->uid[3]);
    lvgl_port_unlock();
}

static void rfid_menu_draw_read_view()
{
    current_view = RFID_VIEW_READ;
    lvgl_port_lock(0);

    lv_obj_clean(rfid_menu_view);
    rfid_menu_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(rfid_menu_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(rfid_menu_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(rfid_menu_view, get_background_style(), LV_PART_MAIN);

    spinner = lv_spinner_create(rfid_menu_view);
    lv_obj_set_size(spinner, 50, 50);
    lv_obj_center(spinner);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(TEXT_COLOR), 0);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(DANGER_COLOR),
                               LV_PART_INDICATOR);
    lv_spinner_set_anim_params(spinner, 500, 200);

    lv_obj_t *read_label = lv_label_create(rfid_menu_view);
    lv_obj_align(read_label, LV_ALIGN_CENTER, 0, -60);
    lv_label_set_text(read_label, "Waiting for a tag");
    lvgl_port_unlock();
    pn532_background_read_passive_targetID(PN532_MIFARE_ISO14443A, 0, &rfid_menu_draw_tag_view);
}

static void rfid_menu_draw_main_view()
{
    current_view = RFID_VIEW_MAIN;
    lvgl_port_lock(0);
    rfid_menu_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(rfid_menu_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(rfid_menu_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(rfid_menu_view, get_background_style(), LV_PART_MAIN);

    void *menu_labels[RFID_MENU_SIZE];
    uint16_t y = 5;
    for (size_t i = 0; i < RFID_MENU_SIZE; i++)
    {
        menu_labels[i] = lv_label_create(rfid_menu_view);
        lv_obj_align((lv_obj_t *)menu_labels[i], LV_ALIGN_TOP_LEFT, 5, y);
        lv_obj_add_style((lv_obj_t *)menu_labels[i], get_bigfont_style(), LV_PART_MAIN);
        if (i == current_menu_item)
        {
            lv_obj_add_style((lv_obj_t *)menu_labels[i], get_highlight_style(), LV_PART_MAIN);
        }
        lv_label_set_text((lv_obj_t *)menu_labels[i], rfid_menu_texts[i]);
    }
    lvgl_port_unlock();
}

static void rfid_menu_view_clear()
{
    lvgl_port_lock(0);
    lv_obj_clean(rfid_menu_view);
    lvgl_port_unlock();
}

static void rfid_menu_view_draw(view_handler_t *_calling_view)
{
    calling_view = _calling_view;
    if (calling_view != rfid_menu_view_get_handler())
    {
        calling_view->clear_view();
    }
    set_current_view_handler(rfid_menu_view_get_handler());
    rfid_menu_draw_main_view();
}

void rfid_menu_view_init(void)
{
    rfid_menu_view_handler.obj_view = rfid_menu_view;
    rfid_menu_view_handler.input_callback = rfid_menu_input_handler;
    rfid_menu_view_handler.draw_view = rfid_menu_view_draw;
    rfid_menu_view_handler.clear_view = rfid_menu_view_clear;
    current_menu_item = RFID_MENU_READ;
    current_view = RFID_VIEW_MAIN;
}

view_handler_t *rfid_menu_view_get_handler(void)
{
    return &rfid_menu_view_handler;
}

/*
    uint32_t pn532_ver = pn532_get_firmware_version();
    printf("PN5%2x, Ver.%d.%d\n",
           (uint8_t)((pn532_ver >> 24) & 0xFF),
           (uint8_t)((pn532_ver >> 16) & 0xFF),
           (uint8_t)((pn532_ver >> 8) & 0xFF));
*/