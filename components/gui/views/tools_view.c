#include "tools_view.h"
#include "display.h"

static view_handler_t *calling_view;

static lv_obj_t *tools_view;
static view_handler_t tools_view_handler;

typedef enum {
  TOOLS_MENU_REBOOT,
  TOOLS_MENU_BACKLIGHT,

  TOOLS_MENU_SIZE
} tools_menu_items_t;

typedef enum {
  TOOLS_NOT_IN_VIEW,
  TOOLS_VIEW_MAIN,
  TOOLS_VIEW_BACKLIGHT,

  TOOLS_VIEWS_SIZE
} tools_views_t;

static const char *tools_menu_texts[TOOLS_MENU_SIZE] = {"REBOOT", "BACKLIGHT"};

static void tools_view_draw(view_handler_t *_calling_view);

// Backlight
static void tools_draw_backlight_view();
static uint8_t current_backlight;
static lv_obj_t *backlight_slider;

static tools_menu_items_t current_menu_item;
static tools_views_t current_view;

static void tools_input_handler(user_actions_t user_action) {
  if (user_action == KEY_CLICK_SHORT) {
    if (current_view == TOOLS_VIEW_MAIN) {
      calling_view->draw_view(tools_view_get_handler());
    } else {
      tools_view_draw(calling_view);
    }
  } else if (user_action == WHEEL_CLICK_SHORT) {
    if (current_menu_item == TOOLS_MENU_REBOOT) {
      esp_restart();
    } else if (current_menu_item == TOOLS_MENU_BACKLIGHT) {
      tools_draw_backlight_view();
    }
  } else if (user_action == WHEEL_UP) {
    if (current_view == TOOLS_VIEW_MAIN) {
      current_menu_item++;
      if (current_menu_item >= TOOLS_MENU_SIZE) {
        current_menu_item = (tools_menu_items_t)0;
      }
      tools_view_draw(calling_view);
    } else if (current_view == TOOLS_VIEW_BACKLIGHT) {
      if (current_backlight < 100) {
        current_backlight++;
        display_blacklight_set_default_intensity(current_backlight);
        tools_draw_backlight_view();
      }
    }
  } else if (user_action == WHEEL_DOWN) {
    if (current_view == TOOLS_VIEW_MAIN) {
      if (current_menu_item > 0) {
        current_menu_item--;
      } else {
        current_menu_item = TOOLS_MENU_SIZE - 1;
      }
      tools_view_draw(calling_view);
    } else if (current_view == TOOLS_VIEW_BACKLIGHT) {
      if (current_backlight > 1) {
        current_backlight--;
        display_blacklight_set_default_intensity(current_backlight);
        tools_draw_backlight_view();
      }
    }
  }
}

static void tools_draw_backlight_view() {
  current_view = TOOLS_VIEW_BACKLIGHT;
  lvgl_port_lock(0);

  lv_obj_clean(tools_view);
  tools_view = lv_obj_create(lv_screen_active());
  lv_obj_set_size(tools_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
  lv_obj_align(tools_view, LV_ALIGN_TOP_LEFT, 0, 20);
  lv_obj_add_style(tools_view, style_get_background_main(), LV_PART_MAIN);

  backlight_slider = lv_slider_create(tools_view);
  lv_obj_add_style(backlight_slider, style_get_bar_background(), LV_PART_MAIN);
  lv_obj_add_style(backlight_slider, style_get_bar_indic(), LV_PART_INDICATOR);
  lv_obj_add_style(backlight_slider, style_get_background_alt_highlight(),
                   LV_PART_KNOB);

  lv_obj_center(backlight_slider);
  lv_slider_set_value(backlight_slider, current_backlight, LV_ANIM_OFF);

  lvgl_port_unlock();
}

static void tools_view_clear() {
  lvgl_port_lock(0);
  lv_obj_clean(tools_view);
  lvgl_port_unlock();
}

static void tools_view_draw(view_handler_t *_calling_view) {
  calling_view = _calling_view;
  if (calling_view != tools_view_get_handler()) {
    calling_view->clear_view();
  }
  current_view = TOOLS_VIEW_MAIN;
  current_backlight = display_backlight_get_intensity();
  set_current_view_handler(tools_view_get_handler());
  lvgl_port_lock(0);
  tools_view = lv_obj_create(lv_screen_active());
  lv_obj_set_size(tools_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
  lv_obj_align(tools_view, LV_ALIGN_TOP_LEFT, 0, 20);
  lv_obj_add_style(tools_view, style_get_background_main(), LV_PART_MAIN);

  void *menu_labels[TOOLS_MENU_SIZE];
  uint16_t y = 5;
  for (size_t i = 0; i < TOOLS_MENU_SIZE; i++) {
    menu_labels[i] = lv_label_create(tools_view);
    lv_obj_align((lv_obj_t *)menu_labels[i], LV_ALIGN_TOP_LEFT, 5, y);
    lv_obj_add_style((lv_obj_t *)menu_labels[i], style_get_font_bigfont(),
                     LV_PART_MAIN);
    if (i == current_menu_item) {
      lv_obj_add_style((lv_obj_t *)menu_labels[i], style_get_font_highlight(),
                       LV_PART_MAIN);
    }
    lv_label_set_text((lv_obj_t *)menu_labels[i], tools_menu_texts[i]);
    y += 20;
  }
  lvgl_port_unlock();
}

void tools_view_init(void) {
  tools_view_handler.obj_view = tools_view;
  tools_view_handler.input_callback = tools_input_handler;
  tools_view_handler.draw_view = tools_view_draw;
  tools_view_handler.clear_view = tools_view_clear;
  current_menu_item = TOOLS_MENU_REBOOT;
  current_view = TOOLS_NOT_IN_VIEW;
}

view_handler_t *tools_view_get_handler(void) { return &tools_view_handler; }
