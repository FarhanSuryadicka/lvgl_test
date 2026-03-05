#include "ui_new.h"
#include "screens.h"
#include "lvgl/lvgl.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdio>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "moil/moildev.hpp"

/* GStreamer capture (appsink) */
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

// =====================================================================
// Variabel Global & GStreamer
// =====================================================================
bool is_camera_running = false;
lv_timer_t * camera_timer = NULL;

static GstElement * gst_pipeline = nullptr;
static GstElement * gst_appsink = nullptr;

// =====================================================================
// Image Descriptors & Buffers
// =====================================================================
static lv_image_dsc_t dsc_pano = {0};
static std::vector<uint16_t> buf_pano;
static lv_image_dsc_t dsc_any1 = {0};
static std::vector<uint16_t> buf_any1;
static lv_image_dsc_t dsc_any2 = {0};
static std::vector<uint16_t> buf_any2;
static lv_image_dsc_t dsc_any3 = {0};
static std::vector<uint16_t> buf_any3;
static lv_image_dsc_t dsc_any4 = {0};
static std::vector<uint16_t> buf_any4;
static lv_image_dsc_t dsc_moil = {0};
static std::vector<uint16_t> buf_moil;

// Local FPS label
static lv_obj_t * ui_lbl_fps_local = NULL;

// =====================================================================
// Moildev Engine & Maps (High-Res & Low-Res)
// =====================================================================
static moildev::Moildev* moil = nullptr;
static bool maps_initialized = false;

// Map untuk Layar Utama (1600x1200)
static cv::Mat mapX_pano, mapY_pano;
static cv::Mat mapX_any1, mapY_any1;
static cv::Mat mapX_any2, mapY_any2;
static cv::Mat mapX_any3, mapY_any3;
static cv::Mat mapX_any4, mapY_any4;

// Map untuk Thumbnail (320x240) - Rahasia Performa 30FPS Realtime
static cv::Mat mapX_pano_thumb, mapY_pano_thumb;
static cv::Mat mapX_any1_thumb, mapY_any1_thumb;
static cv::Mat mapX_any2_thumb, mapY_any2_thumb;
static cv::Mat mapX_any3_thumb, mapY_any3_thumb;
static cv::Mat mapX_any4_thumb, mapY_any4_thumb;

// Variabel Penyimpan Parameter Saat Ini
// Panorama: alpha -> alpha_max, beta -> alpha_min, zoom -> beta_degree
static float pano_alpha_max = 110.0f, pano_alpha_min = 0.0f, pano_beta_deg = 0.0f;
// Anypoint: Tetap alpha, beta, zoom
static float any1_alpha = 5.0f, any1_beta = 0.0f, any1_zoom = 3.0f;

// =====================================================================
// Fungsi Helper: Generate Map secara Dinamis dengan Target Resolusi
// =====================================================================
static void generate_scaled_map(float val1, float val2, float val3, int mode, cv::Mat& outX, cv::Mat& outY, float targetW, float targetH) {
    if(!moil) return;
    
    float imgW = 2592.0f, imgH = 1944.0f;
    float src_w = 1600.0f, src_h = 1200.0f;

    cv::Mat mX(imgH, imgW, CV_32F);
    cv::Mat mY(imgH, imgW, CV_32F);

    if (mode == 0) { 
        moil->AnyPointM((float*)mX.data, (float*)mY.data, val1, val2, val3);
    } else { 
        moil->PanoramaCar((float*)mX.data, (float*)mY.data, val1, val2, val3, false, false);
    }

    // Scale koordinat agar menunjuk ke piksel yang benar di raw_frame (1600x1200)
    mX *= (src_w / imgW);
    mY *= (src_h / imgH);

    // Resize ukuran grid grid map ke target (Main View atau Thumbnail)
    cv::resize(mX, outX, cv::Size(targetW, targetH));
    cv::resize(mY, outY, cv::Size(targetW, targetH));
}

