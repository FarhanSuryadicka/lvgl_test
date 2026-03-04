#include "ui_new.h"
#include "screens.h"
#include "lvgl/lvgl.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <cstdio>

/* GStreamer capture (appsink) */
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

// Variabel Global
bool is_camera_running = false;
lv_timer_t * camera_timer = NULL;

// GStreamer elements for capture
static GstElement * gst_pipeline = nullptr;
static GstElement * gst_appsink = nullptr;
lv_image_dsc_t img_dsc = {0}; // Struktur gambar milik LVGL

// Local FPS label for compatibility with older UI
static lv_obj_t * ui_lbl_fps_local = NULL;

// Callback Timer (Berjalan ~30 FPS)
static void camera_stream_cb(lv_timer_t * timer) {
    if(!is_camera_running) return;
    // Pull a sample from appsink (non-blocking, short timeout)
    if(gst_appsink == nullptr) return;

    GstSample * sample = gst_app_sink_try_pull_sample(GST_APP_SINK(gst_appsink), 10000000 /* 10ms in ns */);
    if(!sample) return;

    GstBuffer * buf = gst_sample_get_buffer(sample);
    GstCaps * caps = gst_sample_get_caps(sample);
    if(!buf || !caps) {
        if(sample) gst_sample_unref(sample);
        return;
    }

    // Get frame width/height from caps
    GstStructure * str = gst_caps_get_structure(caps, 0);
    int w = 640, h = 480;
    gst_structure_get_int(str, "width", &w);
    gst_structure_get_int(str, "height", &h);

    GstMapInfo map;
    if(!gst_buffer_map(buf, &map, GST_MAP_READ)) {
        gst_sample_unref(sample);
        return;
    }

    // Expect map.data to be in RGB24 (3 bytes per pixel) as the pipeline's caps request
    const uint8_t * src = map.data;
    const size_t src_size = map.size;

    // 4. Pack RGB888 -> RGB565 into a persistent buffer for LVGL
    // We scale the incoming frame to the target display object (`moil_image`) size
    // so the sidebar shows a thumbnail instead of the full-sized fisheye image.
    static std::vector<uint16_t> rgb565_buf; // persists between calls

    const size_t expected_size = (size_t)w * (size_t)h * 3;
    if(src_size < expected_size) {
        // insufficient data
        gst_buffer_unmap(buf, &map);
        gst_sample_unref(sample);
        return;
    }

    // Determine target size. Prefer the `moil_image` object size when available.
    int tw = w, th = h;
    if(objects.moil_image) {
        tw = lv_obj_get_width(objects.moil_image);
        th = lv_obj_get_height(objects.moil_image);
        if(tw <= 0) tw = w / 4; // fallback to smaller thumbnail
        if(th <= 0) th = h / 4;
    } else {
        // Default thumbnail if no target object: quarter size
        tw = std::max(16, w / 4);
        th = std::max(16, h / 4);
    }

    rgb565_buf.resize((size_t)tw * (size_t)th);

    // Nearest-neighbor scaling from src (w x h, RGB888) -> dst (tw x th, RGB565)
    for(int yy = 0; yy < th; ++yy) {
        int src_y = (yy * h) / th;
        for(int xx = 0; xx < tw; ++xx) {
            int src_x = (xx * w) / tw;
            size_t si = (size_t)src_y * (size_t)w * 3 + (size_t)src_x * 3;
            const uint8_t r = src[si + 0];
            const uint8_t g = src[si + 1];
            const uint8_t b = src[si + 2];
            const uint16_t p = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
            rgb565_buf[(size_t)yy * (size_t)tw + (size_t)xx] = p;
        }
    }

    // Unmap and release sample
    gst_buffer_unmap(buf, &map);
    gst_sample_unref(sample);

    // 5. Siapkan struktur "Image Descriptor" untuk LVGL versi 9 (RGB565)
    img_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    img_dsc.header.cf = LV_COLOR_FORMAT_RGB565; // Match LV_COLOR_DEPTH = 16
    // Use the scaled target width/height (tw/th) so LVGL knows the image size
    img_dsc.header.w = tw;
    img_dsc.header.h = th;
    img_dsc.header.stride = tw * sizeof(uint16_t); // bytes per line

    // 6. Sambungkan pointer data LVGL ke buffer RGB565 kita
    img_dsc.data = reinterpret_cast<const uint8_t*>(rgb565_buf.data());
    img_dsc.data_size = rgb565_buf.size() * sizeof(uint16_t);

    // 6. Tampilkan ke UI! (map to new UI object's moil_image)
    if(objects.moil_image) {
        lv_image_set_src(objects.moil_image, &img_dsc);
    }

    // ---- Update FPS label (real-time, smoothed) ----
    // Use steady_clock to measure wall-clock time between frames
    static std::chrono::steady_clock::time_point last_tp;
    static double fps_ema = 0.0; // exponential moving average of FPS
    static bool first = true;
    auto now = std::chrono::steady_clock::now();
    if(!first) {
        std::chrono::duration<double> dt = now - last_tp;
        double inst_fps = 0.0;
        if(dt.count() > 0.0) inst_fps = 1.0 / dt.count();
        // EMA smoothing (alpha = 0.15)
        const double alpha = 0.15;
        if(fps_ema <= 0.0) fps_ema = inst_fps;
        else fps_ema = fps_ema * (1.0 - alpha) + inst_fps * alpha;

        if(!ui_lbl_fps_local) {
            ui_lbl_fps_local = lv_label_create(lv_scr_act());
            lv_label_set_text(ui_lbl_fps_local, "FPS: 0.0");
            lv_obj_align(ui_lbl_fps_local, LV_ALIGN_TOP_RIGHT, -10, 10);
        }
        if(ui_lbl_fps_local) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "FPS: %.1f", fps_ema);
            lv_label_set_text(ui_lbl_fps_local, buf);
        }
    } else {
        first = false;
    }
    last_tp = now;
}

