#pragma once

#include "stddef.h"
#include "userinputs.h"

typedef struct popup_content_t popup_content_t;
typedef struct popup_config_t popup_config_t;

typedef void (*popup_callback)(popup_content_t *content);

typedef enum {
  POPUP_NOT_USED,
  POPUP_INFO,
  POPUP_ALERT,
  POPUP_KEYBOARD,
  POPUP_MENU,
  POPUP_TYPES_SIZE
} popup_types_t;

struct popup_config_t {
  popup_types_t type;
  char *title;  // Null if none
  char *content; // Null if none
  char **items; // Null if none
  size_t nb_items;
  popup_callback callback;
};

struct popup_content_t {
  popup_types_t type;
  char *content;        // Only useful for keyboard type
  size_t content_index; // text size or item selected index
  bool user_validation; // True if wheelclick, false if return
};

void popup_start(popup_config_t *config);

popup_types_t popup_get_current_type(void);

void popup_input(user_actions_t action);