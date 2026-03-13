#include "ui_new.h"
#include "screens.h"
#include "lvgl/lvgl.h"
#include "utils/shader_render.hpp"

#include <iostream>
#include <vector>
#include <chrono>
#include <cstdio>
#include <algorithm>
#include <cstring>

#include <opencv2/opencv.hpp>

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
// Shader Engine Instance
// =====================================================================
ShaderRenderer gpuShader;
bool shader_ready = false;

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
// Variabel Penyimpan Parameter Saat Ini
// =====================================================================
// Panorama: alpha_conf -> alpha_max, beta_conf -> alpha_min, zoom_conf -> beta_degree
static float pano_alpha_max = 110.0f;
static float pano_alpha_min = 0.0f;
static float pano_beta_deg  = 0.0f;

// Anypoint
static float any1_alpha = 5.0f;
static float any1_beta  = 0.0f;
static float any1_zoom  = 3.0f;

// =====================================================================
// Helper ukuran widget aktual
// =====================================================================
static inline int obj_w(lv_obj_t *obj, int fallback) {
    if(!obj) return fallback;
    int w = lv_obj_get_content_width(obj);
    return (w > 0) ? w : fallback;
}

static inline int obj_h(lv_obj_t *obj, int fallback) {
    if(!obj) return fallback;
    int h = lv_obj_get_content_height(obj);
    return (h > 0) ? h : fallback;
}

