#include "lvgl/lvgl.h"
#include "ui/ui.h" // Include header utama hasil export SquareLine
#include <unistd.h>
#include <pthread.h>

// Fungsi Main Loop LVGL (perlu diberi nafas/detak)
int main(int argc, char **argv) {
    // 1. Inisialisasi LVGL Core
    lv_init();

    // 2. Inisialisasi Driver Simulator PC (SDL2)
    // Ukuran disesuaikan dengan desain SquareLine (800x480)
    lv_display_t * disp = lv_sdl_window_create(1280, 720);
    lv_indev_t * indev = lv_sdl_mouse_create();

    // 3. Inisialisasi UI hasil export SquareLine
    ui_init(); 

    // 4. Main Loop Aplikasi
    while(1) {
        lv_timer_handler(); // Memproses grafik, event, animasi
        usleep(5000);       // Istirahat 5ms agar CPU tidak 100%
    }

    return 0;
}