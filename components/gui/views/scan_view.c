#include "scan_view.h"
#include "wifi.h"
#include <string.h>

static view_handler_t *calling_view;

static lv_obj_t *scan_view;
static view_handler_t scan_view_handler;
static bool view_active = false;
static uint8_t current_highlight_item = 0;
lv_obj_t *spinner = NULL;

static EventGroupHandle_t s_wifi_event_group;
static uint16_t scan_count;
wifi_ap_record_t *ap_records;
static bool info_print = false;
static void scan_view_draw_ap_list(void);
static void scan_draw_ap_infos(uint8_t ap_index);
static void scan_done_event(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    if (!view_active)
    {
        return;
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        userinputs_set_ignore(false);
        current_highlight_item = 0;
        esp_wifi_scan_get_ap_num(&scan_count);
        ap_records = malloc(scan_count * sizeof(wifi_ap_record_t));
        wifi_ap_record_t ap_record_list[scan_count];
        esp_wifi_scan_get_ap_records(&scan_count, ap_record_list);
        for (size_t i = 0; i < scan_count; i++)
        {
            memcpy(&ap_records[i], &ap_record_list[i], sizeof(wifi_ap_record_t));
        }
        scan_view_draw_ap_list();
    }
}

static void scan_input_handler(user_actions_t user_action)
{
    if (user_action == KEY_CLICK_SHORT)
    {
        if (info_print)
        {
            scan_view_draw_ap_list();
        }
        else
        {
            calling_view->draw_view(scan_view_get_handler());
        }
    }
    else if (user_action == WHEEL_UP && !info_print)
    {
        current_highlight_item++;
        if (current_highlight_item >= scan_count)
        {
            current_highlight_item = 0;
        }
        scan_view_draw_ap_list();
    }
    else if (user_action == WHEEL_DOWN && !info_print)
    {
        if (current_highlight_item > 0)
        {
            current_highlight_item--;
        }
        else
        {
            current_highlight_item = scan_count - 1;
        }
        scan_view_draw_ap_list();
    }
    else if (user_action == WHEEL_CLICK_SHORT)
    {
        scan_draw_ap_infos(current_highlight_item);
    }
}

static void scan_draw_ap_infos(uint8_t ap_index)
{
    info_print = true;
    lvgl_port_lock(0);
    lv_obj_clean(scan_view);
    scan_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(scan_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(scan_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(scan_view, get_background_style(), LV_PART_MAIN);

    lv_obj_t *ssid_lbl = lv_label_create(scan_view);
    lv_obj_align(ssid_lbl, LV_ALIGN_CENTER, 0, -60);
    lv_label_set_text_fmt(ssid_lbl, "%s", ap_records[ap_index].ssid);
    lv_obj_add_style(ssid_lbl, get_bigfont_style(), LV_PART_MAIN);

    lv_obj_t *bssid_lbl = lv_label_create(scan_view);
    lv_obj_align(bssid_lbl, LV_ALIGN_TOP_LEFT, 5, 30);
    lv_label_set_text_fmt(bssid_lbl, "BSSID: %02x:%02x:%02x:%02x   Channel:%d   RSSI:%d",
                          ap_records[ap_index].bssid[0],
                          ap_records[ap_index].bssid[1],
                          ap_records[ap_index].bssid[2],
                          ap_records[ap_index].bssid[3],
                          ap_records[ap_index].primary,
                          ap_records[ap_index].rssi);

    lv_obj_t *auth_lbl = lv_label_create(scan_view);
    lv_obj_align(auth_lbl, LV_ALIGN_TOP_LEFT, 5, 50);
    lv_label_set_text_fmt(auth_lbl, "Auth Mode: %s", wifi_get_auth_string(ap_records[ap_index].authmode));

    lv_obj_t *cipher_lbl = lv_label_create(scan_view);
    lv_obj_align(cipher_lbl, LV_ALIGN_TOP_LEFT, 5, 70);
    lv_label_set_text_fmt(cipher_lbl, "Pairwise Cipher: %s", wifi_get_cipher_string(ap_records[ap_index].pairwise_cipher));

    lv_obj_t *cipher_group_lbl = lv_label_create(scan_view);
    lv_obj_align(cipher_group_lbl, LV_ALIGN_TOP_LEFT, 5, 90);
    lv_label_set_text_fmt(cipher_group_lbl, "Group Cipher: %s", wifi_get_cipher_string(ap_records[ap_index].group_cipher));

    lvgl_port_unlock();
}

static void scan_view_clear()
{
    view_active = false;
    free(ap_records);
    lvgl_port_lock(0);
    lv_obj_clean(scan_view);
    lvgl_port_unlock();
}

static void scan_view_draw_ap_list()
{
    info_print = false;
    lvgl_port_lock(0);
    if (spinner != NULL)
    {
        lv_obj_delete(spinner);
        spinner = NULL;
    }
    lv_obj_clean(scan_view);
    scan_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(scan_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(scan_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(scan_view, get_background_style(), LV_PART_MAIN);
    uint16_t y = 5;
    void *wifi_labels[scan_count];
    for (size_t i = 0; i < scan_count; i++)
    {
        wifi_labels[i] = lv_label_create(scan_view);
        lv_obj_align((lv_obj_t *)wifi_labels[i], LV_ALIGN_TOP_LEFT, 5, y);
        if (i == current_highlight_item)
        {
            lv_obj_add_style((lv_obj_t *)wifi_labels[i], get_highlight_style(), LV_PART_MAIN);
        }
        lv_label_set_text_fmt((lv_obj_t *)wifi_labels[i], "%s, RSSI:%d, Channel:%d", ap_records[i].ssid, ap_records[i].rssi, ap_records[i].primary);
        y += 20;
    }
    lvgl_port_unlock();
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