// =====================================================================
// Event Handler: Checkbox Radio Button & UI Config
// =====================================================================
static void mode_checkbox_event_cb(lv_event_t * e) {
    lv_obj_t * target = (lv_obj_t *)lv_event_get_target(e);
    
    if(lv_obj_has_state(target, LV_STATE_CHECKED)) {
        // Hilangkan centang mode lain
        if(target != objects.btn_original) lv_obj_remove_state(objects.btn_original, LV_STATE_CHECKED);
        if(target != objects.btn_panorama) lv_obj_remove_state(objects.btn_panorama, LV_STATE_CHECKED);
        if(target != objects.btn_anypoint) lv_obj_remove_state(objects.btn_anypoint, LV_STATE_CHECKED);
        
        if(target == objects.btn_panorama) {
            lv_obj_clear_flag(objects.config, LV_OBJ_FLAG_HIDDEN); // Tampilkan Panel Config
            
            lv_label_set_text(lv_obj_get_child(objects.config, 1), "Alpha Max");
            lv_label_set_text(lv_obj_get_child(objects.config, 3), "Alpha Min");
            lv_label_set_text(lv_obj_get_child(objects.config, 5), "Beta");

            lv_slider_set_range(objects.alpha_conf, 10, 110);
            lv_slider_set_range(objects.beta_conf, 0, 110);
            lv_slider_set_range(objects.zoom_conf, -180, 180);
            
            lv_slider_set_value(objects.alpha_conf, (int)pano_alpha_max, LV_ANIM_OFF);
            lv_slider_set_value(objects.beta_conf, (int)pano_alpha_min, LV_ANIM_OFF);
            lv_slider_set_value(objects.zoom_conf, (int)pano_beta_deg, LV_ANIM_OFF);
        } 
        else if(target == objects.btn_anypoint) {
            lv_obj_clear_flag(objects.config, LV_OBJ_FLAG_HIDDEN); // Tampilkan Panel Config
            
            lv_label_set_text(lv_obj_get_child(objects.config, 1), "Alpha");
            lv_label_set_text(lv_obj_get_child(objects.config, 3), "Beta");
            lv_label_set_text(lv_obj_get_child(objects.config, 5), "Zoom");

            lv_slider_set_range(objects.alpha_conf, 0, 110);
            lv_slider_set_range(objects.beta_conf, -180, 180);
            lv_slider_set_range(objects.zoom_conf, 10, 140); 
            
            lv_slider_set_value(objects.alpha_conf, (int)any1_alpha, LV_ANIM_OFF);
            lv_slider_set_value(objects.beta_conf, (int)any1_beta, LV_ANIM_OFF);
            lv_slider_set_value(objects.zoom_conf, (int)(any1_zoom * 10), LV_ANIM_OFF);
        }
        else if(target == objects.btn_original) {
            lv_obj_add_flag(objects.config, LV_OBJ_FLAG_HIDDEN); // Sembunyikan Panel Config
        }
    } else {
        if(target != objects.btn_original) {
            lv_obj_add_state(objects.btn_original, LV_STATE_CHECKED);
            lv_obj_add_flag(objects.config, LV_OBJ_FLAG_HIDDEN); 
        }
    }
}

// =====================================================================
// Event Handler: Geseran Slider (Re-generate High & Low Res Map)
// =====================================================================
static void slider_event_cb(lv_event_t * e) {
    if(!maps_initialized) return;

    int alpha_val = lv_slider_get_value(objects.alpha_conf);
    int beta_val  = lv_slider_get_value(objects.beta_conf);
    int zoom_val  = lv_slider_get_value(objects.zoom_conf);

    if(lv_obj_has_state(objects.btn_panorama, LV_STATE_CHECKED)) {
        pano_alpha_max = (float)alpha_val;
        pano_alpha_min = (float)beta_val;
        pano_beta_deg  = (float)zoom_val;
        
        // Generate High-Res (Main) & Low-Res (Thumb)
        generate_scaled_map(pano_alpha_max, pano_alpha_min, pano_beta_deg, 1, mapX_pano, mapY_pano, 1600, 1200);
        generate_scaled_map(pano_alpha_max, pano_alpha_min, pano_beta_deg, 1, mapX_pano_thumb, mapY_pano_thumb, 320, 240);
    } 
    else if(lv_obj_has_state(objects.btn_anypoint, LV_STATE_CHECKED)) {
        any1_alpha = (float)alpha_val;
        any1_beta  = (float)beta_val;
        any1_zoom  = (float)zoom_val / 10.0f;
        
        // Generate High-Res (Main) & Low-Res (Thumb)
        generate_scaled_map(any1_alpha, any1_beta, any1_zoom, 0, mapX_any1, mapY_any1, 1600, 1200);
        generate_scaled_map(any1_alpha, any1_beta, any1_zoom, 0, mapX_any1_thumb, mapY_any1_thumb, 320, 240);
    }
}

