#pragma once

#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <string.h>

#define MOUNT_POINT "/KlausSD"
#define MAX_PATH_SIZE 254

esp_err_t sd_init(spi_host_device_t spi_host, gpio_num_t cs);

esp_err_t sd_write_file(const char *path, char *data);

void sd_ls(const char *path);

char *sd_get_file_content(const char *path);

bool sd_is_present(void);