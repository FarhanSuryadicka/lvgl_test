#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN_VIEW = 1,
    _SCREEN_ID_LAST = 1
};

typedef struct _objects_t {
    lv_obj_t *main_view;
    lv_obj_t *main_panel;
    lv_obj_t *panorama;
    lv_obj_t *panorama_image;
    lv_obj_t *anypoint;
    lv_obj_t *anypoint_1;
    lv_obj_t *anypoint_1_image;
    lv_obj_t *anypoint_2;
    lv_obj_t *anypoint_2_image;
    lv_obj_t *anypoint_3;
    lv_obj_t *anypoint_3_image;
    lv_obj_t *anypoint_4;
    lv_obj_t *anypoint_4_image;
    lv_obj_t *sidebar;
    lv_obj_t *open_camera;
    lv_obj_t *moil_image;
    lv_obj_t *mode_panel;
    lv_obj_t *btn_original;
    lv_obj_t *btn_panorama;
    lv_obj_t *btn_anypoint;
    lv_obj_t *config;
    lv_obj_t *alpha_conf;
    lv_obj_t *beta_conf;
    lv_obj_t *zoom_conf;
} objects_t;

extern objects_t objects;

void create_screen_main_view();
void tick_screen_main_view();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/