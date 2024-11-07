#include "scan_view.h"

#include <string.h>

#include "wifi.h"
#include "wifi_attacks.h"

static view_handler_t *calling_view;

typedef enum
{
    NOT_IN_VIEW,
    SCANNING_VIEW,
    APLIST_VIEW,
    APINFOS_VIEW,
    DEAUTH_VIEW
} scan_views_t;

scan_views_t current_view = NOT_IN_VIEW;

static lv_obj_t *scan_view;
static view_handler_t scan_view_handler;
static uint8_t current_highlight_item = 0;
lv_obj_t *spinner = NULL;

static EventGroupHandle_t s_wifi_event_group;

static void scan_view_draw_ap_list(void);
static void scan_draw_deauth(uint8_t ap_index);
static void scan_draw_ap_infos(uint8_t ap_index);
static void scan_done_event(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    if (current_view == NOT_IN_VIEW)
    {
        return;
    }
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE)
    {
        userinputs_set_ignore(false);
        current_highlight_item = 0;
        scan_view_draw_ap_list();
    }
}

static void scan_input_handler(user_actions_t user_action)
{
    if (user_action == KEY_CLICK_SHORT)
    {
        if (current_view == DEAUTH_VIEW)
        {
            wifi_attack_dos_stop();
            scan_view_draw_ap_list();
        }
        else if (current_view == APINFOS_VIEW)
        {
            scan_view_draw_ap_list();
        }
        else
        {
            calling_view->draw_view(scan_view_get_handler());
        }
    }
    else if (user_action == WHEEL_UP && current_view == APLIST_VIEW)
    {
        current_highlight_item++;
        if (current_highlight_item >= wifi_get_all_ap_records()->count)
        {
            current_highlight_item = 0;
        }
        scan_view_draw_ap_list();
        if (current_highlight_item > 7)
        {
            lvgl_port_lock(0);
            lv_obj_scroll_to_y(scan_view, -30, LV_ANIM_OFF);
            lvgl_port_unlock();
        }
    }
    else if (user_action == WHEEL_DOWN && current_view == APLIST_VIEW)
    {
        if (current_highlight_item > 0)
        {
            current_highlight_item--;
        }
        else
        {
            current_highlight_item = wifi_get_all_ap_records()->count - 1;
        }

        scan_view_draw_ap_list();
        if (lv_obj_get_scroll_top(scan_view) < 0)
        {
            lvgl_port_lock(0);
            lv_obj_scroll_to_y(scan_view, 30, LV_ANIM_OFF);
            lvgl_port_unlock();
        }
    }
    else if (user_action == WHEEL_CLICK_SHORT)
    {
        if (current_view == APLIST_VIEW)
        {
            scan_draw_ap_infos(current_highlight_item);
        }
        else if (current_view == APINFOS_VIEW)
        {
            wifi_attack_dos_start(wifi_get_one_ap_record(current_highlight_item));
            scan_draw_deauth(current_highlight_item);
        }
    }
}

