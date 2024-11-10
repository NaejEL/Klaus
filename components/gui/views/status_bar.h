#pragma once

// LVGL
#include "esp_lvgl_port.h"

#include "view_commons.h"
#include "style.h"
#include "display.h"
#include "clock.h"
#include "sd.h"
#include "wifi.h"
#include "battery.h"

#define STATUS_BAR_HEIGHT (20)
#define STATUS_BAR_WIDTH (320)

#define BATTERY_BAR_REFRESH_RATE 300
#define SD_LOGO_REFRESH_RATE 3000
#define WIFI_LOGO_REFRESH_RATE 1000
#define CLOCK_REFRESH_RATE 100

// LVGL images declaration
LV_IMG_DECLARE(battery_15);
LV_IMG_DECLARE(sd_15);
LV_IMG_DECLARE(charge_15);
LV_IMG_DECLARE(wifi_nok_15);
LV_IMG_DECLARE(wifi_ok_15);

// Battery
lv_obj_t *battery_logo = NULL;
lv_obj_t *battery_bar = NULL;
//lv_obj_t *battery_label = NULL;

// SD
lv_obj_t *sd_logo = NULL;

// Wifi
lv_obj_t *wifi_bar = NULL;
lv_obj_t *wifi_logo = NULL;

// Time
lv_obj_t *time_label = NULL;

// Status bar
lv_obj_t *status_bar;

static void batteryTask(void *pvParameters)
{
    while (1)
    {
        uint16_t battery_value = battery_get_percent();
        lvgl_port_lock(0);
        lv_bar_set_value(battery_bar, battery_value, LV_ANIM_OFF);
        //lv_label_set_text_fmt(battery_label, "%d%%", battery_value);
        if (battery_get_charging_state())
        {
            lv_image_set_src(battery_logo, &charge_15);
        }
        else
        {
            lv_image_set_src(battery_logo, &battery_15);
        }
        lvgl_port_unlock();
        vTaskDelay(BATTERY_BAR_REFRESH_RATE / portTICK_PERIOD_MS);
    }
}

