#pragma once
#include "view_commons.h"

#define MAIN_SCREEN_HEIGHT (150)
#define MAIN_SCREEN_WIDTH (320)

#define BACKGROUND_COLOR (0x353D4D)
#define BACKGROUND_GRAD_COLOR (0x0AB2A0)

#define FOREGROUND_COLOR (0xE75F3F)
#define FOREGROUND_GRAD_COLOR (0xF8B234)

#define HIGHLIGHT_COLOR (0x00EBBE)
#define DANGER_COLOR (0xE55E3C)

#define TEXT_COLOR (0XF9EDC6)

#define BAR_BACKGROUND_COLOR (0X000000)

void style_init(void);

lv_style_t *get_background_style(void);

lv_style_t *get_bigfont_style(void);
lv_style_t *get_highlight_style(void);

lv_style_t *get_bar_style_indic(void);
lv_style_t *get_bar_style_background(void);
lv_style_t *get_bar_style_knob(void);