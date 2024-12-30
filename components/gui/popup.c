#include "popup.h"

#include "string.h"
#include "style.h"
#include "view_commons.h"

static lv_obj_t *popup_container;
static lv_obj_t *popup_background;

static popup_config_t current_config;
static popup_content_t content;

#define POPUP_VIEW_WIDTH 300
#define POPUP_VIEW_HEIGHT 130

#define ASCII_START 0x21
#define START_CHAR 0x41
#define ASCII_END 0x7E

#define TEXT_CHUNK 16
static char *tmp_content = NULL;
static size_t text_size = 0;
static char keyboard_char = START_CHAR;

static lv_obj_t *keyboard_span_container;
static lv_span_t *keyboard_span;

static size_t current_item;

static void popup_clear_view() {
  current_config.type = POPUP_NOT_USED;
  lvgl_port_lock(0);
  lv_obj_clean(popup_container);
  lvgl_port_unlock();
}

static void popup_refresh() {
  lvgl_port_lock(0);

  uint8_t y_offset = 0;
  if (current_config.title != NULL) {
    lv_obj_t *title_bar = lv_obj_create(popup_background);
    lv_obj_set_size(title_bar, POPUP_VIEW_WIDTH, 25);
    lv_obj_align(title_bar, LV_ALIGN_TOP_MID, 0, -2);
    lv_obj_add_style(title_bar, style_get_background_title_bar(), LV_PART_MAIN);
    lv_obj_t *title_label = lv_label_create(title_bar);
    lv_obj_add_style(title_label, style_get_font_bigfont(), LV_PART_MAIN);
    if (current_config.type == POPUP_ALERT) {
      lv_obj_add_style(title_label, style_get_font_alt_highlight(),
                       LV_PART_MAIN);
    }
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 2);
    lv_label_set_text(title_label, current_config.title);
    y_offset += 30;
  }
  if (current_config.type == POPUP_KEYBOARD) {
    if (keyboard_span_container != NULL) {
      lv_spangroup_delete_span(keyboard_span_container, keyboard_span);
      keyboard_span_container = NULL;
    }
    keyboard_span_container = lv_spangroup_create(popup_background);
    lv_obj_set_width(keyboard_span_container, POPUP_VIEW_WIDTH - 20);
    lv_obj_set_height(keyboard_span_container, POPUP_VIEW_HEIGHT - 20);
    lv_obj_align(keyboard_span_container, LV_ALIGN_TOP_LEFT, 5, y_offset);

    lv_obj_add_style(keyboard_span_container,
                     style_get_background_transparent(), LV_PART_MAIN);
    lv_spangroup_set_align(keyboard_span_container, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(keyboard_span_container, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_indent(keyboard_span_container, 0);
    lv_spangroup_set_mode(keyboard_span_container, LV_SPAN_MODE_BREAK);

    keyboard_span = lv_spangroup_new_span(keyboard_span_container);
    lv_span_set_text(keyboard_span, content.content);
    lv_style_set_text_color(lv_span_get_style(keyboard_span),
                            lv_color_hex(TEXT_COLOR));

    keyboard_span = lv_spangroup_new_span(keyboard_span_container);
    char cur_char_buffer[2];
    cur_char_buffer[0] = (char)keyboard_char;
    cur_char_buffer[1] = '\0';
    lv_span_set_text(keyboard_span, cur_char_buffer);
    lv_style_set_text_color(lv_span_get_style(keyboard_span),
                            lv_color_hex(HIGHLIGHT_COLOR));

    lv_spangroup_refr_mode(keyboard_span_container);
  } else if (current_config.type == POPUP_ALERT ||
             current_config.type == POPUP_INFO) {
    lv_obj_t *popup_label = lv_label_create(popup_background);
    lv_obj_align(popup_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(popup_label, current_config.content);
  } else if (current_config.type == POPUP_MENU) {
    void *menu_labels[current_config.nb_items];
    for (size_t i = 0; i < current_config.nb_items; i++) {
      menu_labels[i] = lv_label_create(popup_background);
      lv_obj_align((lv_obj_t *)menu_labels[i], LV_ALIGN_TOP_LEFT, 5, y_offset);
      lv_label_set_text((lv_obj_t *)menu_labels[i], current_config.items[i]);
      if (i == current_item) {
        lv_obj_add_style((lv_obj_t *)menu_labels[i], style_get_font_highlight(),
                         LV_PART_MAIN);
      }
      y_offset += 10;
    }
  }

  lvgl_port_unlock();
}

popup_types_t popup_get_current_type() { return current_config.type; }

void popup_start(popup_config_t *config) {
  current_config = *config;
  if (current_config.type == POPUP_KEYBOARD) {
    if (content.content != NULL) {
      free(content.content);
    }
    text_size = 0;
    content.content = NULL;
    content.content_index = 0;
    keyboard_char = START_CHAR;
  } else if (current_config.type == POPUP_MENU) {
    current_item = 0;
  }
  content.type = current_config.type;
  lvgl_port_lock(0);
  popup_container = lv_obj_create(lv_screen_active());
  lv_obj_set_size(popup_container, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
  lv_obj_align(popup_container, LV_ALIGN_CENTER, 0, 10);
  lv_obj_add_style(popup_container, style_get_background_transparent(),
                   LV_PART_MAIN);

  popup_background = lv_obj_create(popup_container);
  lv_obj_set_size(popup_background, POPUP_VIEW_WIDTH, POPUP_VIEW_HEIGHT);
  lv_obj_align(popup_background, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(popup_background, style_get_background_popup(),
                   LV_PART_MAIN);
  lv_obj_add_style(popup_background, style_get_background_danger(),
                   LV_PART_SCROLLBAR);

  keyboard_span_container = NULL;
  lvgl_port_unlock();
  popup_refresh();
}

void popup_input(user_actions_t action) {
  if (action == KEY_CLICK_SHORT) {
    content.user_validation = false;
    content.content_index = current_item;
    popup_clear_view();
    if (current_config.callback != NULL) {
      current_config.callback(&content);
    }
    return;
  } else if (action == WHEEL_CLICK_LONG) {
    content.user_validation = true;
    content.content_index = current_item;
    popup_clear_view();
    if (current_config.callback != NULL) {
      current_config.callback(&content);
    }
    return;
  } else if (action == WHEEL_UP) {
    if (current_config.type == POPUP_KEYBOARD) {
      if (keyboard_char < ASCII_END) {
        keyboard_char++;
      } else {
        keyboard_char = ASCII_START;
      }
    } else if (current_config.type == POPUP_MENU) {
      if (current_item < current_config.nb_items - 1) {
        current_item++;
      } else {
        current_item = 0;
      }
    }
  } else if (action == WHEEL_DOWN) {
    if (current_config.type == POPUP_KEYBOARD) {
      if (keyboard_char > ASCII_START) {
        keyboard_char--;
      } else {
        keyboard_char = ASCII_END;
      }
    } else if (current_config.type == POPUP_MENU) {
      if (current_item > 0) {
        current_item--;
      } else {
        current_item = current_config.nb_items - 1;
      }
    }
  } else if (action == WHEEL_CLICK_SHORT) {
    if (current_config.type == POPUP_KEYBOARD) {
      if (text_size <= content.content_index + 1) {
        text_size += TEXT_CHUNK;
        tmp_content = realloc(content.content, text_size);
        content.content = tmp_content;
      }
      content.content[content.content_index++] = (char)keyboard_char;
      content.content[content.content_index] = '\0';
    } else {
      content.user_validation = true;
      content.content_index = current_item;
      popup_clear_view();
      if (current_config.callback != NULL) {
      current_config.callback(&content);
    }
      return;
    }
  }
  popup_refresh();
}
