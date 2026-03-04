#ifndef EEZ_LVGL_UI_GUI_H
#define EEZ_LVGL_UI_GUI_H

#include <lvgl/lvgl.h>

#include "screens.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_init();
void ui_tick();

void loadScreen(enum ScreensEnum screenId);

/* Status labels exposed for the application to update realtime stats */
extern lv_obj_t * ui_stat_cpu;
extern lv_obj_t * ui_stat_mem;
extern lv_obj_t * ui_stat_gpu;

#ifdef __cplusplus
}
#endif

#endif // EEZ_LVGL_UI_GUI_H