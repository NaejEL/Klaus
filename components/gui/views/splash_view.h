#pragma once

// LVGL
#include "esp_lvgl_port.h"

#include "../gui_commons.h"

// LVGL images declaration
LV_IMG_DECLARE(klaus_112x85);

// Background Image
lv_obj_t *splash_image;

void splash_view_draw()
{
    lvgl_port_lock(0);
    active_view = SPLASH_VIEW;
    if(current_view!=NULL){
        lv_obj_clean(current_view);
    }
    current_view = splash_view;
    splash_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(splash_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(splash_view, LV_ALIGN_TOP_LEFT, 0, 20);

    // Set splash screen style
    static lv_style_t splash_screen_style;
    lv_style_init(&splash_screen_style);
    lv_style_reset(&splash_screen_style);
    lv_style_set_bg_color(&splash_screen_style, lv_color_hex(BACKGROUND_COLOR));
    lv_style_set_text_color(&splash_screen_style, lv_color_hex(TEXT_COLOR));
    lv_style_set_radius(&splash_screen_style, 0);
    lv_style_set_border_width(&splash_screen_style, 0);
    lv_style_set_pad_all(&splash_screen_style, 0);
    lv_obj_add_style(splash_view, &splash_screen_style, LV_PART_MAIN);
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
    lv_style_set_bg_grad(&splash_screen_style, &grad);

    lv_obj_add_style(splash_view, &splash_screen_style, 0);

    splash_image = lv_image_create(splash_view);
    lv_image_set_src(splash_image, &klaus_112x85);
    lv_obj_align(splash_image, LV_ALIGN_CENTER, 0, 0);
    lvgl_port_unlock();
}