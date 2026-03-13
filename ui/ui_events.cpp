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
#include <cstdint>

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

// FPS label di sidebar
static lv_obj_t * ui_lbl_fps_local = NULL;

// =====================================================================
// FPS global untuk ditampilkan/diakses file lain
// =====================================================================
double g_camera_fps_ema = 0.0;

// =====================================================================
// Variabel Penyimpan Parameter Saat Ini
// =====================================================================
static float pano_alpha_max = 110.0f;
static float pano_alpha_min = 0.0f;
static float pano_beta_deg  = 0.0f;

struct AnyPointState {
    float alpha;
    float beta;
    float zoom;
};

static AnyPointState any_views[4] = {
    {  5.0f,    0.0f, 3.0f },
    { 45.0f,    2.0f, 3.2f },
    { 40.0f, -175.0f, 3.2f },
    { 50.0f, -180.0f, 3.4f }
};

static int selected_any_idx = 0;

// =====================================================================
// Helper
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

static inline float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

static inline float wrap_beta(float beta) {
    while(beta > 180.0f) beta -= 360.0f;
    while(beta < -180.0f) beta += 360.0f;
    return beta;
}

static lv_obj_t* anypoint_box_from_index(int idx) {
    switch(idx) {
        case 0: return objects.anypoint_1;
        case 1: return objects.anypoint_2;
        case 2: return objects.anypoint_3;
        case 3: return objects.anypoint_4;
        default: return nullptr;
    }
}

static bool point_in_obj(lv_obj_t *obj, int x, int y) {
    if(!obj) return false;

    lv_area_t a;
    lv_obj_get_coords(obj, &a);

    return (x >= a.x1 && x <= a.x2 && y >= a.y1 && y <= a.y2);
}

static int anypoint_hit_test(int x, int y) {
    if(point_in_obj(objects.anypoint_1, x, y)) return 0;
    if(point_in_obj(objects.anypoint_2, x, y)) return 1;
    if(point_in_obj(objects.anypoint_3, x, y)) return 2;
    if(point_in_obj(objects.anypoint_4, x, y)) return 3;
    return -1;
}

