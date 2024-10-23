#include "keybtn.h"

static const char *TAG = "Key Button";

static button_handle_t gpio_btn = NULL;

static void keybtn_clicked(void *arg, void *data)
{
    button_event_t event;
    event = iot_button_get_event(gpio_btn);
    switch (event)
    {
    case BUTTON_LONG_PRESS_START:
        user_action(KEY_CLICK_LONG);
        break;

    case BUTTON_SINGLE_CLICK:
        user_action(KEY_CLICK_SHORT);

    default:
        break;
    }
}

void keybtn_init(void)
{
    // create gpio button
    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = KEY_BTN_LONG_CLICK,
        .short_press_time = KEY_BTN_SHORT_CLICK,
        .gpio_button_config = {
            .gpio_num = KEY_BTN,
            .active_level = 0,
        },
    };

    gpio_btn = iot_button_create(&gpio_btn_cfg);
    if (NULL == gpio_btn)
    {
        ESP_LOGE(TAG, "Button create failed");
    }
    iot_button_register_cb(gpio_btn, BUTTON_LONG_PRESS_START, keybtn_clicked, NULL);
    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, keybtn_clicked, NULL);
}
