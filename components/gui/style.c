#include "style.h"

static lv_style_t background_main;
static lv_style_t background_status_bar;
static lv_style_t background_transparent;
static lv_style_t background_highlight;
static lv_style_t background_alt_highlight;
static lv_style_t background_danger;
static lv_style_t background_popup;

static lv_style_t font_bigfont;
static lv_style_t font_highlight;
static lv_style_t font_alt_highlight;
static lv_style_t font_danger;

static lv_style_t bar_indic;
static lv_style_t bar_bg;
static lv_style_t bar_knob;

void style_init(void) {
  // Transparent background
  lv_style_init(&background_transparent);
  lv_style_reset(&background_transparent);
  lv_style_set_bg_opa(&background_transparent, LV_OPA_0);
  lv_style_set_text_color(&background_transparent, lv_color_hex(TEXT_COLOR));
  lv_style_set_radius(&background_transparent, 0);
  lv_style_set_border_width(&background_transparent, 0);
  lv_style_set_pad_all(&background_transparent, 0);

  // Popup background
  lv_style_init(&background_popup);
  lv_style_set_bg_color(&background_popup, lv_color_hex(BACKGROUND_COLOR));
  lv_style_set_text_color(&background_popup, lv_color_hex(TEXT_COLOR));
  lv_style_set_border_width(&background_popup, 0);
  /*
  lv_style_set_border_color(&background_popup,
                            lv_color_hex(HIGHLIGHT_ALT_COLOR));
  */
  lv_style_set_radius(&background_popup, 10);
  lv_style_set_pad_all(&background_popup, 2);

  // Highlight background
  lv_style_init(&background_highlight);
  lv_style_set_bg_color(&background_highlight, lv_color_hex(HIGHLIGHT_COLOR));

  // Highlight Alt background
  lv_style_init(&background_alt_highlight);
  lv_style_set_bg_color(&background_alt_highlight,
                        lv_color_hex(HIGHLIGHT_ALT_COLOR));

  // Danger background use by scrollbar
  lv_style_init(&background_danger);
  lv_style_set_bg_color(&background_danger, lv_color_hex(DANGER_COLOR));

  // Status bar background
  lv_style_init(&background_status_bar);
  lv_style_reset(&background_status_bar);
  lv_style_set_bg_color(&background_status_bar, lv_color_hex(BACKGROUND_COLOR));
  lv_style_set_text_color(&background_status_bar, lv_color_hex(TEXT_COLOR));
  lv_style_set_radius(&background_status_bar, 0);
  lv_style_set_border_width(&background_status_bar, 0);
  lv_style_set_pad_all(&background_status_bar, 0);

  // Add a gradient to background
  static lv_grad_dsc_t grad_bar;
  grad_bar.dir = LV_GRAD_DIR_VER;
  grad_bar.stops_count = 2;
  grad_bar.stops[0].color = lv_color_hex(BACKGROUND_GRAD_COLOR);
  grad_bar.stops[0].opa = LV_OPA_COVER;
  grad_bar.stops[1].color = lv_color_hex(BACKGROUND_COLOR);
  grad_bar.stops[1].opa = LV_OPA_COVER;
  /*Shift the gradient to the bottom*/
  grad_bar.stops[0].frac = 100;
  grad_bar.stops[1].frac = 200;
  lv_style_set_bg_grad(&background_status_bar, &grad_bar);

  // Background Main with gradient
  lv_style_init(&background_main);
  lv_style_reset(&background_main);
  lv_style_set_bg_color(&background_main, lv_color_hex(BACKGROUND_COLOR));
  lv_style_set_text_color(&background_main, lv_color_hex(TEXT_COLOR));
  lv_style_set_radius(&background_main, 0);
  lv_style_set_border_width(&background_main, 0);
  lv_style_set_pad_all(&background_main, 0);
  // Add a gradient to background
  lv_color_t back_color = lv_color_hex(BACKGROUND_GRAD_COLOR);
  lv_color_t back_grad_color = lv_color_hex(BACKGROUND_COLOR);

  lv_color_t grad_colors[2] = {
      back_grad_color,
      back_color,
  };
  static lv_grad_dsc_t grad;
  lv_gradient_init_stops(&grad, grad_colors, NULL, NULL,
                         sizeof(grad_colors) / sizeof(lv_color_t));
  lv_grad_radial_init(&grad, LV_GRAD_CENTER, LV_GRAD_CENTER, LV_GRAD_RIGHT,
                      LV_GRAD_BOTTOM, LV_GRAD_EXTEND_PAD);
  lv_style_set_bg_grad(&background_main, &grad);

  // Bigfont
  lv_style_set_text_font(&font_bigfont, &lv_font_unscii_16);

  // Highlight
  lv_style_set_text_color(&font_highlight, lv_color_hex(HIGHLIGHT_COLOR));

  // Alt Highlight
  lv_style_set_text_color(&font_alt_highlight,
                          lv_color_hex(HIGHLIGHT_ALT_COLOR));

  // Danger color
  lv_style_set_text_color(&font_danger, lv_color_hex(DANGER_COLOR));

  // Bar indic
  lv_style_init(&bar_indic);
  lv_style_set_bg_opa(&bar_indic, LV_OPA_COVER);
  lv_style_set_radius(&bar_indic, 6);
  // Add a gradient to bar indic
  static lv_grad_dsc_t bar_grad;
  bar_grad.dir = LV_GRAD_DIR_HOR;
  bar_grad.stops_count = 2;
  bar_grad.stops[0].color = lv_color_hex(FOREGROUND_COLOR);
  bar_grad.stops[0].opa = LV_OPA_COVER;
  bar_grad.stops[1].color = lv_color_hex(FOREGROUND_GRAD_COLOR);
  bar_grad.stops[1].opa = LV_OPA_COVER;
  /*Shift the gradient to the bottom*/
  bar_grad.stops[0].frac = 100;
  bar_grad.stops[1].frac = 200;
  lv_style_set_bg_grad(&bar_indic, &bar_grad);

  // Bar background
  lv_style_init(&bar_bg);
  lv_style_set_bg_opa(&bar_bg, LV_OPA_40);
  lv_style_set_bg_color(&bar_bg, lv_color_hex(BAR_BACKGROUND_COLOR));
  lv_style_set_radius(&bar_bg, 6);

  // Bar Knob
  lv_style_init(&bar_knob);
  lv_style_set_bg_opa(&bar_knob, LV_OPA_COVER);
  lv_style_set_bg_color(&bar_knob, lv_color_hex(TEXT_COLOR));
}

lv_style_t *style_get_background_main(void) { return &background_main; }

lv_style_t *style_get_background_status_bar(void) {
  return &background_status_bar;
}

lv_style_t *style_get_background_transparent(void) {
  return &background_transparent;
}

lv_style_t *style_get_background_highlight(void) {
  return &background_highlight;
}

lv_style_t *style_get_background_alt_highlight(void) {
  return &background_alt_highlight;
}

lv_style_t *style_get_background_danger(void) { return &background_danger; }

lv_style_t *style_get_background_popup(void) { return &background_popup; }

lv_style_t *style_get_font_bigfont(void) { return &font_bigfont; }

lv_style_t *style_get_font_highlight(void) { return &font_highlight; }

lv_style_t *style_get_font_alt_highlight(void) { return &font_alt_highlight; }

lv_style_t *style_get_font_danger(void) { return &font_danger; }

lv_style_t *style_get_bar_indic(void) { return &bar_indic; }

lv_style_t *style_get_bar_background(void) { return &bar_bg; }

lv_style_t *style_get_bar_knob(void) { return &bar_knob; }