static void refresh_anypoint_selection_ui() {
    for(int i = 0; i < 4; ++i) {
        lv_obj_t *box = anypoint_box_from_index(i);
        if(!box) continue;

        if(i == selected_any_idx) {
            lv_obj_set_style_border_width(box, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(box, lv_palette_main(LV_PALETTE_BLUE), LV_PART_MAIN | LV_STATE_DEFAULT);
        } else {
            lv_obj_set_style_border_width(box, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(box, lv_palette_darken(LV_PALETTE_GREY, 2), LV_PART_MAIN | LV_STATE_DEFAULT);
        }
    }
}

static void sync_anypoint_config_title() {
    if(!objects.config) return;
    lv_obj_t *title = lv_obj_get_child(objects.config, 0);
    if(!title) return;

    char txt[64];
    std::snprintf(txt, sizeof(txt), "Parameter Config - A%d", selected_any_idx + 1);
    lv_label_set_text(title, txt);
}

static void sync_anypoint_sliders_from_selected() {
    if(!objects.alpha_conf || !objects.beta_conf || !objects.zoom_conf) return;

    lv_slider_set_value(objects.alpha_conf, (int)any_views[selected_any_idx].alpha, LV_ANIM_OFF);
    lv_slider_set_value(objects.beta_conf,  (int)any_views[selected_any_idx].beta,  LV_ANIM_OFF);
    lv_slider_set_value(objects.zoom_conf,  (int)(any_views[selected_any_idx].zoom * 10.0f), LV_ANIM_OFF);
    sync_anypoint_config_title();
}

static void select_anypoint(int idx, bool sync_ui) {
    if(idx < 0 || idx > 3) return;
    selected_any_idx = idx;
    refresh_anypoint_selection_ui();

    if(sync_ui && lv_obj_has_state(objects.btn_anypoint, LV_STATE_CHECKED)) {
        sync_anypoint_sliders_from_selected();
    }
}

static void ensure_fps_label_position() {
    if(!ui_lbl_fps_local && objects.sidebar) {
        ui_lbl_fps_local = lv_label_create(objects.sidebar);
        lv_obj_add_flag(ui_lbl_fps_local, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_label_set_text(ui_lbl_fps_local, "FPS: 0.0");
        lv_obj_move_foreground(ui_lbl_fps_local);
    }

    if(ui_lbl_fps_local) {
        if(ui_stat_gpu) {
            lv_obj_align_to(ui_lbl_fps_local, ui_stat_gpu, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
        } else if(ui_stat_mem) {
            lv_obj_align_to(ui_lbl_fps_local, ui_stat_mem, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 28);
        } else {
            lv_obj_align(ui_lbl_fps_local, LV_ALIGN_TOP_LEFT, 12, 720);
        }
    }
}

// =====================================================================
// Mouse wheel SDL bridge
// Dipanggil dari main.cpp via SDL_AddEventWatch
// =====================================================================
extern "C" void ui_anypoint_mousewheel(int mouse_x, int mouse_y, int wheel_y) {
    if(!is_camera_running) return;
    if(wheel_y == 0) return;

    int idx = anypoint_hit_test(mouse_x, mouse_y);
    if(idx < 0) return;

    select_anypoint(idx, true);

    // wheel up => zoom in
    any_views[idx].zoom = clampf(any_views[idx].zoom + (float)wheel_y * 0.12f, 1.0f, 4.5f);

    if(lv_obj_has_state(objects.btn_anypoint, LV_STATE_CHECKED) && idx == selected_any_idx) {
        sync_anypoint_sliders_from_selected();
    }
}

// =====================================================================
// Event Handler: Checkbox Radio Button & UI Config
// =====================================================================
static void mode_checkbox_event_cb(lv_event_t * e) {
    lv_obj_t * target = (lv_obj_t *)lv_event_get_target(e);
    if(!target) return;

    if(lv_obj_has_state(target, LV_STATE_CHECKED)) {
        if(target != objects.btn_original)  lv_obj_remove_state(objects.btn_original, LV_STATE_CHECKED);
        if(target != objects.btn_panorama)  lv_obj_remove_state(objects.btn_panorama, LV_STATE_CHECKED);
        if(target != objects.btn_anypoint)  lv_obj_remove_state(objects.btn_anypoint, LV_STATE_CHECKED);

        if(target == objects.btn_panorama) {
            lv_obj_clear_flag(objects.config, LV_OBJ_FLAG_HIDDEN);

            lv_label_set_text(lv_obj_get_child(objects.config, 0), "Parameter Config");
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
            lv_slider_set_range(objects.zoom_conf, 10, 45); // 1.0 - 4.5

            sync_anypoint_sliders_from_selected();
        }
        else if(target == objects.btn_original) {
            lv_obj_add_flag(objects.config, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
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
        any_views[selected_any_idx].alpha = clampf((float)alpha_val, 0.0f, 110.0f);
        any_views[selected_any_idx].beta  = wrap_beta((float)beta_val);
        any_views[selected_any_idx].zoom  = clampf((float)zoom_val / 10.0f, 1.0f, 4.5f);
    }
}

// =====================================================================
// Interaksi langsung di anypoint: drag = alpha/beta
// =====================================================================
static void anypoint_interact_event_cb(lv_event_t * e) {
    const int idx = (int)(intptr_t)lv_event_get_user_data(e);
    const lv_event_code_t code = lv_event_get_code(e);

    if(idx < 0 || idx > 3) return;

    if(code == LV_EVENT_PRESSED) {
        select_anypoint(idx, true);
    }
    else if(code == LV_EVENT_PRESSING) {
        lv_indev_t * indev = lv_indev_active();
        if(!indev) return;

        lv_point_t vect;
        lv_indev_get_vect(indev, &vect);

        if(vect.x == 0 && vect.y == 0) return;

        select_anypoint(idx, false);

        // drag horizontal -> beta
        // drag vertical   -> alpha
        any_views[idx].beta  = wrap_beta(any_views[idx].beta + (float)vect.x * 0.40f);
        any_views[idx].alpha = clampf(any_views[idx].alpha - (float)vect.y * 0.18f, 0.0f, 110.0f);

        if(lv_obj_has_state(objects.btn_anypoint, LV_STATE_CHECKED) && idx == selected_any_idx) {
            sync_anypoint_sliders_from_selected();
        }
    }
}

static void bind_anypoint_interaction(lv_obj_t *box, lv_obj_t *img, int idx) {
    if(box) {
        lv_obj_add_flag(box, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(box, anypoint_interact_event_cb, LV_EVENT_PRESSED,  (void*)(intptr_t)idx);
        lv_obj_add_event_cb(box, anypoint_interact_event_cb, LV_EVENT_PRESSING, (void*)(intptr_t)idx);
    }

    if(img) {
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, anypoint_interact_event_cb, LV_EVENT_PRESSED,  (void*)(intptr_t)idx);
        lv_obj_add_event_cb(img, anypoint_interact_event_cb, LV_EVENT_PRESSING, (void*)(intptr_t)idx);
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

    const int pano_w = obj_w(objects.panorama_image, 1536);
    const int pano_h = obj_h(objects.panorama_image, 550);

    const int any_w  = obj_w(objects.anypoint_1_image, 382);
    const int any_h  = obj_h(objects.anypoint_1_image, 340);

    const int moil_w = obj_w(objects.moil_image, 336);
    const int moil_h = obj_h(objects.moil_image, 220);

    if(shader_ready) {
        gpuShader.render(raw_frame, pano_w, pano_h, 2.0f,
                         pano_alpha_max, pano_alpha_min, pano_beta_deg, pano_alpha_max,
                         buf_pano);
        update_lvgl_image(objects.panorama_image, dsc_pano, buf_pano, pano_w, pano_h);

        gpuShader.render(raw_frame, any_w, any_h, 0.0f,
                         any_views[0].alpha, any_views[0].beta, any_views[0].zoom, 0.0f,
                         buf_any1);
        update_lvgl_image(objects.anypoint_1_image, dsc_any1, buf_any1, any_w, any_h);

        gpuShader.render(raw_frame, any_w, any_h, 0.0f,
                         any_views[1].alpha, any_views[1].beta, any_views[1].zoom, 0.0f,
                         buf_any2);
        update_lvgl_image(objects.anypoint_2_image, dsc_any2, buf_any2, any_w, any_h);

        gpuShader.render(raw_frame, any_w, any_h, 0.0f,
                         any_views[2].alpha, any_views[2].beta, any_views[2].zoom, 0.0f,
                         buf_any3);
        update_lvgl_image(objects.anypoint_3_image, dsc_any3, buf_any3, any_w, any_h);

        gpuShader.render(raw_frame, any_w, any_h, 0.0f,
                         any_views[3].alpha, any_views[3].beta, any_views[3].zoom, 0.0f,
                         buf_any4);
        update_lvgl_image(objects.anypoint_4_image, dsc_any4, buf_any4, any_w, any_h);

        if(lv_obj_has_state(objects.btn_panorama, LV_STATE_CHECKED)) {
            gpuShader.render(raw_frame, moil_w, moil_h, 2.0f,
                             pano_alpha_max, pano_alpha_min, pano_beta_deg, pano_alpha_max,
                             buf_moil);
        }
        else if(lv_obj_has_state(objects.btn_anypoint, LV_STATE_CHECKED)) {
            const AnyPointState &sel = any_views[selected_any_idx];
            gpuShader.render(raw_frame, moil_w, moil_h, 0.0f,
                             sel.alpha, sel.beta, sel.zoom, 0.0f,
                             buf_moil);
        }
        else {
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

    // FPS monitoring
    static std::chrono::steady_clock::time_point last_tp;
    static bool first = true;

    auto now = std::chrono::steady_clock::now();
    if(!first) {
        std::chrono::duration<double> dt = now - last_tp;
        double inst_fps = (dt.count() > 0.0) ? (1.0 / dt.count()) : 0.0;
        g_camera_fps_ema = (g_camera_fps_ema <= 0.0) ? inst_fps : (g_camera_fps_ema * 0.85 + inst_fps * 0.15);

        ensure_fps_label_position();
        if(ui_lbl_fps_local) {
            char fps_str[32];
            std::snprintf(fps_str, sizeof(fps_str), "FPS: %.1f", g_camera_fps_ema);
            lv_label_set_text(ui_lbl_fps_local, fps_str);
        }
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
        if(objects.main_view) {
            lv_obj_update_layout(objects.main_view);
        }

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

            bind_anypoint_interaction(objects.anypoint_1, objects.anypoint_1_image, 0);
            bind_anypoint_interaction(objects.anypoint_2, objects.anypoint_2_image, 1);
            bind_anypoint_interaction(objects.anypoint_3, objects.anypoint_3_image, 2);
            bind_anypoint_interaction(objects.anypoint_4, objects.anypoint_4_image, 3);

            refresh_anypoint_selection_ui();
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

        ensure_fps_label_position();
        if(ui_lbl_fps_local) {
            lv_label_set_text(ui_lbl_fps_local, "FPS: 0.0");
        }

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
        g_camera_fps_ema = 0.0;

        ensure_fps_label_position();
        if(ui_lbl_fps_local) {
            lv_label_set_text(ui_lbl_fps_local, "FPS: 0.0");
        }

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