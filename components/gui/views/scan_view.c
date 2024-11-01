#include "scan_view.h"
#include "wifi.h"
#include <string.h>

static view_handler_t *calling_view;

static lv_obj_t *scan_view;
static view_handler_t scan_view_handler;
static bool view_active = false;
lv_obj_t *spinner;

static EventGroupHandle_t s_wifi_event_group;
static uint16_t scan_count;

static void scan_view_draw_ap_list(void);

static void scan_done_event(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    if (!view_active)
    {
        return;
    }
    userinputs_set_ignore(false);
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        scan_view_draw_ap_list();
    }
}

static void scan_input_handler(user_actions_t user_action)
{
    if (user_action == KEY_CLICK_SHORT)
    {
        calling_view->draw_view(scan_view_get_handler());
    }
    else if (user_action == WHEEL_UP)
    {
        lvgl_port_lock(0);
        lv_obj_scroll_by(scan_view, 0, -20, LV_ANIM_OFF);
        lvgl_port_unlock();
    }
    else if (user_action == WHEEL_DOWN)
    {
        lvgl_port_lock(0);
        lv_obj_scroll_by(scan_view, 0, 20, LV_ANIM_OFF);
        lvgl_port_unlock();
    }
}

static void scan_view_clear()
{
    view_active = false;
    lvgl_port_lock(0);
    lv_obj_clean(scan_view);
    lvgl_port_unlock();
}

static void scan_view_draw_ap_list()
{
    lvgl_port_lock(0);
    lv_obj_delete(spinner);
    lvgl_port_unlock();
    esp_wifi_scan_get_ap_num(&scan_count);
    wifi_ap_record_t ap_records[scan_count];
    esp_wifi_scan_get_ap_records(&scan_count, ap_records);
    uint16_t y = 5;
    void *wifi_labels[scan_count];
    for (int i = 0; i < scan_count; i++)
    {
        wifi_labels[i] = lv_label_create(scan_view);
        lv_obj_align((lv_obj_t *)wifi_labels[i], LV_ALIGN_TOP_LEFT, 5, y);
        lv_label_set_text_fmt((lv_obj_t *)wifi_labels[i], "%s, RSSI:%d, Channel:%d", ap_records[i].ssid, ap_records[i].rssi, ap_records[i].primary);
        y += 20;
    }
}

static void scan_view_draw(view_handler_t *_calling_view)
{
    calling_view = _calling_view;
    if (calling_view != scan_view_get_handler())
    {
        calling_view->clear_view();
    }
    set_current_view_handler(scan_view_get_handler());
    lvgl_port_lock(0);
    scan_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(scan_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(scan_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(scan_view, get_background_style(), LV_PART_MAIN);

    view_active = true;
    spinner = lv_spinner_create(scan_view);
    lv_obj_set_size(spinner, 85, 85);
    lv_obj_center(spinner);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(TEXT_COLOR), 0);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(DANGER_COLOR), LV_PART_INDICATOR);
    lv_spinner_set_anim_params(spinner, 1000, 200);
    lvgl_port_unlock();

    userinputs_set_ignore(true);
    scan_count = 0;
    wifi_launch_scan();
}

void scan_view_init(void)
{
    scan_view_handler.obj_view = scan_view;
    scan_view_handler.input_callback = scan_input_handler;
    scan_view_handler.draw_view = scan_view_draw;
    scan_view_handler.clear_view = scan_view_clear;
    s_wifi_event_group = xEventGroupCreate();
    esp_event_loop_create_default();
    esp_event_handler_instance_t scan_done;
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &scan_done_event,
                                        NULL,
                                        &scan_done);
}

view_handler_t *scan_view_get_handler(void)
{
    return &scan_view_handler;
}