// =====================================================================
// Event Handler: Checkbox Radio Button & UI Config
// =====================================================================
static void mode_checkbox_event_cb(lv_event_t * e) {
    lv_obj_t * target = (lv_obj_t *)lv_event_get_target(e);
    if(!target) return;

    if(lv_obj_has_state(target, LV_STATE_CHECKED)) {
        // Hilangkan centang mode lain
        if(target != objects.btn_original)  lv_obj_remove_state(objects.btn_original, LV_STATE_CHECKED);
        if(target != objects.btn_panorama)  lv_obj_remove_state(objects.btn_panorama, LV_STATE_CHECKED);
        if(target != objects.btn_anypoint)  lv_obj_remove_state(objects.btn_anypoint, LV_STATE_CHECKED);

        if(target == objects.btn_panorama) {
            lv_obj_clear_flag(objects.config, LV_OBJ_FLAG_HIDDEN);

            // Tetap sesuai struktur child config yang saya susun sebelumnya:
            // 0 title, 1 label1, 2 slider1, 3 label2, 4 slider2, 5 label3, 6 slider3
            lv_label_set_text(lv_obj_get_child(objects.config, 1), "Alpha Max");
            lv_label_set_text(lv_obj_get_child(objects.config, 3), "Alpha");
            lv_label_set_text(lv_obj_get_child(objects.config, 5), "Beta");

            lv_slider_set_range(objects.alpha_conf, 10, 110);
            lv_slider_set_range(objects.beta_conf, 0, 110);
            lv_slider_set_range(objects.zoom_conf, -180, 180);

            lv_slider_set_value(objects.alpha_conf, (int)pano_alpha_max, LV_ANIM_OFF);
            lv_slider_set_value(objects.beta_conf, (int)pano_alpha_min, LV_ANIM_OFF);
            lv_slider_set_value(objects.zoom_conf, (int)pano_beta_deg, LV_ANIM_OFF);
        }
        else if(target == objects.btn_anypoint) {
            lv_obj_clear_flag(objects.config, LV_OBJ_FLAG_HIDDEN);

            lv_label_set_text(lv_obj_get_child(objects.config, 1), "Alpha");
            lv_label_set_text(lv_obj_get_child(objects.config, 3), "Beta");
            lv_label_set_text(lv_obj_get_child(objects.config, 5), "Zoom");

            lv_slider_set_range(objects.alpha_conf, 0, 110);
            lv_slider_set_range(objects.beta_conf, -180, 180);
            lv_slider_set_range(objects.zoom_conf, 10, 140);

            lv_slider_set_value(objects.alpha_conf, (int)any1_alpha, LV_ANIM_OFF);
            lv_slider_set_value(objects.beta_conf, (int)any1_beta, LV_ANIM_OFF);
            lv_slider_set_value(objects.zoom_conf, (int)(any1_zoom * 10.0f), LV_ANIM_OFF);
        }
        else if(target == objects.btn_original) {
            lv_obj_add_flag(objects.config, LV_OBJ_FLAG_HIDDEN);
        }
    }
    else {
        if(target != objects.btn_original) {
            lv_obj_add_state(objects.btn_original, LV_STATE_CHECKED);
            lv_obj_add_flag(objects.config, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// =====================================================================
// Event Handler: Geseran Slider
// =====================================================================
static void slider_event_cb(lv_event_t * e) {
    LV_UNUSED(e);

    int alpha_val = lv_slider_get_value(objects.alpha_conf);
    int beta_val  = lv_slider_get_value(objects.beta_conf);
    int zoom_val  = lv_slider_get_value(objects.zoom_conf);

    if(lv_obj_has_state(objects.btn_panorama, LV_STATE_CHECKED)) {
        pano_alpha_max = (float)alpha_val;
        pano_alpha_min = (float)beta_val;
        pano_beta_deg  = (float)zoom_val;
    }
    else if(lv_obj_has_state(objects.btn_anypoint, LV_STATE_CHECKED)) {
        any1_alpha = (float)alpha_val;
        any1_beta  = (float)beta_val;
        any1_zoom  = (float)zoom_val / 10.0f;
    }
}

// =====================================================================
// Helper: Assign Buffer ke LVGL Image
// =====================================================================
static void update_lvgl_image(lv_obj_t* widget,
                              lv_image_dsc_t& dsc,
                              std::vector<uint16_t>& buf,
                              int w,
                              int h) {
    if(!widget || w <= 0 || h <= 0 || buf.empty()) return;

    dsc.header.magic  = LV_IMAGE_HEADER_MAGIC;
    dsc.header.cf     = LV_COLOR_FORMAT_RGB565;
    dsc.header.w      = w;
    dsc.header.h      = h;
    dsc.header.stride = w * (int)sizeof(uint16_t);
    dsc.data          = reinterpret_cast<const uint8_t*>(buf.data());
    dsc.data_size     = buf.size() * sizeof(uint16_t);

    lv_image_set_src(widget, &dsc);
    lv_obj_invalidate(widget);
}

// =====================================================================
// Callback Timer (Render Loop GPU Shader)
// =====================================================================
static void camera_stream_cb(lv_timer_t * timer) {
    LV_UNUSED(timer);

    if(!is_camera_running || gst_appsink == nullptr) return;

    GstSample * sample = gst_app_sink_try_pull_sample(GST_APP_SINK(gst_appsink), 10000000);
    if(!sample) return;

    GstBuffer * buf = gst_sample_get_buffer(sample);
    GstCaps   * caps = gst_sample_get_caps(sample);

    if(!buf || !caps) {
        gst_sample_unref(sample);
        return;
    }

    GstStructure * str = gst_caps_get_structure(caps, 0);
    int w = 1600;
    int h = 1200;
    gst_structure_get_int(str, "width", &w);
    gst_structure_get_int(str, "height", &h);

    GstMapInfo map;
    if(!gst_buffer_map(buf, &map, GST_MAP_READ)) {
        gst_sample_unref(sample);
        return;
    }

    cv::Mat raw_frame(h, w, CV_8UC3, map.data);

    // Ambil ukuran widget aktual dari layout 1920x1080
    const int pano_w = obj_w(objects.panorama_image, 1536);
    const int pano_h = obj_h(objects.panorama_image, 550);

    const int any_w  = obj_w(objects.anypoint_1_image, 382);
    const int any_h  = obj_h(objects.anypoint_1_image, 340);

    const int moil_w = obj_w(objects.moil_image, 336);
    const int moil_h = obj_h(objects.moil_image, 220);

    if(shader_ready) {
        // 1. Panorama
        gpuShader.render(raw_frame, pano_w, pano_h, 2.0f,
                         pano_alpha_max, pano_alpha_min, pano_beta_deg, pano_alpha_max,
                         buf_pano);
        update_lvgl_image(objects.panorama_image, dsc_pano, buf_pano, pano_w, pano_h);

        // 2. Anypoint 1
        gpuShader.render(raw_frame, any_w, any_h, 0.0f,
                         any1_alpha, any1_beta, any1_zoom, 0.0f,
                         buf_any1);
        update_lvgl_image(objects.anypoint_1_image, dsc_any1, buf_any1, any_w, any_h);

        // 3. Anypoint 2
        gpuShader.render(raw_frame, any_w, any_h, 0.0f,
                         45.0f, 2.0f, 4.0f, 0.0f,
                         buf_any2);
        update_lvgl_image(objects.anypoint_2_image, dsc_any2, buf_any2, any_w, any_h);

        // 4. Anypoint 3
        gpuShader.render(raw_frame, any_w, any_h, 0.0f,
                         40.0f, -175.0f, 4.0f, 0.0f,
                         buf_any3);
        update_lvgl_image(objects.anypoint_3_image, dsc_any3, buf_any3, any_w, any_h);

        // 5. Anypoint 4
        gpuShader.render(raw_frame, any_w, any_h, 0.0f,
                         50.0f, -180.0f, 4.5f, 0.0f,
                         buf_any4);
        update_lvgl_image(objects.anypoint_4_image, dsc_any4, buf_any4, any_w, any_h);

        // 6. Sidebar preview (moil_image)
        if(lv_obj_has_state(objects.btn_panorama, LV_STATE_CHECKED)) {
            gpuShader.render(raw_frame, moil_w, moil_h, 2.0f,
                             pano_alpha_max, pano_alpha_min, pano_beta_deg, pano_alpha_max,
                             buf_moil);
        }
        else if(lv_obj_has_state(objects.btn_anypoint, LV_STATE_CHECKED)) {
            gpuShader.render(raw_frame, moil_w, moil_h, 0.0f,
                             any1_alpha, any1_beta, any1_zoom, 0.0f,
                             buf_moil);
        }
        else {
            // Original Mode - resize ringan via OpenCV
            cv::Mat resized_moil, rgb565_moil;
            cv::resize(raw_frame, resized_moil, cv::Size(moil_w, moil_h), 0, 0, cv::INTER_AREA);
            cv::cvtColor(resized_moil, rgb565_moil, cv::COLOR_RGB2BGR565);

            const size_t needed = (size_t)moil_w * (size_t)moil_h;
            if(buf_moil.size() != needed) {
                buf_moil.resize(needed);
            }

            std::memcpy(buf_moil.data(), rgb565_moil.data, needed * sizeof(uint16_t));
        }

        update_lvgl_image(objects.moil_image, dsc_moil, buf_moil, moil_w, moil_h);
    }

    gst_buffer_unmap(buf, &map);
    gst_sample_unref(sample);

    // ==========================================================
    // FPS MONITORING
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
    }
    else {
        first = false;
    }

    last_tp = now;
}

// =====================================================================
// Event Handler Tombol Mulai / Stop Kamera
// =====================================================================
extern "C" void on_start_kamera_clicked(lv_event_t * e) {
    if(!is_camera_running) {
        // Pastikan layout sudah fix sebelum ambil ukuran widget
        if(objects.main_view) {
            lv_obj_update_layout(objects.main_view);
        }

        // Inisialisasi Shader GPU dari file .frag
        if(!shader_ready) {
            std::cout << "[INFO] Menginisialisasi GPU Shader..." << std::endl;
            shader_ready = gpuShader.init("moil/anypoint_gl.frag");
            if(!shader_ready) {
                std::cerr << "ERROR: Gagal init shader renderer" << std::endl;
                return;
            }
        }

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
            "video/x-raw,format=RGB,width=1600,height=1200 ! "
            "appsink name=appsink max-buffers=1 drop=true sync=false";

        GError * err = nullptr;
        gst_pipeline = gst_parse_launch(pipeline_desc, &err);
        if(!gst_pipeline || err) {
            std::cerr << "ERROR: Failed to create GStreamer pipeline: "
                      << (err ? err->message : "unknown") << std::endl;
            if(err) g_error_free(err);
            return;
        }

        gst_appsink = gst_bin_get_by_name(GST_BIN(gst_pipeline), "appsink");
        if(!gst_appsink) {
            std::cerr << "ERROR: Failed to get appsink from pipeline" << std::endl;
            gst_object_unref(gst_pipeline);
            gst_pipeline = nullptr;
            return;
        }

        g_object_set(gst_appsink,
                     "emit-signals", FALSE,
                     "sync", FALSE,
                     "max-buffers", 1,
                     "drop", TRUE,
                     NULL);

        GstStateChangeReturn sres = gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);
        if(sres == GST_STATE_CHANGE_FAILURE) {
            std::cerr << "ERROR: Failed to set pipeline to PLAYING" << std::endl;
            if(gst_appsink) {
                gst_object_unref(gst_appsink);
                gst_appsink = nullptr;
            }
            if(gst_pipeline) {
                gst_object_unref(gst_pipeline);
                gst_pipeline = nullptr;
            }
            return;
        }

        is_camera_running = true;

        lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
        lv_obj_t * label = btn ? lv_obj_get_child(btn, 0) : NULL;
        if(label) {
            lv_label_set_text(label, "Stop Kamera");
        }

        if(camera_timer == NULL) {
            camera_timer = lv_timer_create(camera_stream_cb, 33, NULL);
        }
        else {
            lv_timer_resume(camera_timer);
        }
    }
    else {
        is_camera_running = false;

        if(gst_pipeline) {
            gst_element_set_state(gst_pipeline, GST_STATE_NULL);

            if(gst_appsink) {
                gst_object_unref(gst_appsink);
                gst_appsink = nullptr;
            }

            gst_object_unref(gst_pipeline);
            gst_pipeline = nullptr;
        }

        if(camera_timer != NULL) {
            lv_timer_pause(camera_timer);
        }

        if(objects.moil_image)       lv_image_set_src(objects.moil_image, NULL);
        if(objects.panorama_image)   lv_image_set_src(objects.panorama_image, NULL);
        if(objects.anypoint_1_image) lv_image_set_src(objects.anypoint_1_image, NULL);
        if(objects.anypoint_2_image) lv_image_set_src(objects.anypoint_2_image, NULL);
        if(objects.anypoint_3_image) lv_image_set_src(objects.anypoint_3_image, NULL);
        if(objects.anypoint_4_image) lv_image_set_src(objects.anypoint_4_image, NULL);

        lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
        lv_obj_t * label = btn ? lv_obj_get_child(btn, 0) : NULL;
        if(label) {
            lv_label_set_text(label, "Open Camera");
        }
    }
}