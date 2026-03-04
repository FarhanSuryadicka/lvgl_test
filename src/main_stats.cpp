#include "lvgl/lvgl.h"
#include "ui/ui_new.h"
#include "platform_stats.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iostream>
#include <unistd.h>

// No background thread required: we sample process stats on each LVGL timer tick.

int main(int argc, char **argv) {
    lv_init();

    lv_obj_t * disp;
    lv_indev_t * indev;

#ifdef TARGET_RENESAS
    std::cout << "[INFO] Menjalankan mode Renesas (DRM + Evdev)..." << std::endl;
    
    // Inisialisasi Layar DRM (platform-specific helper assumed available)
    disp = (lv_obj_t*)lv_linux_drm_create();
    lv_linux_drm_set_file((lv_display_t*)disp, "/dev/dri/card0", -1);

    indev = (lv_indev_t*)lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
#else
    std::cout << "[INFO] Menjalankan mode PC Simulator (SDL2)..." << std::endl;
    
    disp = (lv_obj_t*)lv_sdl_window_create(1280, 720);
    indev = (lv_indev_t*)lv_sdl_mouse_create();
#endif

    // Inisialisasi UI
    ui_init();

    // Create LVGL timer to refresh labels from latest stats.
    // The labels are created by the generated UI (`ui_new.c`) and exposed as
    // `ui_stat_cpu`, `ui_stat_mem`, `ui_stat_gpu`.
    auto stats_timer_cb = [](lv_timer_t * timer){
        SysStats s;
        if(read_process_stats(s)){
            char buf[128];
            // CPU (process) overall percent
            if(ui_stat_cpu) {
                snprintf(buf, sizeof(buf), "CPU: %.2f%%", s.proc_cpu_percent);
                lv_label_set_text(ui_stat_cpu, buf);
            }
            // Memory (process resident)
            if(ui_stat_mem) {
                if(s.proc_rss_kb>0){
                    snprintf(buf, sizeof(buf), "MEM: %ld MB", s.proc_rss_kb/1024);
                } else {
                    snprintf(buf, sizeof(buf), "MEM: N/A");
                }
                lv_label_set_text(ui_stat_mem, buf);
            }
            // GPU utilization estimate per-process
            if(ui_stat_gpu) {
                if(s.gpu_util_percent >= 0){
                    snprintf(buf, sizeof(buf), "GPU: %.1f%%", s.gpu_util_percent);
                } else {
                    snprintf(buf, sizeof(buf), "GPU: N/A");
                }
                lv_label_set_text(ui_stat_gpu, buf);
            }
        }
    };

    lv_timer_t * t = lv_timer_create((lv_timer_cb_t)stats_timer_cb, 1000, NULL);

    // Main Loop
    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    // never reached
    return 0;
}
