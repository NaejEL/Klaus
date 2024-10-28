#pragma once

// Common ESP-IDF Helpers
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

// knob
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/pulse_cnt.h"
#include "driver/gpio.h"

// Button
#include "iot_button.h"

// Update display
#include "display.h"

#define KNOB_A (GPIO_NUM_4)
#define KNOB_B (GPIO_NUM_5)

#define KNOB_BTN (GPIO_NUM_0)
#define KNOB_LONG_CLICK (1000)
#define KNOB_SHORT_CLICK (50)

#define KNOB_HIGH_LIMIT (100)
#define KNOB_LOW_LIMIT (-100)

#define KNOB_GLITCH_FILTER (5000) // in ns
#define KNOB_CHECK_TIME (25)     // in ms
#define KNOB_TRESHOLD (2)

#define KNOB_QUEUE_SIZE (5)
#define KNOB_TASK_STACK_SIZE (3500)

void knob_task(void *pvParameter);