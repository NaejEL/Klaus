#include "bq25896.h"

// Common ESP-IDF Helpers
#include "esp_check.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "bq25896";

static i2c_port_t i2c_port;
static SemaphoreHandle_t i2c_lock = NULL;

static esp_err_t bq25896_read_register(uint8_t reg, uint8_t *buffer,
                                       uint8_t buffer_size) {
  while (xSemaphoreTake(i2c_lock, portMAX_DELAY) != pdPASS) {
    ESP_LOGI(TAG, "Read cannot take the lock");
    return ESP_FAIL;
  }
  esp_err_t result = i2c_master_write_read_device(
      i2c_port, BQ25896_ADDR, (uint8_t *)&reg, 1, buffer, buffer_size,
      200 / portTICK_PERIOD_MS);
  xSemaphoreGive(i2c_lock);
  return result;
}

static esp_err_t bq25896_write_register(uint8_t reg, uint8_t *buffer,
                                        uint8_t buffer_size) {
  uint8_t *write_buffer =
      (uint8_t *)malloc(sizeof(uint8_t) * (buffer_size + 1));
  if (!write_buffer) {
    return ESP_FAIL;
  }
  write_buffer[0] = reg;
  memcpy(write_buffer + 1, buffer, buffer_size);
  while (xSemaphoreTake(i2c_lock, portMAX_DELAY) != pdPASS) {
    ESP_LOGI(TAG, "Write cannot take the lock");
    return ESP_FAIL;
  }
  esp_err_t ret =
      i2c_master_write_to_device(i2c_port, BQ25896_ADDR, write_buffer,
                                 buffer_size + 1, 200 / portTICK_PERIOD_MS);
  xSemaphoreGive(i2c_lock);
  free(write_buffer);
  return ret;
}

static bool bq25896_get_register_bit(uint8_t reg, uint8_t bit) {
  uint8_t value;
  bq25896_read_register(reg, &value, 1);
  return value & BIT(bit);
}

static esp_err_t bq25896_set_register_bit(uint8_t reg, uint8_t bit) {
  uint8_t value;
  esp_err_t ret = bq25896_read_register(reg, &value, 1);
  if (ret != ESP_OK) {
    return ret;
  }
  value |= (BIT(bit));
  return bq25896_write_register(reg, &value, 1);
}

static esp_err_t bq25896_clear_register_bit(uint8_t reg, uint8_t bit) {
  uint8_t value;
  esp_err_t ret = bq25896_read_register(reg, &value, 1);
  if (ret != ESP_OK) {
    return ret;
  }
  value &= (~BIT(bit));
  return bq25896_write_register(reg, &value, 1);
}

void bq25896_init(i2c_port_t _i2c_port, SemaphoreHandle_t _i2c_lock) {
  i2c_lock = _i2c_lock;
  i2c_port = _i2c_port;
  bq25896_set_power_off_voltage(BQ25896_PWOFF_TRESHOLD);
  bq25896_set_input_current_limit(BQ25896_CURR_MAX);
  bq25896_disable_current_limit_pin();
  bq25896_set_charge_target_voltage(BQ25896_CHARGE_VOLT_DEFAULT);
  bq25896_set_precharge_current(BQ25896_PRECHARGE_CURR_MIN);
  bq25896_set_charger_constant_current(BQ25896_CHARGE_CONST_CURR_DEFAULT);
  bq25896_enable_adc();
  bq25896_disable_watchdog();
  bq25896_enable_charge();
  // bq25896_reset_registers();
}

void bq25896_set_precharge_current(uint16_t milliamps) {
  if (milliamps % BQ25896_PRECHARGE_CURR_STEP) {
    uint16_t remainder = milliamps % BQ25896_PRECHARGE_CURR_STEP;
    milliamps = milliamps + BQ25896_PRECHARGE_CURR_STEP - remainder;
  }
  if (milliamps > BQ25896_PRECHARGE_CURR_MAX) {
    milliamps = BQ25896_PRECHARGE_CURR_MAX;
  } else if (milliamps < BQ25896_PRECHARGE_CURR_MIN) {
    milliamps = BQ25896_PRECHARGE_CURR_MIN;
  }

  uint8_t value;
  bq25896_read_register(0x05, &value, 1);
  value &= 0x0F;
  value |=
      (((milliamps - BQ25896_PRECHARGE_CURR_MIN) / BQ25896_PRECHARGE_CURR_STEP)
       << 4);
  bq25896_write_register(0x05, &value, 1);
}