// =====================================================================
// Inisialisasi Moildev dan Generate Seluruh Map
// =====================================================================
void init_moildev_maps() {
    if (maps_initialized) return;

    float sensorW = 1.0f, sensorH = 1.0f;
    float iCx = 1238.0f, iCy = 987.0f, ratio = 1.0f, imgW = 2592.0f, imgH = 1944.0f, calib = 1.0f;
    float p0 = 0.0f, p1 = 0.0f, p2 = -32.973f, p3 = 67.825f, p4 = -41.581f, p5 = 504.7f;

    moil = new moildev::Moildev(sensorW, sensorH, iCx, iCy, ratio, imgW, imgH, calib, p0, p1, p2, p3, p4, p5);

    std::cout << "[INFO] Membuat Maps Moildev (High-Res & Low-Res)..." << std::endl;
    
    // 1. Map Layar Utama (1600x1200)
    generate_scaled_map(pano_alpha_max, pano_alpha_min, pano_beta_deg, 1, mapX_pano, mapY_pano, 1600, 1200);
    generate_scaled_map(any1_alpha, any1_beta, any1_zoom, 0, mapX_any1, mapY_any1, 1600, 1200);
    generate_scaled_map(45.0f, 2.0f, 4.0f, 0, mapX_any2, mapY_any2, 1600, 1200);
    generate_scaled_map(40.0f, -175.0f, 4.0f, 0, mapX_any3, mapY_any3, 1600, 1200);
    generate_scaled_map(50.0f, -180.0f, 4.5f, 0, mapX_any4, mapY_any4, 1600, 1200);

    // 2. Map Thumbnail (320x240) - Untuk Performa Optimal Renesas
    int t_w = 320, t_h = 240;
    generate_scaled_map(pano_alpha_max, pano_alpha_min, pano_beta_deg, 1, mapX_pano_thumb, mapY_pano_thumb, t_w, t_h);
    generate_scaled_map(any1_alpha, any1_beta, any1_zoom, 0, mapX_any1_thumb, mapY_any1_thumb, t_w, t_h);
    generate_scaled_map(45.0f, 2.0f, 4.0f, 0, mapX_any2_thumb, mapY_any2_thumb, t_w, t_h);
    generate_scaled_map(40.0f, -175.0f, 4.0f, 0, mapX_any3_thumb, mapY_any3_thumb, t_w, t_h);
    generate_scaled_map(50.0f, -180.0f, 4.5f, 0, mapX_any4_thumb, mapY_any4_thumb, t_w, t_h);
    
    maps_initialized = true;
    std::cout << "[INFO] Maps Moildev selesai dibuat." << std::endl;
}

