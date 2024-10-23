#include "knob.h"

static const char *TAG = "Knob";

static int pulse_count = 0;
static int prev_count = 0;
static int event_count = 0;
static QueueHandle_t queue;
static pcnt_unit_handle_t pcnt_unit = NULL;
static button_handle_t gpio_btn = NULL;

static bool pcnt_on_reach(pcnt_unit_handle_t unit, const pcnt_watch_event_data_t *edata, void *user_ctx)
{
    BaseType_t high_task_wakeup;
    QueueHandle_t queue = (QueueHandle_t)user_ctx;
    xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
    return (high_task_wakeup == pdTRUE);
}

static void knob_clicked(void *arg, void *data)
{
    button_event_t event;
    event = iot_button_get_event(gpio_btn);
    switch (event)
    {
    case BUTTON_LONG_PRESS_START:
        user_action(WHEEL_CLICK_LONG);
        break;

    case BUTTON_SINGLE_CLICK:
        user_action(WHEEL_CLICK_SHORT);

    default:
        break;
    }
}

static void knob_init(void)
{
    // Create encoder button
    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = KNOB_LONG_CLICK,
        .short_press_time = KNOB_SHORT_CLICK,
        .gpio_button_config = {
            .gpio_num = KNOB_BTN,
            .active_level = 0,
        },
    };

    gpio_btn = iot_button_create(&gpio_btn_cfg);
    if (gpio_btn == NULL)
    {
        ESP_LOGE(TAG, "Button create failed");
    }
    iot_button_register_cb(gpio_btn, BUTTON_LONG_PRESS_START, knob_clicked, NULL);
    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, knob_clicked, NULL);

    pcnt_unit_config_t unit_config = {
        .high_limit = KNOB_HIGH_LIMIT,
        .low_limit = KNOB_LOW_LIMIT,
    };

    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    ESP_LOGI(TAG, "set glitch filter");
    pcnt_glitch_filter_config_t filter_config = {
        .max_glitch_ns = KNOB_GLITCH_FILTER,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    ESP_LOGI(TAG, "install pcnt channels");
    // Channel A
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = KNOB_A,
        .level_gpio_num = KNOB_B,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a));
    // Channel B
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = KNOB_B,
        .level_gpio_num = KNOB_A,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b));

    ESP_LOGI(TAG, "set edge and level actions for pcnt channels");
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE));

    ESP_LOGI(TAG, "add watch points and register callbacks");
    int watch_points[] = {KNOB_LOW_LIMIT, KNOB_HIGH_LIMIT};
    for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++)
    {
        ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, watch_points[i]));
    }

    pcnt_event_callbacks_t cbs = {
        .on_reach = pcnt_on_reach,
    };
    queue = xQueueCreate(KNOB_QUEUE_SIZE, sizeof(int));
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));

    ESP_LOGI(TAG, "enable pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_LOGI(TAG, "clear pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_LOGI(TAG, "start pcnt unit");
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
}

void knob_task(void *pvParameter)
{
    knob_init();
    while (1)
    {
        if (xQueueReceive(queue, &event_count, pdMS_TO_TICKS(KNOB_CHECK_TIME)))
        {
            prev_count = 0;
        }
        else
        {
            ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
            if (pulse_count != prev_count)
            {

                if (pulse_count >= prev_count + KNOB_TRESHOLD)
                {
                    prev_count = pulse_count;
                    user_action(WHEEL_UP);
                }
                else if (pulse_count <= prev_count - KNOB_TRESHOLD)
                {
                    prev_count = pulse_count;
                    user_action(WHEEL_DOWN);
                }
            }
        }
    }
}