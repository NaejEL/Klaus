#include "sd.h"
#include <dirent.h>
static const char *TAG = "SD";

//static const char *config_file = "config.json";

static char *error_msg = "SD Error";

static sdmmc_card_t *card;
static bool sd_present = false;

//static const cJSON *json;
/*
static esp_err_t sd_parse_json()
{
    json = cJSON_Parse(sd_get_file_content(config_file));
    printf("Parsed Json:%s", cJSON_Print(json));
    return ESP_OK;
}
*/
esp_err_t sd_init(spi_host_device_t spi_host)
{
    sdspi_device_config_t sd_device = SDSPI_DEVICE_CONFIG_DEFAULT();
    sd_device.gpio_cs = SD_SPI_CS;
    sd_device.host_id = spi_host;

    sdspi_dev_handle_t sd_handle;
    sdspi_host_init_device(&sd_device, &sd_handle);

    sdmmc_host_t sd_host = SDSPI_HOST_DEFAULT();
    sd_host.slot = sd_handle;
    //sd_host.max_freq_khz = 10000;

    sdmmc_card_t *card;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 0};

    ESP_LOGI(TAG, "Mounting filesystem");
    esp_err_t ret = esp_vfs_fat_sdspi_mount(MOUNT_POINT, &sd_host, &sd_device, &mount_config, &card);
    if (ret != ESP_OK)
    {
        printf("Failed to mount filesystem. Error: %s\n", esp_err_to_name(ret));
        return ret;
    }
    sdmmc_card_print_info(stdout, card);
    sd_present = true;
    sd_ls(MOUNT_POINT);
    //sd_parse_json();
    return ESP_OK;
}

void sd_ls(const char *path)
{
    struct dirent *dp;
    DIR *dfd = opendir(path);
    if (dfd != NULL)
    {
        printf("\n%s:\n",path);
        while ((dp = readdir(dfd)) != NULL)
            printf("%s\n", dp->d_name);
        closedir(dfd);
        printf("\n");
    }
}

esp_err_t sd_write_file(const char *filename, char *data)
{
    char toOpen[MAX_PATH_SIZE];
    sprintf(toOpen, "%s/%s", MOUNT_POINT, filename);
    ESP_LOGI(TAG, "Opening file to write: %s", filename);
    FILE *f = fopen(toOpen, "w");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file:%s for writing", filename);
        fclose(f);
        return ESP_FAIL;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");
    return ESP_OK;
}

char *sd_get_file_content(const char *filename)
{
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    char toOpen[MAX_PATH_SIZE];
    sprintf(toOpen, "%s/%s", MOUNT_POINT, filename);
    ESP_LOGI(TAG, "Opening file to read: %s", toOpen);
    file = fopen(toOpen, "rb");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file:%s for reading", filename);
        fclose(file);
        return error_msg;
    }
    if (fseek(file, 0, SEEK_END) != 0)
    {
        ESP_LOGE(TAG, "Failed to seek (SEEK_END) file:%s", filename);
        fclose(file);
        return error_msg;
    }
    length = ftell(file);
    if (length < 0)
    {
        ESP_LOGE(TAG, "Failed to tell (length<0) file:%s", filename);
        fclose(file);
        return error_msg;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        ESP_LOGE(TAG, "Failed to seek (SEEK_SET) file:%s", filename);
        fclose(file);
        return error_msg;
    }
    /* allocate content buffer */
    content = (char *)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate content file:%s", filename);
        fclose(file);
        return error_msg;
    }
    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        fclose(file);
    }
    content[read_chars] = '\0';
    ESP_LOGI(TAG, "File length: %ldContent:%s\n",length, content);
    return content;
}

bool sd_is_present(void)
{
    return sd_present;
}