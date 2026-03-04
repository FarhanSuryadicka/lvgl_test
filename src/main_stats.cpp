#include "lvgl/lvgl.h"
#include "ui/ui.h"
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

    // Create labels to show stats
    lv_obj_t * label_cpu = lv_label_create(lv_scr_act());
    lv_label_set_text(label_cpu, "CPU: ...");
    lv_obj_align(label_cpu, LV_ALIGN_TOP_LEFT, 8, 8);

    lv_obj_t * label_mem = lv_label_create(lv_scr_act());
    lv_label_set_text(label_mem, "MEM: ...");
    lv_obj_align(label_mem, LV_ALIGN_TOP_LEFT, 8, 32);

    lv_obj_t * label_gpu = lv_label_create(lv_scr_act());
    lv_label_set_text(label_gpu, "GPU: ...");
    lv_obj_align(label_gpu, LV_ALIGN_TOP_LEFT, 8, 56);

    // Create LVGL timer to refresh labels from latest stats
    struct StatsLabelPtrs { lv_obj_t *cpu; lv_obj_t *mem; lv_obj_t *gpu; };
    auto *labels = new StatsLabelPtrs{label_cpu, label_mem, label_gpu};
    auto stats_timer_cb = [](lv_timer_t * timer){
        void * ud = lv_timer_get_user_data(timer);
        StatsLabelPtrs *p = static_cast<StatsLabelPtrs*>(ud);
        if(!p) return;
        SysStats s;
        if(read_process_stats(s)){
            char buf[128];
            // CPU (process) overall percent
            snprintf(buf, sizeof(buf), "CPU: %.2f%%", s.proc_cpu_percent);
            lv_label_set_text(p->cpu, buf);
            // Memory (process resident)
            if(s.proc_rss_kb>0){
                snprintf(buf, sizeof(buf), "MEM: %ld MB", s.proc_rss_kb/1024);
            } else {
                snprintf(buf, sizeof(buf), "MEM: N/A");
            }
            lv_label_set_text(p->mem, buf);
            // GPU utilization (device-wide) as percent
            if(s.gpu_util_percent >= 0){
                snprintf(buf, sizeof(buf), "GPU: %.0f%%", s.gpu_util_percent);
            } else {
                snprintf(buf, sizeof(buf), "GPU: N/A");
            }
            lv_label_set_text(p->gpu, buf);
        }
    };

    lv_timer_t * t = lv_timer_create((lv_timer_cb_t)stats_timer_cb, 500, labels);

    // Main Loop
    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    // never reached
    return 0;
}
