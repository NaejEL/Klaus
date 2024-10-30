#include "gui.h"

static const char *TAG = "GUI";

#include "userinputs.h"

#include "display.h"
#include "gui_commons.h"
#include "status_bar.h"
#include "splash_screen.h"

// LVGL
#include "esp_lvgl_port.h"

// LVGL Display handler
static lv_display_t *lvgl_disp = NULL;

// LVGL images declaration
LV_IMG_DECLARE(klaus_dab_126x85);

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
    splash_screen_draw();
    status_bar_init();
    return ESP_OK;
}