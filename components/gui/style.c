#include "style.h"

static lv_style_t background_style;

void style_init(void)
{
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
}

lv_style_t *get_background_style(void)
{
    return &background_style;
}