uint16_t bq25896_get_precharge_current(void) {
  uint8_t value;
  bq25896_read_register(0x05, &value, 1);
  uint16_t toreturn = (value & 0xF0) >> 4;
  return BQ25896_PRECHARGE_CURR_STEP + (toreturn * BQ25896_PRECHARGE_CURR_STEP);
}

void bq25896_set_charge_target_voltage(uint16_t millivolts) {
  // Check if cahrge target is multiple of 16
  if (millivolts % BQ25896_CHARGE_VOLT_STEP) {
    uint16_t remainder = millivolts % BQ25896_CHARGE_VOLT_STEP;
    millivolts = millivolts + BQ25896_CHARGE_VOLT_STEP - remainder;
  }
  if (millivolts > BQ25896_CHARGE_VOLT_MAX) {
    millivolts = BQ25896_CHARGE_VOLT_MAX;
  } else if (millivolts < BQ25896_CHARGE_VOLT_MIN) {
    millivolts = BQ25896_CHARGE_VOLT_MIN;
  }

  uint8_t value;
  bq25896_read_register(0x06, &value, 1);
  value &= 0x03;
  value |= (((millivolts - BQ25896_CHARGE_VOLT_MIN) / BQ25896_CHARGE_VOLT_STEP)
            << 2);
  bq25896_write_register(0x06, &value, 1);
}

uint16_t bq25896_get_charge_target_voltage(void) {
  uint8_t value;
  bq25896_read_register(0x06, &value, 1);
  uint16_t toreturn = (value & 0xFC) >> 2;
  if (toreturn > 0x30) {
    return BQ25896_CHARGE_VOLT_MAX;
  }
  return (toreturn * BQ25896_CHARGE_VOLT_STEP) + BQ25896_CHARGE_VOLT_MIN;
}

void bq25896_set_charger_constant_current(uint16_t milliamps) {
  if (milliamps % BQ25896_CHARGE_VOLT_STEP) {
    uint16_t remainder = milliamps % BQ25896_CHARGE_CONST_CURR_STEP;
    milliamps = milliamps + BQ25896_CHARGE_CONST_CURR_STEP - remainder;
  }
  if (milliamps > BQ25896_CHARGE_CONST_CURR_MAX) {
    milliamps = BQ25896_CHARGE_CONST_CURR_MAX;
  }

  uint8_t value;
  bq25896_read_register(0x04, &value, 1);
  value &= 0x80;
  value |= (milliamps / BQ25896_CHARGE_CONST_CURR_STEP);
  bq25896_write_register(0x04, &value, 1);
}

uint16_t bq25896_get_charger_constant_current(void) {
  uint8_t value;
  bq25896_read_register(0x04, &value, 1);
  uint16_t toreturn = (value & 0x7F) * BQ25896_CHARGE_CONST_CURR_STEP;
  return toreturn;
}

void bq25896_disable_current_limit_pin(void) {
  bq25896_clear_register_bit(0x00, 6);
}

void bq25896_enable_current_limit_pin(void) {
  bq25896_set_register_bit(0x00, 6);
}

bool bq25896_get_current_limit_pin_state(void) {
  return bq25896_get_register_bit(0x00, 6);
}

