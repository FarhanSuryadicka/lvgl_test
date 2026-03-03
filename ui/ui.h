#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

// Deklarasi variabel global untuk widget kotak video
extern lv_obj_t * ui_img_video;
// Label untuk menampilkan FPS kamera secara real-time
extern lv_obj_t * ui_lbl_fps;

// Deklarasi fungsi inisialisasi UI
void ui_init(void);

// Deklarasi event handler yang ada di ui_events.cpp Anda
void on_start_kamera_clicked(lv_event_t * e);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
