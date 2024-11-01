#include "view_commons.h"

static view_handler_t* current_view_handler;

view_handler_t* get_current_view_handler(void){
    return current_view_handler;
}

void set_current_view_handler(view_handler_t* view_handler){
    current_view_handler = view_handler;
}