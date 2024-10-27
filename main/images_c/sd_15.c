
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#elif defined(LV_BUILD_TEST)
#include "../lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_SD_15
#define LV_ATTRIBUTE_SD_15
#endif

static const
LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_SD_15
uint8_t sd_15_map[] = {

    0x00,0x00,0x00,0x00,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x00,0x00,
    0x00,0x00,0x00,0x00,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,
    0x00,0x00,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x88,0xf4,
    0x00,0x00,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x88,0xf4,
    0x00,0x00,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x00,0x00,0x88,0xf4,0x88,0xf4,
    0x00,0x00,0x00,0x00,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,
    0x00,0x00,0x00,0x00,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,
    0x00,0x00,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,
    0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,
    0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,
    0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,
    0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,
    0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,
    0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,
    0x00,0x00,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x88,0xf4,0x00,0x00,
    0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0x00,0x00,0xff,0x00,0xff,0x00,0xff,0x09,0xff,0x22,0xff,0xff,0x00,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x22,0xff,0xff,
    0x00,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
    0x00,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00,

};

const lv_image_dsc_t sd_15 = {
  .header.magic = LV_IMAGE_HEADER_MAGIC,
  .header.cf = LV_COLOR_FORMAT_RGB565A8,
  .header.flags = 0,
  .header.w = 12,
  .header.h = 15,
  .header.stride = 24,
  .data_size = sizeof(sd_15_map),
  .data = sd_15_map,
};

