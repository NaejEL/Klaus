#pragma once

#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"

/*
Time zone spec: https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
*/


esp_err_t clock_set(const char* timezone);
const char* clock_get_time(void);