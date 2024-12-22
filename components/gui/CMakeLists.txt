idf_component_register(
    SRCS "gui.c" 
         "style.c" 
         "view_commons.c" 
         views/splash_view.c 
         views/main_menu_view.c 
         views/wifi_menu_view.c
         views/scan_view.c
         views/rfid_menu_view.c
         views/tools_view.c
    INCLUDE_DIRS . views
    PRIV_REQUIRES display userinputs sd wifi clock battery wsl_bypasser pn532
    )

# Convert image to C Array usibng LVGL tools
lvgl_port_create_c_image("images_png/klaus_112x85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/klaus_dab_126x85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/klaus_afraid_104x85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/klaus_bored_102x85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/klaus_fuck_100x85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/battery_15.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/sd_15.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/charge_15.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/wifi_ok_15.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/wifi_nok_15.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/wifi_107x85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/scan_85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/rfid_85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/nowifi_85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/tools_85.png" "images_c/" "RGB565A8" "NONE")

# Add generated images to build
lvgl_port_add_images(${COMPONENT_LIB} "images_c/")