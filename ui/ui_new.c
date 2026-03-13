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
        return NULL;
    }
    return ((lv_obj_t **)&objects)[index];
}

void loadScreen(enum ScreensEnum screenId) {
    currentScreen = screenId - 1;
    lv_obj_t *screen = getLvglObjectFromIndex(currentScreen);
    if (screen) {
        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, false);
    }
}

void ui_init() {
    create_screens();
    loadScreen(SCREEN_ID_MAIN_VIEW);

    extern void on_start_kamera_clicked(lv_event_t * e);
    if (objects.open_camera) {
        lv_obj_add_event_cb(objects.open_camera, on_start_kamera_clicked, LV_EVENT_CLICKED, NULL);
    }

    /* Status box floating di sidebar */
    ui_stat_cpu = NULL;
    ui_stat_mem = NULL;
    ui_stat_gpu = NULL;

    if (objects.sidebar) {
        lv_obj_t *stats_box = lv_obj_create(objects.sidebar);
        lv_obj_add_flag(stats_box, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_set_size(stats_box, 220, 86);
        lv_obj_align(stats_box, LV_ALIGN_BOTTOM_LEFT, 12, -12);
        lv_obj_set_style_pad_all(stats_box, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_gap(stats_box, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(stats_box, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(stats_box, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(stats_box, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_move_foreground(stats_box);

        ui_stat_cpu = lv_label_create(stats_box);
        lv_label_set_text(ui_stat_cpu, "CPU: N/A");
        lv_obj_align(ui_stat_cpu, LV_ALIGN_TOP_LEFT, 0, 0);

        ui_stat_mem = lv_label_create(stats_box);
        lv_label_set_text(ui_stat_mem, "MEM: N/A");
        lv_obj_align(ui_stat_mem, LV_ALIGN_TOP_LEFT, 0, 24);

        ui_stat_gpu = lv_label_create(stats_box);
        lv_label_set_text(ui_stat_gpu, "GPU: N/A");
        lv_obj_align(ui_stat_gpu, LV_ALIGN_TOP_LEFT, 0, 48);
    }

    if (objects.main_view) {
        lv_obj_update_layout(objects.main_view);
    }
}

void ui_tick() {
    if (currentScreen >= 0) {
        tick_screen(currentScreen);
    }
}