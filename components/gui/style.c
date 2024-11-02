#include "style.h"

static lv_style_t background_style;
static lv_style_t bigfont;
static lv_style_t highlight;
static lv_style_t bar_style_indic;
static lv_style_t bar_style_bg;
static lv_style_t bar_style_knob;

void style_init(void)
{
    // Background with gradient
    lv_style_init(&background_style);
    lv_style_reset(&background_style);
    lv_style_set_bg_color(&background_style, lv_color_hex(BACKGROUND_COLOR));
    lv_style_set_text_color(&background_style, lv_color_hex(TEXT_COLOR));
    lv_style_set_radius(&background_style, 0);
    lv_style_set_border_width(&background_style, 0);
    lv_style_set_pad_all(&background_style, 0);
    // Add a gradient to background
    lv_color_t back_color = lv_color_hex(BACKGROUND_COLOR);
    lv_color_t back_grad_color = lv_color_hex(BACKGROUND_GRAD_COLOR);

    lv_color_t grad_colors[2] = {
        back_grad_color,
        back_color,
    };
    static lv_grad_dsc_t grad;
    lv_gradient_init_stops(&grad, grad_colors, NULL, NULL, sizeof(grad_colors) / sizeof(lv_color_t));
    lv_grad_radial_init(&grad, LV_GRAD_CENTER, LV_GRAD_CENTER, LV_GRAD_RIGHT, LV_GRAD_BOTTOM, LV_GRAD_EXTEND_PAD);
    lv_style_set_bg_grad(&background_style, &grad);

    // Bigfont
    lv_style_set_text_font(&bigfont, &lv_font_montserrat_22);

    // Highlight
    lv_style_set_text_color(&highlight,lv_color_hex(HIGHLIGHT_COLOR));

    // Bar indic
    lv_style_init(&bar_style_indic);
    lv_style_set_bg_opa(&bar_style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&bar_style_indic, lv_color_hex(FOREGROUND_COLOR));
    lv_style_set_radius(&bar_style_indic, 6);
    // Add a gradient to bar indic
    static lv_grad_dsc_t bar_grad;
    bar_grad.dir = LV_GRAD_DIR_HOR;
    bar_grad.stops_count = 2;
    bar_grad.stops[0].color = lv_color_hex(FOREGROUND_GRAD_COLOR);
    bar_grad.stops[0].opa = LV_OPA_COVER;
    bar_grad.stops[1].color = lv_color_hex(FOREGROUND_COLOR);
    bar_grad.stops[1].opa = LV_OPA_COVER;
    /*Shift the gradient to the bottom*/
    bar_grad.stops[0].frac = 100;
    bar_grad.stops[1].frac = 200;
    lv_style_set_bg_grad(&bar_style_indic, &bar_grad);

    // Bar background
    lv_style_init(&bar_style_bg);
    lv_style_set_bg_opa(&bar_style_bg, LV_OPA_30);
    lv_style_set_bg_color(&bar_style_bg, lv_color_hex(BAR_BACKGROUND_COLOR));
    lv_style_set_radius(&bar_style_bg, 6);

    // Bar Knob
    lv_style_init(&bar_style_knob);
    lv_style_set_bg_opa(&bar_style_knob, LV_OPA_COVER);
    lv_style_set_bg_color(&bar_style_knob, lv_color_hex(TEXT_COLOR));
}

lv_style_t *get_background_style(void)
{
    return &background_style;
}

lv_style_t *get_bigfont_style(void)
{
    return &bigfont;
}

lv_style_t *get_highlight_style(void){
    return &highlight;
}

lv_style_t *get_bar_style_indic(void)
{
    return &bar_style_indic;
}

lv_style_t *get_bar_style_background(void)
{
    return &bar_style_bg;
}

lv_style_t *get_bar_style_knob(void)
{
    return &bar_style_knob;
}