// Common ESP-IDF Helpers
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "KlausFirmware";

// GUI
#include "display.h"

// Key Button
#include "keybtn.h"

// knob
#include "knob.h"

void app_main(void)
{
    display_init();
    display_backlight_on();
    lvgl_init();
    start_gui();
    keybtn_init();
    xTaskCreate(knob_task, "knob_task", KNOB_TASK_STACK_SIZE, NULL, 3, NULL);
}
