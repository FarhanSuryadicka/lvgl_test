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

// Variabel Global
bool is_camera_running = false;
lv_timer_t * camera_timer = NULL;

// GStreamer elements for capture
static GstElement * gst_pipeline = nullptr;
static GstElement * gst_appsink = nullptr;

// Image descriptors & buffers
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

// Moildev Engine & Maps
static moildev::Moildev* moil = nullptr;
static bool maps_initialized = false;
static cv::Mat mapX_pano, mapY_pano;
static cv::Mat mapX_any1, mapY_any1;
static cv::Mat mapX_any2, mapY_any2;
static cv::Mat mapX_any3, mapY_any3;
static cv::Mat mapX_any4, mapY_any4;

// Inisialisasi Moildev dan Generate Map
void init_moildev_maps() {
    if (maps_initialized) return;

    // Parameter asli: 94,wxsj_7730_4,220,1,1,1238,987,1,1,2592,1944,0,0,-32.973,67.825,-41.581,504.7,0
    float sensorW = 1.0f, sensorH = 1.0f;
    float iCx = 1238.0f, iCy = 987.0f;
    float ratio = 1.0f;
    float imgW = 2592.0f, imgH = 1944.0f;
    float calib = 1.0f;
    float p0 = 0.0f, p1 = 0.0f, p2 = -32.973f, p3 = 67.825f, p4 = -41.581f, p5 = 504.7f;

    moil = new moildev::Moildev(sensorW, sensorH, iCx, iCy, ratio, imgW, imgH, calib, p0, p1, p2, p3, p4, p5);

    // Target resolusi yang lebih ringan
    float targetW = 1600.0f;
    float targetH = 1200.0f;

    // Helper lambda: Generate map ukuran 2592x1944, lalu scale down ke 1600x1200
    auto generate_scaled_map = [&](float alpha, float beta, float zoom, int mode, cv::Mat& outX, cv::Mat& outY) {
        cv::Mat mX(imgH, imgW, CV_32F);
        cv::Mat mY(imgH, imgW, CV_32F);

        if (mode == 0) { // Anypoint
            moil->AnyPointM((float*)mX.data, (float*)mY.data, alpha, beta, zoom);
        } else { // Panorama
            moil->PanoramaCar((float*)mX.data, (float*)mY.data, alpha, beta, zoom, false, false);
        }

        // Scale koordinat map agar sesuai untuk gambar 1600x1200
        float scale_x = targetW / imgW;
        float scale_y = targetH / imgH;
        mX *= scale_x;
        mY *= scale_y;

        // Resize ukuran map itu sendiri menjadi 1600x1200
        cv::resize(mX, outX, cv::Size(targetW, targetH));
        cv::resize(mY, outY, cv::Size(targetW, targetH));
    };

    std::cout << "[INFO] Membuat Maps Moildev (Resolusi Optimasi 1600x1200)..." << std::endl;
    generate_scaled_map(110.0f, 0.0f, 100.0f, 1, mapX_pano, mapY_pano); // Panorama
    generate_scaled_map(5.0f, 0.0f, 3.0f, 0, mapX_any1, mapY_any1);     // Anypoint 1
    generate_scaled_map(45.0f, 2.0f, 4.0f, 0, mapX_any2, mapY_any2);    // Anypoint 2
    generate_scaled_map(40.0f, -175.0f, 4.0f, 0, mapX_any3, mapY_any3); // Anypoint 3
    generate_scaled_map(50.0f, -180.0f, 4.5f, 0, mapX_any4, mapY_any4); // Anypoint 4
    
    maps_initialized = true;
    std::cout << "[INFO] Maps Moildev selesai dibuat." << std::endl;
}

// Callback Timer
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

    // Wrap GStreamer buffer ke cv::Mat (1600x1200)
    cv::Mat raw_frame(h, w, CV_8UC3, map.data);
    
    cv::Mat frame_pano, frame_any1, frame_any2, frame_any3, frame_any4;

    // Remap pada resolusi menengah
    if(maps_initialized) {
        cv::remap(raw_frame, frame_pano, mapX_pano, mapY_pano, cv::INTER_LINEAR);
        cv::remap(raw_frame, frame_any1, mapX_any1, mapY_any1, cv::INTER_LINEAR);
        cv::remap(raw_frame, frame_any2, mapX_any2, mapY_any2, cv::INTER_LINEAR);
        cv::remap(raw_frame, frame_any3, mapX_any3, mapY_any3, cv::INTER_LINEAR);
        cv::remap(raw_frame, frame_any4, mapX_any4, mapY_any4, cv::INTER_LINEAR);
    } else {
        raw_frame.copyTo(frame_pano);
        raw_frame.copyTo(frame_any1);
        raw_frame.copyTo(frame_any2);
        raw_frame.copyTo(frame_any3);
        raw_frame.copyTo(frame_any4);
    }

    // Render ke LVGL
    auto render_to_lvgl = [&](const cv::Mat& src, std::vector<uint16_t>& dst_buf, lv_image_dsc_t& dsc, lv_obj_t* widget) {
        if(!widget) return;
        
        int tw = lv_obj_get_width(widget);
        int th = lv_obj_get_height(widget);
        if(tw <= 0 || th <= 0) { tw = 128; th = 128; } // Fallback

        // Resize cv::Mat ke ukuran widget UI
        cv::Mat resized;
        if(src.cols != tw || src.rows != th) {
            cv::resize(src, resized, cv::Size(tw, th), 0, 0, cv::INTER_LINEAR);
        } else {
            resized = src;
        }

        size_t needed = (size_t)tw * th;
        if(dst_buf.size() != needed) dst_buf.resize(needed, 0);

        // RGB888 -> RGB565
        const uint8_t* ptr = resized.data;
        for(size_t i = 0; i < needed; ++i) {
            uint8_t r = ptr[i*3 + 0];
            uint8_t g = ptr[i*3 + 1];
            uint8_t b = ptr[i*3 + 2];
            dst_buf[i] = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
        }

        dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
        dsc.header.cf = LV_COLOR_FORMAT_RGB565;
        dsc.header.w = tw;
        dsc.header.h = th;
        dsc.header.stride = tw * sizeof(uint16_t);
        dsc.data = reinterpret_cast<const uint8_t*>(dst_buf.data());
        dsc.data_size = dst_buf.size() * sizeof(uint16_t);

        lv_image_set_src(widget, &dsc);
    };

    render_to_lvgl(raw_frame,  buf_moil, dsc_moil, objects.moil_image);
    render_to_lvgl(frame_pano, buf_pano, dsc_pano, objects.panorama_image);
    render_to_lvgl(frame_any1, buf_any1, dsc_any1, objects.anypoint_1_image);
    render_to_lvgl(frame_any2, buf_any2, dsc_any2, objects.anypoint_2_image);
    render_to_lvgl(frame_any3, buf_any3, dsc_any3, objects.anypoint_3_image);
    render_to_lvgl(frame_any4, buf_any4, dsc_any4, objects.anypoint_4_image);

    gst_buffer_unmap(buf, &map);
    gst_sample_unref(sample);

    // FPS Counter
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

// Event Handler Tombol
extern "C" void on_start_kamera_clicked(lv_event_t * e) {
    if(!is_camera_running) {
        
        init_moildev_maps();
        gst_init(nullptr, nullptr);

        // Pipeline diubah untuk ukuran 1600x1200
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