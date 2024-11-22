#include "userinputs.h"

static const char *TAG = "User Inputs";

// Common ESP-IDF helpers
#include "esp_check.h"
#include "esp_log.h"

// Key Button
#include "iot_button.h"

#define KEY_BTN (GPIO_NUM_6)
#define KEY_BTN_LONG_CLICK (2000)
#define KEY_BTN_SHORT_CLICK (50)

static button_handle_t key_btn = NULL;

// Knob
#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define KNOB_A (GPIO_NUM_4)
#define KNOB_B (GPIO_NUM_5)

#define KNOB_BTN (GPIO_NUM_0)
#define KNOB_LONG_CLICK (1000)
#define KNOB_SHORT_CLICK (50)

#define KNOB_HIGH_LIMIT (100)
#define KNOB_LOW_LIMIT (-100)

#define KNOB_GLITCH_FILTER (5000) // in ns
#define KNOB_CHECK_TIME (25)      // in ms
#define KNOB_TRESHOLD (2)

#define KNOB_QUEUE_SIZE (16)
#define KNOB_TASK_STACK_SIZE (4096)

static button_handle_t knob_btn = NULL;

static int pulse_count = 0;
static int prev_count = 0;
static int event_count = 0;
static QueueHandle_t queue;
static pcnt_unit_handle_t pcnt_unit = NULL;

// List of callbacks
static size_t userinput_callback_capacity = 10;
static size_t userinput_callback_registrered = 0;
userinputs_callback *userinputs_callbacks;
static bool ignore = false;

static void userinputs_parse_callbacks(user_actions_t action) {
  if (ignore) {
    return;
  }
  for (size_t i = 0; i < userinput_callback_registrered; i++) {
    userinputs_callbacks[i](action);
  }
}

static void knob_task(void *pvParams) {
  while (1) {
    if (xQueueReceive(queue, &event_count, pdMS_TO_TICKS(KNOB_CHECK_TIME))) {
      prev_count = 0;
    } else {
      ESP_ERROR_CHECK(pcnt_unit_get_count(pcnt_unit, &pulse_count));
      if (pulse_count != prev_count) {

        if (pulse_count >= prev_count + KNOB_TRESHOLD) {
          prev_count = pulse_count;
          userinputs_parse_callbacks(WHEEL_UP);
        } else if (pulse_count <= prev_count - KNOB_TRESHOLD) {
          prev_count = pulse_count;
          userinputs_parse_callbacks(WHEEL_DOWN);
        }
      }
    }
  }
}

static void keybtn_clicked(void *arg, void *data) {
  button_event_t event;
  event = iot_button_get_event(key_btn);
  switch (event) {
  case BUTTON_LONG_PRESS_START:
    userinputs_parse_callbacks(KEY_CLICK_LONG);
    break;

  case BUTTON_SINGLE_CLICK:
    userinputs_parse_callbacks(KEY_CLICK_SHORT);

  default:
    break;
  }
}

static void knob_clicked(void *arg, void *data) {
  button_event_t event;
  event = iot_button_get_event(knob_btn);
  switch (event) {
  case BUTTON_LONG_PRESS_START:
    userinputs_parse_callbacks(WHEEL_CLICK_LONG);
    break;

  case BUTTON_SINGLE_CLICK:
    userinputs_parse_callbacks(WHEEL_CLICK_SHORT);

  default:
    break;
  }
}

static bool pcnt_on_reach(pcnt_unit_handle_t unit,
                          const pcnt_watch_event_data_t *edata,
                          void *user_ctx) {
  BaseType_t high_task_wakeup;
  QueueHandle_t queue = (QueueHandle_t)user_ctx;
  xQueueSendFromISR(queue, &(edata->watch_point_value), &high_task_wakeup);
  return (high_task_wakeup == pdTRUE);
}

