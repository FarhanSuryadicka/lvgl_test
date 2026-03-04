#include "ui_new.h"
#include "screens.h"
#include "images.h"
#include "actions.h"
#include "vars.h"

#include <string.h>

static int16_t currentScreen = -1;

/* Definitions for status labels (defined here so they have storage) */
lv_obj_t * ui_stat_cpu = NULL;
lv_obj_t * ui_stat_mem = NULL;
lv_obj_t * ui_stat_gpu = NULL;

static lv_obj_t *getLvglObjectFromIndex(int32_t index) {
    if (index == -1) {
        return 0;
    }
    return ((lv_obj_t **)&objects)[index];
}

void loadScreen(enum ScreensEnum screenId) {
    currentScreen = screenId - 1;
    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
}

void ui_init() {
    create_screens();
    loadScreen(SCREEN_ID_MAIN_VIEW);
    /* Attach event handler for the generated "open_camera" button so the
       existing camera callback (on_start_kamera_clicked) is called when the
       button is pressed. The callback is defined in ui_events.cpp. */
    extern void on_start_kamera_clicked(lv_event_t * e);
    if(objects.open_camera) {
        lv_obj_add_event_cb(objects.open_camera, on_start_kamera_clicked, LV_EVENT_CLICKED, NULL);
    }

    /* Create status labels (CPU / MEM / GPU) inside the sidebar so they
       appear in the generated layout. These are exposed via externs. */
    ui_stat_cpu = NULL; ui_stat_mem = NULL; ui_stat_gpu = NULL;
    if(objects.sidebar) {
        ui_stat_cpu = lv_label_create(objects.sidebar);
        lv_label_set_text(ui_stat_cpu, "CPU: N/A");
        lv_obj_align(ui_stat_cpu, LV_ALIGN_TOP_LEFT, 8, 8);

        ui_stat_mem = lv_label_create(objects.sidebar);
        lv_label_set_text(ui_stat_mem, "MEM: N/A");
        lv_obj_align(ui_stat_mem, LV_ALIGN_TOP_LEFT, 8, 32);

        ui_stat_gpu = lv_label_create(objects.sidebar);
        lv_label_set_text(ui_stat_gpu, "GPU: N/A");
        lv_obj_align(ui_stat_gpu, LV_ALIGN_TOP_LEFT, 8, 56);
    }
}

void ui_tick() {
    tick_screen(currentScreen);
}