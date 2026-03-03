#include "lvgl/lvgl.h"
#include "ui/ui.h"
#include <unistd.h>
#include <iostream>

int main(int argc, char **argv) {
    lv_init();

    lv_display_t * disp;
    lv_indev_t * indev;

#ifdef TARGET_RENESAS
    std::cout << "[INFO] Menjalankan mode Renesas (DRM + Evdev)..." << std::endl;
    
    // Inisialisasi Layar DRM
    disp = lv_linux_drm_create();
    // Default path GPU Renesas, ubah jika perlu (misal /dev/dri/card1)
    lv_linux_drm_set_file(disp, "/dev/dri/card0", -1); 

    // Inisialisasi Input Layar Sentuh / Mouse USB (Evdev)
    // PERHATIAN: Di board fisik, path event bisa berbeda (event0, event1, dst).
    indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0"); 
#else
    std::cout << "[INFO] Menjalankan mode PC Simulator (SDL2)..." << std::endl;
    
    // Inisialisasi Layar SDL
    disp = lv_sdl_window_create(1280, 720);
    indev = lv_sdl_mouse_create();
#endif

    // Inisialisasi UI
    ui_init(); 

    // Main Loop
    while(1) {
        lv_timer_handler();
        usleep(5000); 
    }

    return 0;
}