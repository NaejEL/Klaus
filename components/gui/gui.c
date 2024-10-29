#include "gui.h"

static const char *TAG = "GUI";

#include "display.h"
#include "clock.h"
#include "sd.h"
#include "wifi.h"
#include "userinputs.h"
#include "battery.h"

// LVGL
#include "esp_lvgl_port.h"

#define BACKGROUND_COLOR (0x9BCDEF)
#define FOREGROUND_COLOR (0xF79347)

#define BATTERY_BAR_REFRESH_RATE 300
#define SD_LOGO_REFRESH_RATE 3000
#define WIFI_LOGO_REFRESH_RATE 1000
#define CLOCK_REFRESH_RATE 100

// LVGL Display handler
static lv_display_t *lvgl_disp = NULL;

// LVGL images declaration
LV_IMG_DECLARE(klaus_dab_126x85);
LV_IMG_DECLARE(battery_15);
LV_IMG_DECLARE(sd_15);
LV_IMG_DECLARE(charge_15);
LV_IMG_DECLARE(wifi_nok_15);
LV_IMG_DECLARE(wifi_ok_15);

// Battery
lv_obj_t *battery_logo = NULL;
lv_obj_t *battery_bar = NULL;
lv_obj_t *battery_label = NULL;

// SD
lv_obj_t *sd_logo = NULL;

// Wifi
lv_obj_t *wifi_bar = NULL;
lv_obj_t *wifi_logo = NULL;

// Time
lv_obj_t *time_label = NULL;

static void batteryTask(void *pvParameters)
{
    while (1)
    {
        uint16_t battery_value = battery_get_percent();
        lvgl_port_lock(0);
        lv_bar_set_value(battery_bar, battery_value, LV_ANIM_OFF);
        lv_label_set_text_fmt(battery_label, "%d%%", battery_value);
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
        if (wifi_get_state() && quality != 0)
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

static void user_action(user_actions_t action)
{
    switch (action)
    {
    case WHEEL_UP:
        break;

    case WHEEL_DOWN:
        break;

    case WHEEL_CLICK_SHORT:
        break;

    case WHEEL_CLICK_LONG:
        break;

    case KEY_CLICK_LONG:
        display_backlight_toggle();
        break;

    case KEY_CLICK_SHORT:
        break;

    default:
        break;
    }
}

esp_err_t gui_init()
{
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 4,       /* LVGL task priority */
        .task_stack = 6144,       /* LVGL task stack size */
        .task_affinity = -1,      /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 500, /* Maximum sleep in LVGL task */
        .timer_period_ms = 15     /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    ESP_LOGD(TAG, "Attach screen to LVGL");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = display_get_io_handle(),
        .panel_handle = display_get_panel_handle(),
        .buffer_size = LCD_DRAW_BUFF_SIZE,
        .double_buffer = LCD_DRAW_BUFF_DOUBLE,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .rotation = {
            .swap_xy = LCD_SWAP_XY,
            .mirror_x = LCD_MIRROR_X,
            .mirror_y = LCD_MIRROR_Y,
        },
        .flags = {
            .buff_dma = LCD_USE_DMA,
            .buff_spiram = LCD_USE_SPIRAM,
            .sw_rotate = false,
            .swap_bytes = LCD_BIGENDIAN,
            .full_refresh = false,
            .direct_mode = false,
        }};

    lvgl_disp = lvgl_port_add_disp(&disp_cfg);
    userinputs_register_callback(&user_action);
    return ESP_OK;
}

void gui_start()
{
    lvgl_port_lock(0);
    lv_obj_t *scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, lv_color_hex(BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_text_color(scr, lv_color_hex(FOREGROUND_COLOR), LV_PART_MAIN);

    lv_obj_t *klausDab = lv_image_create(scr);
    lv_image_set_src(klausDab, &klaus_dab_126x85);
    lv_obj_align(klausDab, LV_ALIGN_CENTER, 0, 0);

    battery_logo = lv_image_create(scr);
    lv_image_set_src(battery_logo, &battery_15);
    lv_obj_align(battery_logo, LV_ALIGN_TOP_LEFT, 0, 2);

    // SD Logo
    sd_logo = lv_image_create(scr);
    lv_image_set_src(sd_logo, &sd_15);
    lv_obj_align(sd_logo, LV_ALIGN_TOP_LEFT, 305, 2);

    // Wifi Logo
    wifi_logo = lv_image_create(scr);
    lv_image_set_src(wifi_logo, &wifi_nok_15);
    lv_obj_align(wifi_logo, LV_ALIGN_TOP_LEFT, 90, 2);

    // Battery Label
    battery_label = lv_label_create(scr);
    lv_obj_align(battery_label, LV_ALIGN_TOP_LEFT, 54, 2);
    lv_label_set_text_fmt(battery_label, "%d%%", 0);

    // Time Label
    time_label = lv_label_create(scr);
    lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 205, 2);
    lv_label_set_text(time_label, "");

    // Bars Style
    static lv_style_t bar_indic;
    static lv_style_t bar_bg;

    lv_style_init(&bar_indic);
    lv_style_set_bg_opa(&bar_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&bar_indic, lv_color_hex(FOREGROUND_COLOR));
    lv_style_set_radius(&bar_indic, 6);

    lv_style_init(&bar_bg);
    lv_style_set_bg_opa(&bar_bg, LV_OPA_30);
    lv_style_set_bg_color(&bar_bg, lv_color_hex(0x000000));
    lv_style_set_radius(&bar_bg, 6);

    battery_bar = lv_bar_create(lv_screen_active());
    lv_obj_remove_style_all(battery_bar);
    lv_obj_add_style(battery_bar, &bar_bg, 0);
    lv_obj_add_style(battery_bar, &bar_indic, LV_PART_INDICATOR);
    lv_obj_set_size(battery_bar, 35, 10);
    lv_obj_align(battery_bar, LV_ALIGN_TOP_LEFT, 15, 5);
    lv_bar_set_value(battery_bar, 0, LV_ANIM_OFF);

    // Wifi Bar
    wifi_bar = lv_bar_create(scr);
    lv_obj_remove_style_all(wifi_bar);
    lv_obj_add_style(wifi_bar, &bar_bg, 0);
    lv_obj_add_style(wifi_bar, &bar_indic, LV_PART_INDICATOR);
    lv_obj_set_size(wifi_bar, 30, 10);
    lv_obj_align(wifi_bar, LV_ALIGN_TOP_LEFT, 112, 5);
    lv_bar_set_value(wifi_bar, 0, LV_ANIM_OFF);

    lvgl_port_unlock();
    xTaskCreate(batteryTask, "batteryTask", 4096, NULL, 5, NULL);
    xTaskCreate(sdTask, "sdTask", 4096, NULL, 5, NULL);
    xTaskCreate(wifiTask, "wifiTask", 4096, NULL, 5, NULL);
    xTaskCreate(clockTask, "clockTask", 4096, NULL, 5, NULL);
}
