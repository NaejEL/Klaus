#pragma once

#include <string.h>
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#define SD_SPI_CS (GPIO_NUM_13)

#define MOUNT_POINT "/KlausSD"
#define MAX_PATH_SIZE 254

esp_err_t sd_init(spi_host_device_t spi_host);

esp_err_t sd_write_file(const char *path, char *data);

void sd_ls(const char *path);

char *sd_get_file_content(const char *path);

bool sd_is_present(void);