static void sdTask(void *pvParam)
{
    while (1)
    {
        lvgl_port_lock(0);
        if (sd_is_present())
        {
            lv_obj_remove_flag(sd_logo, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(sd_logo, LV_OBJ_FLAG_HIDDEN);
        }
        lvgl_port_unlock();
        vTaskDelay(SD_LOGO_REFRESH_RATE / portTICK_PERIOD_MS);
    }
}

static void wifiTask(void *pvParam)
{
    while (1)
    {
        lvgl_port_lock(0);
        int rssi = wifi_get_rssi();
        uint8_t quality = 2 * (rssi + 100);
        if (wifi_is_connected() && quality != 0)
        {
            lv_obj_remove_flag(wifi_bar, LV_OBJ_FLAG_HIDDEN);
            lv_image_set_src(wifi_logo, &wifi_ok_15);
            lv_bar_set_value(wifi_bar, quality, LV_ANIM_OFF);
        }
        else
        {
            lv_obj_add_flag(wifi_bar, LV_OBJ_FLAG_HIDDEN);
            lv_image_set_src(wifi_logo, &wifi_nok_15);
        }
        lvgl_port_unlock();
        vTaskDelay(WIFI_LOGO_REFRESH_RATE / portTICK_PERIOD_MS);
    }
}

static void clockTask(void *pvParam)
{
    while (1)
    {
        lvgl_port_lock(0);
        lv_label_set_text(time_label, clock_get_time());
        lvgl_port_unlock();
        vTaskDelay(CLOCK_REFRESH_RATE / portTICK_PERIOD_MS);
    }
}

static void status_bar_draw()
{
    lvgl_port_lock(0);
    status_bar = lv_obj_create(lv_screen_active());
    lv_obj_set_size(status_bar, STATUS_BAR_WIDTH, STATUS_BAR_HEIGHT);
    lv_obj_align(status_bar, LV_ALIGN_TOP_LEFT, 0, 0);

    // Set status bar style
    static lv_style_t status_bar_style;
    lv_style_init(&status_bar_style);
    lv_style_reset(&status_bar_style);
    lv_style_set_bg_color(&status_bar_style, lv_color_hex(BACKGROUND_COLOR));
    lv_style_set_text_color(&status_bar_style, lv_color_hex(TEXT_COLOR));
    lv_style_set_radius(&status_bar_style, 0);
    lv_style_set_border_width(&status_bar_style, 0);
    lv_style_set_pad_all(&status_bar_style, 0);
    lv_obj_add_style(status_bar, &status_bar_style, LV_PART_MAIN);
    // Add a gradient to background
    static lv_grad_dsc_t grad;
    grad.dir = LV_GRAD_DIR_VER;
    grad.stops_count = 2;
    grad.stops[0].color = lv_color_hex(BACKGROUND_GRAD_COLOR);
    grad.stops[0].opa = LV_OPA_COVER;
    grad.stops[1].color = lv_color_hex(BACKGROUND_COLOR);
    grad.stops[1].opa = LV_OPA_COVER;
    /*Shift the gradient to the bottom*/
    grad.stops[0].frac = 100;
    grad.stops[1].frac = 200;
    lv_style_set_bg_grad(&status_bar_style, &grad);

    battery_logo = lv_image_create(status_bar);
    lv_image_set_src(battery_logo, &battery_15);
    lv_obj_align(battery_logo, LV_ALIGN_TOP_LEFT, 0, 2);

    // SD Logo
    sd_logo = lv_image_create(status_bar);
    lv_image_set_src(sd_logo, &sd_15);
    lv_obj_align(sd_logo, LV_ALIGN_TOP_LEFT, 305, 2);

    // Wifi Logo
    wifi_logo = lv_image_create(status_bar);
    lv_image_set_src(wifi_logo, &wifi_nok_15);
    lv_obj_align(wifi_logo, LV_ALIGN_TOP_LEFT, 55, 2);

    // Battery Label
    //battery_label = lv_label_create(status_bar);
    //lv_obj_align(battery_label, LV_ALIGN_TOP_LEFT, 54, 2);
    //lv_label_set_text_fmt(battery_label, "%d%%", 0);

    // Time Label
    time_label = lv_label_create(status_bar);
    lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 205, 2);
    lv_label_set_text(time_label, "");

    battery_bar = lv_bar_create(status_bar);
    lv_obj_remove_style_all(battery_bar);
    lv_obj_add_style(battery_bar, get_bar_style_background(), 0);
    lv_obj_add_style(battery_bar, get_bar_style_indic(), LV_PART_INDICATOR);
    lv_obj_set_size(battery_bar, 35, 10);
    lv_obj_align(battery_bar, LV_ALIGN_TOP_LEFT, 15, 5);
    lv_bar_set_value(battery_bar, 0, LV_ANIM_OFF);

    // Wifi Bar
    wifi_bar = lv_bar_create(status_bar);
    lv_obj_remove_style_all(wifi_bar);
    lv_obj_add_style(wifi_bar, get_bar_style_background(), 0);
    lv_obj_add_style(wifi_bar, get_bar_style_indic(), LV_PART_INDICATOR);
    lv_obj_set_size(wifi_bar, 30, 10);
    lv_obj_align(wifi_bar, LV_ALIGN_TOP_LEFT, 77, 5);
    lv_bar_set_value(wifi_bar, 0, LV_ANIM_OFF);

    lvgl_port_unlock();
}

void status_bar_init()
{
    status_bar_draw();
    xTaskCreate(batteryTask, "batteryTask", 4096, NULL, 5, NULL);
    xTaskCreate(sdTask, "sdTask", 4096, NULL, 5, NULL);
    xTaskCreate(wifiTask, "wifiTask", 4096, NULL, 5, NULL);
    xTaskCreate(clockTask, "clockTask", 4096, NULL, 5, NULL);
}