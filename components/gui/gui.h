#pragma once

// Common ESP-IDF Helpers
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

esp_err_t gui_init(void);

void gui_start(void);