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

// I2C
#define SDA_PIN (GPIO_NUM_8)
#define SCL_PIN (GPIO_NUM_18)
#define I2C_PORT_NUM I2C_NUM_0
#define I2C_FREQ 400000

// Battery
#include "battery.h"

static void i2c_init(void){
i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = SDA_PIN;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = SCL_PIN;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FREQ;
    i2c_param_config(I2C_PORT_NUM, &conf);
    i2c_driver_install(I2C_PORT_NUM, conf.mode, 0, 0, 0);
}

void app_main(void)
{
    i2c_init();
    battery_init(I2C_PORT_NUM);
    
    display_init();
    display_backlight_on();
    lvgl_init();
    start_gui();
    keybtn_init();
    xTaskCreate(knob_task, "knob_task", KNOB_TASK_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(battery_task, "Battery Task", 4096,NULL,5,NULL);
}