// =====================================================================
// Callback Timer (Render Loop) - OPTIMIZED REALTIME FOR ALL DEVICES
// =====================================================================
static void camera_stream_cb(lv_timer_t * timer) {
    if(!is_camera_running || gst_appsink == nullptr) return;

    GstSample * sample = gst_app_sink_try_pull_sample(GST_APP_SINK(gst_appsink), 10000000);
    if(!sample) return;

    GstBuffer * buf = gst_sample_get_buffer(sample);
    GstCaps * caps = gst_sample_get_caps(sample);
    if(!buf || !caps) {
        if(sample) gst_sample_unref(sample);
        return;
    }

    GstStructure * str = gst_caps_get_structure(caps, 0);
    int w = 1600, h = 1200;
    gst_structure_get_int(str, "width", &w);
    gst_structure_get_int(str, "height", &h);

    GstMapInfo map;
    if(!gst_buffer_map(buf, &map, GST_MAP_READ)) {
        gst_sample_unref(sample);
        return;
    }

    cv::Mat raw_frame(h, w, CV_8UC3, map.data);

    // [OPTIMASI 1] Fungsi Render Cepat OpenCV (SIMD)
    auto render_to_lvgl = [&](const cv::Mat& src, std::vector<uint16_t>& dst_buf, lv_image_dsc_t& dsc, lv_obj_t* widget) {
        if(!widget) return;
        
        int tw = lv_obj_get_width(widget);
        int th = lv_obj_get_height(widget);
        if(tw <= 0 || th <= 0) { tw = 128; th = 128; } 

        cv::Mat resized;
        if(src.cols != tw || src.rows != th) {
            cv::resize(src, resized, cv::Size(tw, th), 0, 0, cv::INTER_LINEAR);
        } else {
            resized = src;
        }

        cv::Mat rgb565_mat;
        cv::cvtColor(resized, rgb565_mat, cv::COLOR_RGB2BGR565);

        size_t needed = (size_t)tw * th;
        if(dst_buf.size() != needed) dst_buf.resize(needed, 0);

        std::memcpy(dst_buf.data(), rgb565_mat.data, needed * sizeof(uint16_t));

        dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
        dsc.header.cf = LV_COLOR_FORMAT_RGB565;
        dsc.header.w = tw;
        dsc.header.h = th;
        dsc.header.stride = tw * sizeof(uint16_t);
        dsc.data = reinterpret_cast<const uint8_t*>(dst_buf.data());
        dsc.data_size = dst_buf.size() * sizeof(uint16_t);

        lv_image_set_src(widget, &dsc);
        lv_obj_invalidate(widget); // LVGL 9 Cache Fix
    };

    // [OPTIMASI 2] RENDER LAYAR UTAMA (High Resolution)
    cv::Mat mat_moil_target;
    if (lv_obj_has_state(objects.btn_panorama, LV_STATE_CHECKED)) {
        if(maps_initialized) cv::remap(raw_frame, mat_moil_target, mapX_pano, mapY_pano, cv::INTER_LINEAR);
        else raw_frame.copyTo(mat_moil_target);
    } 
    else if (lv_obj_has_state(objects.btn_anypoint, LV_STATE_CHECKED)) {
        if(maps_initialized) cv::remap(raw_frame, mat_moil_target, mapX_any1, mapY_any1, cv::INTER_LINEAR);
        else raw_frame.copyTo(mat_moil_target);
    } 
    else {
        mat_moil_target = raw_frame; 
    }
    render_to_lvgl(mat_moil_target, buf_moil, dsc_moil, objects.moil_image);

    // [OPTIMASI 3] RENDER SEMUA THUMBNAIL REALTIME (Low Resolution Map)
    if(maps_initialized) {
        cv::Mat t_pano, t_any1, t_any2, t_any3, t_any4;
        
        // Remap menggunakan grid kecil. Cepat di PC, Aman di Renesas!
        cv::remap(raw_frame, t_pano, mapX_pano_thumb, mapY_pano_thumb, cv::INTER_LINEAR);
        cv::remap(raw_frame, t_any1, mapX_any1_thumb, mapY_any1_thumb, cv::INTER_LINEAR);
        cv::remap(raw_frame, t_any2, mapX_any2_thumb, mapY_any2_thumb, cv::INTER_LINEAR);
        cv::remap(raw_frame, t_any3, mapX_any3_thumb, mapY_any3_thumb, cv::INTER_LINEAR);
        cv::remap(raw_frame, t_any4, mapX_any4_thumb, mapY_any4_thumb, cv::INTER_LINEAR);

        render_to_lvgl(t_pano, buf_pano, dsc_pano, objects.panorama_image);
        render_to_lvgl(t_any1, buf_any1, dsc_any1, objects.anypoint_1_image);
        render_to_lvgl(t_any2, buf_any2, dsc_any2, objects.anypoint_2_image);
        render_to_lvgl(t_any3, buf_any3, dsc_any3, objects.anypoint_3_image);
        render_to_lvgl(t_any4, buf_any4, dsc_any4, objects.anypoint_4_image);
    }

    gst_buffer_unmap(buf, &map);
    gst_sample_unref(sample);

    // ==========================================================
    // LOGIKA FPS MONITORING
    // ==========================================================
    static std::chrono::steady_clock::time_point last_tp;
    static double fps_ema = 0.0;
    static bool first = true;
    auto now = std::chrono::steady_clock::now();
    if(!first) {
        std::chrono::duration<double> dt = now - last_tp;
        double inst_fps = (dt.count() > 0.0) ? (1.0 / dt.count()) : 0.0;
        fps_ema = (fps_ema <= 0.0) ? inst_fps : (fps_ema * 0.85 + inst_fps * 0.15);

        if(!ui_lbl_fps_local) {
            ui_lbl_fps_local = lv_label_create(lv_scr_act());
            lv_obj_align(ui_lbl_fps_local, LV_ALIGN_TOP_RIGHT, -10, 10);
            lv_obj_move_foreground(ui_lbl_fps_local);
        }
        char fps_str[32];
        std::snprintf(fps_str, sizeof(fps_str), "FPS: %.1f", fps_ema);
        lv_label_set_text(ui_lbl_fps_local, fps_str);
    } else {
        first = false;
    }
    last_tp = now;
}

