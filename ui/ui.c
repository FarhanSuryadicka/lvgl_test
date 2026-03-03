#include "ui.h"

// Mendefinisikan variabel globalnya di sini
lv_obj_t * ui_img_video;
lv_obj_t * ui_lbl_fps;

void ui_init(void) {
    // Ambil layar aktif saat ini
    lv_obj_t * screen = lv_screen_active();

    // 1. Membuat kotak abu-abu di atas (Mensimulasikan tempat video)
    ui_img_video = lv_image_create(screen);
    lv_obj_set_size(ui_img_video, 640, 480); // Ukuran rasio 16:9
    lv_obj_align(ui_img_video, LV_ALIGN_TOP_MID, 0, 20); // Posisi tengah atas

    // 2. Membuat Tombol di bawah kotak video
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_set_size(btn, 150, 50);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20); // Posisi tengah bawah

    // 3. Menambahkan Teks ke dalam tombol
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Mulai Kamera");
    lv_obj_center(label);

    // 4b. Label FPS (top-right)
    ui_lbl_fps = lv_label_create(screen);
    lv_label_set_text(ui_lbl_fps, "FPS: 0.0");
    lv_obj_align(ui_lbl_fps, LV_ALIGN_TOP_RIGHT, -10, 10);

    // 4. MENGHUBUNGKAN TOMBOL DENGAN FUNGSI C++ ANDA!
    lv_obj_add_event_cb(btn, on_start_kamera_clicked, LV_EVENT_CLICKED, NULL);
}
