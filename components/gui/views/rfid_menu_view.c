#include "rfid_menu_view.h"
#include "pn532.h"

static view_handler_t *calling_view;

static lv_obj_t *rfid_menu_view;
static view_handler_t rfid_menu_view_handler;

typedef enum {
  RFID_MENU_READ,

  RFID_MENU_SIZE
} rfid_menu_items_t;

static const char *rfid_menu_texts[RFID_MENU_SIZE] = {"READ"};

static rfid_menu_items_t current_menu_item;

typedef enum {
  NOT_IN_VIEW,
  RFID_VIEW_MAIN,
  RFID_VIEW_READ,
  RFID_VIEW_TAG,
  RFID_VIEW_DUMP,

  RFID_VIEWS_SIZE
} rfid_menu_views_t;

#define NB_KEYS 4
static const uint8_t mfc_keys_list[NB_KEYS][MFC_KEY_SIZE] = {
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5},
    {0xd3, 0xf7, 0xd3, 0xf7, 0xd3, 0xf7},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

static rfid_menu_views_t current_view = NOT_IN_VIEW;
;

static lv_obj_t *spinner;
static lv_obj_t *tag_info_lbl;

char dump_string[5000] = "";
char block_string[65] = "";

static void rfid_menu_draw_read_view(void);
static void rfid_menu_draw_tag_view(const pn532_record_t *record);
static void rfid_menu_draw_dump_view(const pn532_record_t *record);
static void rfid_menu_view_draw(view_handler_t *_calling_view);

static void rfid_menu_input_handler(user_actions_t user_action) {
  if (current_view == NOT_IN_VIEW) {
    return;
  }
  if (user_action == KEY_CLICK_SHORT) {
    if (current_view == RFID_VIEW_MAIN) {
      calling_view->draw_view(get_current_view_handler());
    } else {
      if (current_view == RFID_VIEW_READ) {
        pn532_cancel_read_task();
      }
      rfid_menu_view_draw(calling_view);
    }
  } else if (user_action == WHEEL_CLICK_SHORT) {
    if (current_view == RFID_VIEW_MAIN) {
      rfid_menu_draw_read_view();
    } else if (current_view == RFID_VIEW_TAG) {
      rfid_menu_draw_dump_view(pn532_get_last_record());
    }
  }
}

