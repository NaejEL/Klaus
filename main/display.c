#include "display.h"

static const char *TAG = "Display";

static bool backlight_state = false;

static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

static esp_err_t display_backlight_init(void);

// LVGL Display handler
static lv_display_t *lvgl_disp = NULL;

// LVGL images declaration
LV_IMG_DECLARE(klaus_dab_126x85);
LV_IMG_DECLARE(battery_15);

// Battery gauge
static void updateBatteryGauge(void);
lv_obj_t *battery_bar = NULL;
lv_obj_t *battery_label = NULL;
int battery_value = 0;

esp_err_t display_init(void)
{
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .mosi_io_num = LCD_SPI_MOSI,
        .miso_io_num = LCD_SPI_MIS0,
        .sclk_io_num = LCD_SPI_CLK,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = 0,
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "Initialize SPI bus failed");

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = LCD_SPI_CS,
        .dc_gpio_num = LCD_DC,
        .spi_mode = 0,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = LCD_QUEUE_SIZE,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_NUM, &io_config, &io_handle), TAG, "LCD new panel IO SPI failed");

    ESP_LOGD(TAG, "Install LCD driver");
    const esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST,
        .color_space = LCD_COLOR_SPACE,
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
    };
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle), TAG, "Create LCD panel failed");

    ESP_RETURN_ON_ERROR(esp_lcd_panel_reset(panel_handle), TAG, "LCD Panel Reset failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel_handle), TAG, "LCD panel init failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_swap_xy(panel_handle, LCD_SWAP_XY), TAG, "LCD panel swap X/Y failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_mirror(panel_handle, LCD_MIRROR_X, LCD_MIRROR_Y), TAG, "LCD panel mirror failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_invert_color(panel_handle, LCD_INVERT_COLOR), TAG, "LCD panel invert color failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_set_gap(panel_handle, LCD_X_GAP, LCD_Y_GAP), TAG, "LCD panel set gap failed");
    ESP_RETURN_ON_ERROR(display_backlight_init(), TAG, "LCD backlight init failed");
    ESP_RETURN_ON_ERROR(esp_lcd_panel_disp_on_off(panel_handle, true), TAG, "LCD Panel turn on failed");
    return ESP_OK;
}

static esp_err_t display_backlight_init(void)
{
    return gpio_set_direction(LCD_BACKLIGHT, GPIO_MODE_OUTPUT);
}

void display_backlight_on(void)
{
    gpio_set_level(LCD_BACKLIGHT, 1);
    backlight_state = true;
}

void display_backlight_off(void)
{
    gpio_set_level(LCD_BACKLIGHT, 0);
    backlight_state = false;
}

void display_backlight_toggle(void)
{
    if (backlight_state)
    {
        display_backlight_off();
    }
    else
    {
        display_backlight_on();
    }
}

esp_err_t lvgl_init(void)
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
        .io_handle = io_handle,
        .panel_handle = panel_handle,
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

    return ESP_OK;
}

void setBatteryValue(int value)
{
    battery_value = value;
}

void updateBatteryGauge(void)
{
    if (battery_bar != NULL && battery_value >= 0)
    {
        lvgl_port_lock(0);
        lv_bar_set_value(battery_bar, battery_value, LV_ANIM_OFF);
        lvgl_port_unlock();
    }
}

void updateBatteryLabel(void)
{
    if (battery_label != NULL && battery_value >= 0)
    {
        lvgl_port_lock(0);
        lv_label_set_text_fmt(battery_label, "%d%%", battery_value);
        lvgl_port_unlock();
    }
}

void batteryTask(void *pvParameters)
{
    while (1)
    {
        updateBatteryGauge();
        updateBatteryLabel();
        vTaskDelay(BATTERY_BAR_REFRESH_RATE / portTICK_PERIOD_MS); // Delay 5 seconds between updates
    }
}

void user_action(user_actions_t action)
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

void start_gui(void)
{
    lvgl_port_lock(0);
    lv_obj_t *scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, lv_color_hex(BACKGROUND_COLOR), LV_PART_MAIN);
    lv_obj_set_style_text_color(scr, lv_color_hex(FOREGROUND_COLOR), LV_PART_MAIN);

    lv_obj_t *klausDab = lv_image_create(scr);
    lv_image_set_src(klausDab, &klaus_dab_126x85);
    lv_obj_align(klausDab, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *batLogo = lv_image_create(scr);
    lv_image_set_src(batLogo, &battery_15);
    lv_obj_align(batLogo, LV_ALIGN_TOP_LEFT, 0, 2);

    // Battery Label
    battery_label = lv_label_create(scr);
    lv_obj_align(battery_label, LV_ALIGN_TOP_LEFT, 90, 2);
    lv_label_set_text_fmt(battery_label, "%d%%", battery_value);

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
    lv_obj_set_size(battery_bar, 70, 10);
    lv_obj_align(battery_bar, LV_ALIGN_TOP_LEFT, 15, 5);
    lv_bar_set_value(battery_bar, 70, LV_ANIM_OFF);

    lvgl_port_unlock();
    xTaskCreate(batteryTask, "batteryTask", 4096, NULL, 4, NULL);
}