#include "lvgl/lvgl.h"
#include "ui/ui_new.h"
#include "platform_stats.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iostream>
#include <unistd.h>

extern double g_camera_fps_ema;

#ifndef TARGET_RENESAS
#include <SDL2/SDL.h>
extern "C" void ui_anypoint_mousewheel(int mouse_x, int mouse_y, int wheel_y);

static int sdl_mousewheel_bridge(void *userdata, SDL_Event *event) {
    (void)userdata;

    if(event && event->type == SDL_MOUSEWHEEL) {
        int mx = 0;
        int my = 0;
        SDL_GetMouseState(&mx, &my);
        ui_anypoint_mousewheel(mx, my, event->wheel.y);
    }
    return 0;
}
#endif

int main(int argc, char **argv) {
    lv_init();

    lv_obj_t * disp;
    lv_indev_t * indev;

#ifdef TARGET_RENESAS
    std::cout << "[INFO] Menjalankan mode Renesas (DRM + Evdev)..." << std::endl;

    disp = (lv_obj_t*)lv_linux_drm_create();
    lv_linux_drm_set_file((lv_display_t*)disp, "/dev/dri/card0", -1);

    indev = (lv_indev_t*)lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event0");
#else
    std::cout << "[INFO] Menjalankan mode PC Simulator (SDL2)..." << std::endl;

    disp = (lv_obj_t*)lv_sdl_window_create(1920, 1080);
    indev = (lv_indev_t*)lv_sdl_mouse_create();
#endif

    ui_init();

#ifndef TARGET_RENESAS
    SDL_AddEventWatch(sdl_mousewheel_bridge, NULL);
#endif

    auto stats_timer_cb = [](lv_timer_t * timer){
        LV_UNUSED(timer);

        SysStats s;
        if(read_process_stats(s)){
            char buf[128];

            if(ui_stat_cpu) {
                std::snprintf(buf, sizeof(buf), "CPU: %.2f%%", s.proc_cpu_percent);
                lv_label_set_text(ui_stat_cpu, buf);
            }

            if(ui_stat_mem) {
                if(s.proc_rss_kb > 0) {
                    std::snprintf(buf, sizeof(buf), "MEM: %ld MB", s.proc_rss_kb / 1024);
                } else {
                    std::snprintf(buf, sizeof(buf), "MEM: N/A");
                }
                lv_label_set_text(ui_stat_mem, buf);
            }

            if(ui_stat_gpu) {
                if(s.gpu_util_percent >= 0) {
                    std::snprintf(buf, sizeof(buf), "GPU: %.1f%%", s.gpu_util_percent);
                } else {
                    std::snprintf(buf, sizeof(buf), "GPU: N/A");
                }
                lv_label_set_text(ui_stat_gpu, buf);
            }
        }
    };

    lv_timer_t * t = lv_timer_create((lv_timer_cb_t)stats_timer_cb, 500, NULL);
    (void)t;
    (void)disp;
    (void)indev;
    (void)argc;
    (void)argv;

    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

    return 0;
}