// =====================================================================
// Event Handler Tombol Mulai / Stop Kamera
// =====================================================================
extern "C" void on_start_kamera_clicked(lv_event_t * e) {
    if(!is_camera_running) {
        
        init_moildev_maps();
        
        static bool cb_initialized = false;
        if(!cb_initialized) {
            lv_obj_add_state(objects.btn_original, LV_STATE_CHECKED);
            lv_obj_add_flag(objects.config, LV_OBJ_FLAG_HIDDEN);
            
            lv_obj_add_event_cb(objects.btn_original, mode_checkbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            lv_obj_add_event_cb(objects.btn_panorama, mode_checkbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            lv_obj_add_event_cb(objects.btn_anypoint, mode_checkbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            
            lv_obj_add_event_cb(objects.alpha_conf, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            lv_obj_add_event_cb(objects.beta_conf, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            lv_obj_add_event_cb(objects.zoom_conf, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
            
            cb_initialized = true;
        }

        gst_init(nullptr, nullptr);

        const char * pipeline_desc =
            "v4l2src device=/dev/video0 ! videoconvert ! videoscale ! "
            "video/x-raw,format=RGB,width=1600,height=1200 ! appsink name=appsink max-buffers=1 drop=true sync=false";

        GError * err = nullptr;
        gst_pipeline = gst_parse_launch(pipeline_desc, &err);
        if(!gst_pipeline || err) {
            std::cerr << "ERROR: Failed to create GStreamer pipeline: " << (err ? err->message : "unknown") << std::endl;
            if(err) g_error_free(err);
            return;
        }

        gst_appsink = gst_bin_get_by_name(GST_BIN(gst_pipeline), "appsink");
        g_object_set(gst_appsink, "emit-signals", FALSE, "sync", FALSE, "max-buffers", 1, "drop", TRUE, NULL);

        GstStateChangeReturn sres = gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);
        if(sres == GST_STATE_CHANGE_FAILURE) {
            std::cerr << "ERROR: Failed to set pipeline to PLAYING" << std::endl;
            return;
        }

        is_camera_running = true;

        lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        if(label) lv_label_set_text(label, "Stop Kamera");

        if(camera_timer == NULL) {
            camera_timer = lv_timer_create(camera_stream_cb, 33, NULL);
        } else {
            lv_timer_resume(camera_timer);
        }

    } else {
        is_camera_running = false;

        if(gst_pipeline) {
            gst_element_set_state(gst_pipeline, GST_STATE_NULL);
            if(gst_appsink) gst_object_unref(gst_appsink);
            gst_object_unref(gst_pipeline);
            gst_pipeline = nullptr;
            gst_appsink = nullptr;
        }

        if(camera_timer != NULL) lv_timer_pause(camera_timer);

        if(objects.moil_image) lv_image_set_src(objects.moil_image, NULL);
        if(objects.panorama_image) lv_image_set_src(objects.panorama_image, NULL);
        if(objects.anypoint_1_image) lv_image_set_src(objects.anypoint_1_image, NULL);
        if(objects.anypoint_2_image) lv_image_set_src(objects.anypoint_2_image, NULL);
        if(objects.anypoint_3_image) lv_image_set_src(objects.anypoint_3_image, NULL);
        if(objects.anypoint_4_image) lv_image_set_src(objects.anypoint_4_image, NULL);

        lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        if(label) lv_label_set_text(label, "Mulai Kamera");
    }
}