static void rfid_menu_draw_dump_view(const pn532_record_t *record) {
  if (current_view != RFID_VIEW_TAG) {
    return;
  }
  current_view = RFID_VIEW_DUMP;
  printf("Enter dump view\n");
  lvgl_port_lock(0);
  rfid_menu_view = lv_obj_create(lv_screen_active());
  lv_obj_set_size(rfid_menu_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
  lv_obj_align(rfid_menu_view, LV_ALIGN_TOP_LEFT, 0, 20);
  lv_obj_add_style(rfid_menu_view, get_background_style(), LV_PART_MAIN);

  uint8_t pn532_dump[MIFARE1K_SECTORS][MFC_BLOCK_BY_SECTOR][MFC_BLOCK_SIZE];
  size_t current_block = 0;
  printf("Start dump\n");
  for (size_t sectors = 0; sectors < MIFARE1K_SECTORS; sectors++) {
    int8_t keyA = -1;
    int8_t keyB = -1;
    // Try to authenticate with common keys
    for (size_t key = 0; key < NB_KEYS; key++) {
      // Try autheticate keyA
      if (keyA == -1) {
        if (pn532_mfc_authenticate_block(record->uid, record->uid_length,
                                         current_block, 0,
                                         mfc_keys_list[key]) == ESP_OK) {
          printf("%d: KeyA Authenticated:%d\n", current_block, key);
          keyA = key;
        }
      }
      // Try autheticate with keyB if keyA not found
      if (keyA == -1 && keyB == -1) {
        if (pn532_mfc_authenticate_block(record->uid, record->uid_length,
                                         current_block, 1,
                                         mfc_keys_list[key]) == ESP_OK) {
          // printf("%d: KeyB Authenticated:%d\n", current_block, key);
          keyB = key;
        }
      }
    }
    // Sector not authenticated go next don't try to read blocks
    if (keyA == -1 && keyB == -1) {
      printf("%d: Not authenticated\n", current_block);
      continue;
    } else {
      for (size_t block = 0; block < MFC_BLOCK_BY_SECTOR; block++) {
        if (pn532_mfc_read_data_block(current_block,
                                      pn532_dump[sectors][block]) == ESP_OK) {
          // trailer block need to override keyA since it's not readable
          if (block == 3 && keyA != -1) {
            for (size_t key_idx = 0; key_idx < MFC_KEY_SIZE; key_idx++) {
              pn532_dump[sectors][block][key_idx] =
                  mfc_keys_list[keyA][key_idx];
            }
          }
          sprintf(
              block_string,
              "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"
              "%02x",
              pn532_dump[sectors][block][0], pn532_dump[sectors][block][1],
              pn532_dump[sectors][block][2], pn532_dump[sectors][block][3],
              pn532_dump[sectors][block][4], pn532_dump[sectors][block][5],
              pn532_dump[sectors][block][6], pn532_dump[sectors][block][7],
              pn532_dump[sectors][block][8], pn532_dump[sectors][block][9],
              pn532_dump[sectors][block][10], pn532_dump[sectors][block][11],
              pn532_dump[sectors][block][12], pn532_dump[sectors][block][13],
              pn532_dump[sectors][block][14], pn532_dump[sectors][block][15]);
          strncat(dump_string, block_string, 32);
        }
        current_block++;
        strncat(dump_string, "\n ", 1);
      }
    }
    strncat(dump_string, "\n ", 1);
  }
  lv_obj_t *dump_lbl = lv_label_create(rfid_menu_view);
  lv_obj_align(dump_lbl, LV_ALIGN_TOP_LEFT, 5, 5);
  lv_label_set_text(dump_lbl, dump_string);
  lvgl_port_unlock();
  printf("End dump:%s\n", dump_string);
}

static void rfid_menu_draw_tag_view(const pn532_record_t *record) {
  if (current_view != RFID_VIEW_READ) {
    return;
  }
  current_view = RFID_VIEW_TAG;
  lvgl_port_lock(0);
  rfid_menu_view = lv_obj_create(lv_screen_active());
  lv_obj_set_size(rfid_menu_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
  lv_obj_align(rfid_menu_view, LV_ALIGN_TOP_LEFT, 0, 20);
  lv_obj_add_style(rfid_menu_view, get_background_style(), LV_PART_MAIN);

  tag_info_lbl = lv_label_create(rfid_menu_view);
  lv_obj_align(tag_info_lbl, LV_ALIGN_TOP_LEFT, 5, 5);
  char uid_buffer[PN532_UID_MAX_SIZE * 2] = "";
  pn532_get_last_uid_string(uid_buffer);
  char type_buffer[20] = "";
  pn532_get_last_type_string(type_buffer);
  lv_label_set_text_fmt(
      tag_info_lbl, "UID:\t%s\nATQA:\t%04x\nSAK:\t%02x\nType:\t%s",
      strupr(uid_buffer), record->ATQA, record->SAK, type_buffer);

  if (pn532_get_last_type() == TYPE_MIFARE_1K) {
    lv_obj_t *short_click_lbl = lv_label_create(rfid_menu_view);
    lv_obj_add_style(short_click_lbl, get_danger_style(), LV_PART_MAIN);
    lv_obj_align(short_click_lbl, LV_ALIGN_CENTER, 0, 60);
    lv_label_set_text(short_click_lbl, "Click wheel to try to read dump");
  }

  lvgl_port_unlock();
}

static void rfid_menu_draw_read_view() {
  current_view = RFID_VIEW_READ;
  lvgl_port_lock(0);
  rfid_menu_view = lv_obj_create(lv_screen_active());
  lv_obj_set_size(rfid_menu_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
  lv_obj_align(rfid_menu_view, LV_ALIGN_TOP_LEFT, 0, 20);
  lv_obj_add_style(rfid_menu_view, get_background_style(), LV_PART_MAIN);

  spinner = lv_spinner_create(rfid_menu_view);
  lv_obj_set_size(spinner, 50, 50);
  lv_obj_center(spinner);
  lv_obj_set_style_arc_color(spinner, lv_color_hex(TEXT_COLOR), 0);
  lv_obj_set_style_arc_color(spinner, lv_color_hex(DANGER_COLOR),
                             LV_PART_INDICATOR);
  lv_spinner_set_anim_params(spinner, 500, 200);

  lv_obj_t *read_label = lv_label_create(rfid_menu_view);
  lv_obj_align(read_label, LV_ALIGN_CENTER, 0, -60);
  lv_label_set_text(read_label, "Waiting for a tag");
  lvgl_port_unlock();
  pn532_background_read_passive_targetID(PN532_MIFARE_ISO14443A, 0,
                                         &rfid_menu_draw_tag_view);
}

static void rfid_menu_view_clear() {
  current_view = NOT_IN_VIEW;
  lvgl_port_lock(0);
  lv_obj_clean(rfid_menu_view);
  lvgl_port_unlock();
}

static void rfid_menu_view_draw(view_handler_t *_calling_view) {

  calling_view = _calling_view;

  if (calling_view != rfid_menu_view_get_handler()) {
    calling_view->clear_view();
  }

  set_current_view_handler(rfid_menu_view_get_handler());
  current_view = RFID_VIEW_MAIN;
  lvgl_port_lock(0);
  rfid_menu_view = lv_obj_create(lv_screen_active());
  lv_obj_set_size(rfid_menu_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
  lv_obj_align(rfid_menu_view, LV_ALIGN_TOP_LEFT, 0, 20);
  lv_obj_add_style(rfid_menu_view, get_background_style(), LV_PART_MAIN);

  void *menu_labels[RFID_MENU_SIZE];
  uint16_t y = 5;
  for (size_t i = 0; i < RFID_MENU_SIZE; i++) {
    menu_labels[i] = lv_label_create(rfid_menu_view);
    lv_obj_align((lv_obj_t *)menu_labels[i], LV_ALIGN_TOP_LEFT, 5, y);
    lv_obj_add_style((lv_obj_t *)menu_labels[i], get_bigfont_style(),
                     LV_PART_MAIN);
    if (i == current_menu_item) {
      lv_obj_add_style((lv_obj_t *)menu_labels[i], get_highlight_style(),
                       LV_PART_MAIN);
    }
    lv_label_set_text((lv_obj_t *)menu_labels[i], rfid_menu_texts[i]);
    y += 20;
  }
  lvgl_port_unlock();
}

void rfid_menu_view_init(void) {
  rfid_menu_view_handler.obj_view = rfid_menu_view;
  rfid_menu_view_handler.input_callback = rfid_menu_input_handler;
  rfid_menu_view_handler.draw_view = rfid_menu_view_draw;
  rfid_menu_view_handler.clear_view = rfid_menu_view_clear;
  current_menu_item = RFID_MENU_READ;
  current_view = NOT_IN_VIEW;
  spinner = NULL;
}

view_handler_t *rfid_menu_view_get_handler(void) {
  return &rfid_menu_view_handler;
}

/*
    uint32_t pn532_ver = pn532_get_firmware_version();
    printf("PN5%2x, Ver.%d.%d\n",
           (uint8_t)((pn532_ver >> 24) & 0xFF),
           (uint8_t)((pn532_ver >> 16) & 0xFF),
           (uint8_t)((pn532_ver >> 8) & 0xFF));
*/