// Event Handler Tombol
extern "C" void on_start_kamera_clicked(lv_event_t * e) {
    if(!is_camera_running) {
        // Initialize GStreamer and start pipeline.
        // Change the pipeline string below if you need a different source
        // (nvarguscamerasrc on Jetson, or other pipeline parts).
        gst_init(nullptr, nullptr);

        // Basic v4l2 pipeline -> convert -> scale -> RGB24 -> appsink
        const char * pipeline_desc =
            "v4l2src device=/dev/video0 ! videoconvert ! videoscale ! "
            "video/x-raw,format=RGB,width=640,height=480 ! appsink name=appsink max-buffers=1 drop=true sync=false";

        GError * err = nullptr;
        gst_pipeline = gst_parse_launch(pipeline_desc, &err);
        if(!gst_pipeline || err) {
            std::cerr << "ERROR: Failed to create GStreamer pipeline: " << (err ? err->message : "unknown") << std::endl;
            if(err) g_error_free(err);
            return;
        }

        // Retrieve appsink element
        gst_appsink = gst_bin_get_by_name(GST_BIN(gst_pipeline), "appsink");
        if(!gst_appsink) {
            std::cerr << "ERROR: appsink not found in pipeline" << std::endl;
            gst_object_unref(gst_pipeline);
            gst_pipeline = nullptr;
            return;
        }

        // Configure appsink properties
        g_object_set(gst_appsink, "emit-signals", FALSE, "sync", FALSE, "max-buffers", 1, "drop", TRUE, NULL);

        GstStateChangeReturn sres = gst_element_set_state(gst_pipeline, GST_STATE_PLAYING);
        if(sres == GST_STATE_CHANGE_FAILURE) {
            std::cerr << "ERROR: Failed to set GStreamer pipeline to PLAYING" << std::endl;
            gst_object_unref(gst_appsink);
            gst_appsink = nullptr;
            gst_object_unref(gst_pipeline);
            gst_pipeline = nullptr;
            return;
        }

        std::cout << "Kamera Dimulai... (GStreamer)" << std::endl;
        is_camera_running = true;

    // Ubah teks tombol
    lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    if(label) lv_label_set_text(label, "Stop Kamera");

        // Jalankan timer untuk mengambil frame setiap 33ms (~30 FPS)
        if(camera_timer == NULL) {
            camera_timer = lv_timer_create(camera_stream_cb, 33, NULL);
        } else {
            lv_timer_resume(camera_timer);
        }

    } else {
        std::cout << "Kamera Dihentikan..." << std::endl;
        is_camera_running = false;

        // Stop and free GStreamer pipeline
        if(gst_pipeline) {
            gst_element_set_state(gst_pipeline, GST_STATE_NULL);
            if(gst_appsink) {
                gst_object_unref(gst_appsink);
                gst_appsink = nullptr;
            }
            gst_object_unref(gst_pipeline);
            gst_pipeline = nullptr;
        }

    // Bersihkan layar (map to new UI object)
    if(objects.moil_image) lv_image_set_src(objects.moil_image, NULL);

    // Kembalikan teks tombol
    lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    if(label) lv_label_set_text(label, "Mulai Kamera");

        if(camera_timer != NULL) {
            lv_timer_pause(camera_timer);
        }
    }
}