void bq25896_set_input_current_limit(uint16_t milliamps) {
  // Check if limit is multiple of 50
  if (milliamps % BQ25896_CURR_STEP) {
    uint16_t remainder = milliamps % BQ25896_CURR_STEP;
    milliamps = milliamps + BQ25896_CURR_STEP - remainder;
  }
  if (milliamps > BQ25896_CURR_MAX) {
    milliamps = BQ25896_CURR_MAX;
  } else if (milliamps < BQ25896_CURR_MIN) {
    milliamps = BQ25896_CURR_MIN;
  }

  uint8_t value;
  bq25896_read_register(0x00, &value, 1);
  value &= 0xC0;
  value |= ((milliamps - BQ25896_CURR_MIN) / BQ25896_CURR_STEP);
  bq25896_write_register(0x00, &value, 1);
}

uint16_t bq25896_get_input_current_limit(void) {
  uint8_t value;
  bq25896_read_register(0x00, &value, 1);
  uint16_t toreturn = value & 0x3F;
  return (toreturn * BQ25896_CURR_STEP) + BQ25896_CURR_MIN;
}

void bq25896_enable_adc(void) {
  uint8_t value;
  bq25896_read_register(0x02, &value, 1);
  value |= BIT(6);
  value |= BIT(7);
  bq25896_write_register(0x02, &value, 1);
}

void bq25896_disable_adc(void) {
  uint8_t value;
  bq25896_read_register(0x02, &value, 1);
  value &= (~BIT(7));
  bq25896_write_register(0x02, &value, 1);
}

void bq25896_enable_charge(void) { bq25896_set_register_bit(0x03, 4); }

void bq25896_disable_charge(void) { bq25896_clear_register_bit(0x03, 4); }

bool bq25896_get_charge_state(void) {
  return bq25896_get_register_bit(0x03, 4);
}

void bq25896_enable_watchdog(bq25896_watchdog_timeout_t timeout) {
  uint8_t value;
  bq25896_read_register(0x07, &value, 1);
  value &= 0xCF;
  switch (timeout) {
  case TIMEOUT_40SEC:
    value |= 0x10;
    break;
  case TIMEOUT_80SEC:
    value |= 0x20;
    break;
  case TIMEOUT_160SEC:
    value |= 0x30;
    break;

  default:
    break;
  }
  bq25896_write_register(0x0, &value, 1);
}

void bq25896_disable_watchdog(void) {
  uint8_t value;
  bq25896_read_register(0x07, &value, 1);
  value &= 0xCF;
  bq25896_write_register(0x07, &value, 1);
}

void bq25896_set_power_off_voltage(uint16_t threshold_millivolts) {
  // Check if threshold is multiple of 100
  if (threshold_millivolts % BQ25896_VOLT_STEP) {
    uint16_t remainder = threshold_millivolts % BQ25896_VOLT_STEP;
    threshold_millivolts = threshold_millivolts + BQ25896_VOLT_STEP - remainder;
  }
  if (threshold_millivolts > BQ25896_VOLT_MAX) {
    threshold_millivolts = BQ25896_VOLT_MAX;
  } else if (threshold_millivolts < BQ25896_VOLT_MIN) {
    threshold_millivolts = BQ25896_VOLT_MIN;
  }

  uint8_t value;
  bq25896_read_register(0x03, &value, 1);
  value &= 0xF1;
  value |= (threshold_millivolts - BQ25896_VOLT_MIN) / BQ25896_VOLT_STEP;
  value <<= 1;
  bq25896_write_register(0x03, &value, 1);
}

uint16_t bq25896_get_power_off_voltage(void) {
  uint8_t value;
  bq25896_read_register(0x03, &value, 1);
  uint16_t toreturn = value & 0x0E;
  toreturn >>= 1;
  return (toreturn * BQ25896_VOLT_STEP) + BQ25896_VOLT_MIN;
}

uint8_t bq25896_get_devices_id() {
  uint8_t value;
  bq25896_read_register(0x14, &value, 1);
  return (value & 0x03);
}

bq25896_charge_status_t bq25896_get_charge_status() {
  uint8_t value;
  bq25896_read_register(0x0B, &value, 1);
  return (bq25896_charge_status_t)((value >> 3) & 0x03);
}

void bq25896_reset_registers() {
  bq25896_set_register_bit(0x14, 7);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  bq25896_clear_register_bit(0x14, 7);
}