esp_err_t userinputs_init(void) {
  // Allocate space for callback list
  userinputs_callbacks =
      malloc(userinput_callback_capacity * sizeof(userinputs_callback));

  // Create gpio key button
  button_config_t key_btn_cfg = {
      .type = BUTTON_TYPE_GPIO,
      .long_press_time = KEY_BTN_LONG_CLICK,
      .short_press_time = KEY_BTN_SHORT_CLICK,
      .gpio_button_config =
          {
              .gpio_num = KEY_BTN,
              .active_level = 0,
          },
  };

  key_btn = iot_button_create(&key_btn_cfg);
  if (key_btn == NULL) {
    ESP_LOGE(TAG, "Failed to create key button");
    return ESP_FAIL;
  }
  iot_button_register_cb(key_btn, BUTTON_LONG_PRESS_START, keybtn_clicked,
                         NULL);
  iot_button_register_cb(key_btn, BUTTON_SINGLE_CLICK, keybtn_clicked, NULL);

  // Create knob click button
  button_config_t knob_btn_cfg = {
      .type = BUTTON_TYPE_GPIO,
      .long_press_time = KNOB_LONG_CLICK,
      .short_press_time = KNOB_SHORT_CLICK,
      .gpio_button_config =
          {
              .gpio_num = KNOB_BTN,
              .active_level = 0,
          },
  };

  knob_btn = iot_button_create(&knob_btn_cfg);
  if (knob_btn == NULL) {
    ESP_LOGE(TAG, "Failed to create knob click button");
    return ESP_FAIL;
  }
  iot_button_register_cb(knob_btn, BUTTON_LONG_PRESS_START, knob_clicked, NULL);
  iot_button_register_cb(knob_btn, BUTTON_SINGLE_CLICK, knob_clicked, NULL);

  pcnt_unit_config_t unit_config = {
      .high_limit = KNOB_HIGH_LIMIT,
      .low_limit = KNOB_LOW_LIMIT,
  };

  ESP_RETURN_ON_ERROR(pcnt_new_unit(&unit_config, &pcnt_unit), TAG,
                      "PCNT Init failed");

  pcnt_glitch_filter_config_t filter_config = {
      .max_glitch_ns = KNOB_GLITCH_FILTER,
  };
  pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config);

  // Channel A
  pcnt_chan_config_t chan_a_config = {
      .edge_gpio_num = KNOB_A,
      .level_gpio_num = KNOB_B,
  };
  pcnt_channel_handle_t pcnt_chan_a = NULL;
  ESP_RETURN_ON_ERROR(pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a),
                      TAG, "Failed to init channel A");
  // Channel B
  pcnt_chan_config_t chan_b_config = {
      .edge_gpio_num = KNOB_B,
      .level_gpio_num = KNOB_A,
  };
  pcnt_channel_handle_t pcnt_chan_b = NULL;
  ESP_RETURN_ON_ERROR(pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b),
                      TAG, "Failed to init channel A");

  pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE,
                               PCNT_CHANNEL_EDGE_ACTION_INCREASE);
  pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
  pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE,
                               PCNT_CHANNEL_EDGE_ACTION_DECREASE);
  pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP,
                                PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

  int watch_points[] = {KNOB_LOW_LIMIT, KNOB_HIGH_LIMIT};
  for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++) {
    ESP_ERROR_CHECK(pcnt_unit_add_watch_point(pcnt_unit, watch_points[i]));
  }

  pcnt_event_callbacks_t cbs = {
      .on_reach = pcnt_on_reach,
  };
  queue = xQueueCreate(KNOB_QUEUE_SIZE, sizeof(int));
  ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcnt_unit, &cbs, queue));
  ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
  ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
  ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));
  xTaskCreate(knob_task, "knob_task", 4096, NULL, 2, NULL);
  return ESP_OK;
}

void userinputs_register_callback(userinputs_callback callback) {
  // Callback list is full reallocate space for it
  if (userinput_callback_registrered >= userinput_callback_capacity) {
    userinput_callback_capacity *= 2;
    userinputs_callbacks =
        realloc(userinputs_callbacks, userinput_callback_capacity);
  }
  userinput_callback_registrered++;
  userinputs_callbacks[userinput_callback_registrered - 1] = callback;
}

void userinputs_set_ignore(bool _ignore) { ignore = _ignore; }