static void scan_draw_ap_infos(uint8_t ap_index)
{
    current_view = APINFOS_VIEW;
    lvgl_port_lock(0);
    lv_obj_clean(scan_view);
    scan_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(scan_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(scan_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(scan_view, get_background_style(), LV_PART_MAIN);

    lv_obj_t *ssid_lbl = lv_label_create(scan_view);
    lv_obj_align(ssid_lbl, LV_ALIGN_CENTER, 0, -60);
    lv_label_set_text_fmt(ssid_lbl, "%s", wifi_get_one_ap_record(ap_index)->ssid);
    lv_obj_add_style(ssid_lbl, get_bigfont_style(), LV_PART_MAIN);

    lv_obj_t *bssid_lbl = lv_label_create(scan_view);
    lv_obj_align(bssid_lbl, LV_ALIGN_TOP_LEFT, 5, 30);
    lv_label_set_text_fmt(bssid_lbl,
                          "BSSID: %02x:%02x:%02x:%02x:%02x:%02x Channel:%d RSSI:%d",
                          wifi_get_one_ap_record(ap_index)->bssid[0],
                          wifi_get_one_ap_record(ap_index)->bssid[1],
                          wifi_get_one_ap_record(ap_index)->bssid[2],
                          wifi_get_one_ap_record(ap_index)->bssid[3],
                          wifi_get_one_ap_record(ap_index)->bssid[4],
                          wifi_get_one_ap_record(ap_index)->bssid[5],
                          wifi_get_one_ap_record(ap_index)->primary,
                          wifi_get_one_ap_record(ap_index)->rssi);

    lv_obj_t *auth_phy_lbl = lv_label_create(scan_view);
    lv_obj_align(auth_phy_lbl, LV_ALIGN_TOP_LEFT, 5, 50);
    char phy_str[MAX_PHY_SIZE];
    wifi_get_phy_from_record(ap_index, phy_str);
    lv_label_set_text_fmt(
        auth_phy_lbl, "Auth Mode: %s   PHY: %s",
        wifi_get_auth_string(wifi_get_one_ap_record(ap_index)->authmode), phy_str);

    lv_obj_t *cipher_lbl = lv_label_create(scan_view);
    lv_obj_align(cipher_lbl, LV_ALIGN_TOP_LEFT, 5, 70);
    lv_label_set_text_fmt(cipher_lbl, "Pairwise Cipher: %s",
                          wifi_get_cipher_string(
                              wifi_get_one_ap_record(ap_index)->pairwise_cipher));

    lv_obj_t *cipher_group_lbl = lv_label_create(scan_view);
    lv_obj_align(cipher_group_lbl, LV_ALIGN_TOP_LEFT, 5, 90);
    lv_label_set_text_fmt(
        cipher_group_lbl, "Group Cipher: %s",
        wifi_get_cipher_string(wifi_get_one_ap_record(ap_index)->group_cipher));

    lv_obj_t *short_click_lbl = lv_label_create(scan_view);
    lv_obj_add_style(short_click_lbl, get_danger_style(), LV_PART_MAIN);
    lv_obj_align(short_click_lbl, LV_ALIGN_CENTER, 0, 60);
    lv_label_set_text(short_click_lbl, "Click wheel to deauth");

    lvgl_port_unlock();
}

static void scan_view_clear()
{
    current_view = NOT_IN_VIEW;
    lvgl_port_lock(0);
    lv_obj_clean(scan_view);
    lvgl_port_unlock();
}

static void scan_draw_deauth(uint8_t ap_index)
{
        current_view = DEAUTH_VIEW;
    lvgl_port_lock(0);
    lv_obj_clean(scan_view);
    scan_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(scan_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(scan_view, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_add_style(scan_view, get_background_style(), LV_PART_MAIN);

    spinner = lv_spinner_create(scan_view);
    lv_obj_set_size(spinner, 50, 50);
    lv_obj_center(spinner);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(TEXT_COLOR), 0);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(DANGER_COLOR),
                               LV_PART_INDICATOR);
    lv_spinner_set_anim_params(spinner, 500, 200);

    lv_obj_t *deauth_lbl = lv_label_create(scan_view);
    lv_obj_align(deauth_lbl, LV_ALIGN_CENTER, 0, -60);
    lv_label_set_text_fmt(deauth_lbl,
                          "Deauth attack on %s",
                          wifi_get_one_ap_record(ap_index)->ssid);

    lv_obj_t *return_click_lbl = lv_label_create(scan_view);
    lv_obj_align(return_click_lbl, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_style(return_click_lbl, get_danger_style(), LV_PART_MAIN);
    lv_label_set_text(return_click_lbl, "Click return to stop attack");

    lvgl_port_unlock();
}

static void scan_view_draw_ap_list()
{
    current_view = APLIST_VIEW;
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

    const wifi_ap_records_t *records = wifi_get_all_ap_records();
    uint16_t y = 5;
    void *wifi_labels[wifi_get_all_ap_records()->count];
    for (size_t i = 0; i < wifi_get_all_ap_records()->count; i++)
    {
        wifi_labels[i] = lv_label_create(scan_view);
        lv_obj_align((lv_obj_t *)wifi_labels[i], LV_ALIGN_TOP_LEFT, 5, y);
        if (i == current_highlight_item)
        {
            lv_obj_add_style((lv_obj_t *)wifi_labels[i], get_highlight_style(),
                             LV_PART_MAIN);
            lv_label_set_long_mode((lv_obj_t *)wifi_labels[i],
                                   LV_LABEL_LONG_SCROLL_CIRCULAR);
        }
        lv_label_set_text_fmt((lv_obj_t *)wifi_labels[i], "%s, RSSI:%d, Channel:%d",
                              records->records[i].ssid, records->records[i].rssi,
                              records->records[i].primary);
        y += 20;
    }
    lvgl_port_unlock();
}

static void scan_view_draw(view_handler_t *_calling_view)
{
    current_view = SCANNING_VIEW;
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

    spinner = lv_spinner_create(scan_view);
    lv_obj_set_size(spinner, 85, 85);
    lv_obj_center(spinner);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(TEXT_COLOR), 0);
    lv_obj_set_style_arc_color(spinner, lv_color_hex(DANGER_COLOR),
                               LV_PART_INDICATOR);
    lv_spinner_set_anim_params(spinner, 1000, 200);
    lvgl_port_unlock();
    if (!wifi_is_connecting())
    {
        userinputs_set_ignore(true);
        wifi_launch_scan();
    }
}

void scan_view_init(void)
{
    current_view = NOT_IN_VIEW;
    scan_view_handler.obj_view = scan_view;
    scan_view_handler.input_callback = scan_input_handler;
    scan_view_handler.draw_view = scan_view_draw;
    scan_view_handler.clear_view = scan_view_clear;
    s_wifi_event_group = xEventGroupCreate();
    esp_event_loop_create_default();
    esp_event_handler_instance_t scan_done;
    esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE,
                                        &scan_done_event, NULL, &scan_done);
}

view_handler_t *scan_view_get_handler(void) { return &scan_view_handler; }
