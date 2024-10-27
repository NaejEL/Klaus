idf_component_register(SRCS "keybtn.c" "knob.c" "display.c" "battery.c" "main.c" 
                    INCLUDE_DIRS ".")

# Convert image to C Array usibng LVGL tools
# lvgl_port_create_c_image("images_png/klaus_112x85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/klaus_dab_126x85.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/battery_15.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/sd_15.png" "images_c/" "RGB565A8" "NONE")
lvgl_port_create_c_image("images_png/charge_15.png" "images_c/" "RGB565A8" "NONE")

# Add generated images to build
lvgl_port_add_images(${COMPONENT_LIB} "images_c/")