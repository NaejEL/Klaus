#pragma once
#include "view_commons.h"

/*

Color Palette: https://www.color-hex.com/color-palette/1052927

*/

#define MAIN_SCREEN_HEIGHT (150)
#define MAIN_SCREEN_WIDTH (320)

#define BACKGROUND_COLOR (0x353D4D)
#define BACKGROUND_GRAD_COLOR (0x0AB2A0)

#define FOREGROUND_COLOR (0xF8B234)
#define FOREGROUND_GRAD_COLOR (0xE75F3F)

#define HIGHLIGHT_COLOR (0x00EBBE)
#define HIGHLIGHT_ALT_COLOR (0xF8B234)
#define DANGER_COLOR (0xE75F3F)

#define TEXT_COLOR (0XF9EDC6)

#define BAR_BACKGROUND_COLOR (0x00EBBE)

void style_init(void);

lv_style_t *style_get_background_main(void);
lv_style_t *style_get_background_status_bar(void);
lv_style_t *style_get_background_transparent(void);
lv_style_t *style_get_background_danger(void);
lv_style_t *style_get_background_highlight(void);
lv_style_t *style_get_background_alt_highlight(void);
lv_style_t *style_get_background_popup(void);

lv_style_t *style_get_font_bigfont(void);
lv_style_t *style_get_font_highlight(void);
lv_style_t *style_get_font_alt_highlight(void);
lv_style_t *style_get_font_danger(void);

lv_style_t *style_get_bar_indic(void);
lv_style_t *style_get_bar_background(void);
lv_style_t *style_get_bar_knob(void);