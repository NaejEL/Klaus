#pragma once

// LVGL
#include "esp_lvgl_port.h"

#include "gui_commons.h"

lv_obj_t *main_menu_label;

void main_menu_view_draw()
{
    lvgl_port_lock(0);
    if(current_view!=NULL){
        lv_obj_clean(current_view);
    }
    active_view = MAIN_MENU_VIEW;
    current_view = main_menu_view;
    main_menu_view = lv_obj_create(lv_screen_active());
    lv_obj_set_size(main_menu_view, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT);
    lv_obj_align(main_menu_view, LV_ALIGN_TOP_LEFT, 0, 20);

    main_menu_label = lv_label_create(main_menu_view);
    lv_obj_align(main_menu_label, LV_ALIGN_CENTER, 50, 0);
    lv_label_set_text(time_label, "TEST");
    printf("Main menu view\n");
    lvgl_port_unlock();
}