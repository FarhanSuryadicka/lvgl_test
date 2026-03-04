# lvglkamera — LVGL + Kamera Fisheye (Moildev) + GStreamer

Proyek ini adalah aplikasi C++ performa tinggi yang menggabungkan antarmuka **LVGL (Light and Versatile Graphics Library)**, **GStreamer** untuk penangkapan *pipeline* kamera, **OpenCV** untuk pemrosesan gambar, dan **Moildev** untuk de-warping lensa *fisheye* (mode Panorama & Anypoint). 

Aplikasi ini mendukung dua mode *build* utama:
1. **PC Desktop Simulator:** Menggunakan SDL2 untuk *rendering* UI di lingkungan lokal.
2. **Embedded Linux (Renesas/Poky):** Menggunakan DRM/Evdev untuk *rendering* langsung ke layar perangkat keras pada distribusi berbasis **Yocto (aarch64-poky-linux)**.

---

## 📂 Ringkasan Struktur Direktori

- `CMakeLists.txt` — Konfigurasi build utama. Mendeteksi target platform secara otomatis.
- `src/` — Berisi *entry point* aplikasi (`main_stats.cpp`) dan logika pembacaan statistik perangkat keras (`platform_stats.cpp`).
- `ui/` — Sumber antarmuka LVGL. Logika pemrosesan Moildev dan *event handler* UI berada di `ui_events.cpp`.
- `lib/lvgl/` — Sumber *library* LVGL. Konfigurasi otomatis melalui `lv_conf.h` (SDL2 vs DRM).
- `lib/moil/` — Berisi *precompiled library* Moildev:
  - `aarch64/` -> `libmoildev.so` untuk target Renesas (Poky).
  - `x86_64/` -> `libmoildev.so` untuk PC Simulator.

---

## 🛠️ Dependensi Utama

Proyek ini membutuhkan pustaka berikut:
- **LVGL v9** (Termasuk dalam *source*)
- **SDL2** (Hanya untuk PC Simulator)
- **libdrm** (Hanya untuk target Renesas)
- **GStreamer 1.0** & `gst-plugins-base`
- **OpenCV 4**
- **Moildev** (Disediakan internal di `lib/moil/`)

### Instalasi Dependensi PC (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config git \
    libsdl2-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-good gstreamer1.0-tools libopencv-dev
```

---

## 🚀 Cara Build & Menjalankan

### Opsi 1: Build untuk PC Simulator (x86_64)
Gunakan mode ini untuk menguji UI dan logika Moildev di PC Anda.
```bash
mkdir -p build && cd build
cmake ..
cmake --build . -j$(nproc)
./lvglkamera
```

### Opsi 2: Build untuk Renesas Embedded (Yocto/Poky)
Gunakan Yocto SDK Toolchain untuk *cross-compile*. Script `build.sh` akan otomatis mendeteksi lingkungan SDK Anda.

1. **Aktifkan SDK Yocto:**
   Sesuaikan path dengan lokasi instalasi SDK Anda.
   ```bash
   source /opt/poky/x.y.z/environment-setup-aarch64-poky-linux
   ```

2. **Jalankan Build Script:**
   ```bash
   chmod +x build.sh
   ./build.sh
   ```

**⚠️ Penting saat Deploy ke Renesas:**
1. Salin *executable* `lvglkamera` hasil build ke board.
2. Salin file `lib/moil/aarch64/libmoildev.so` ke direktori library sistem di board (contoh: `/usr/lib`) agar aplikasi dapat berjalan tanpa error *linkage*.

---

## 🧠 Fitur Utama

- **Fisheye Dewarping Real-time:** Transformasi gambar *fisheye* menjadi **Panorama** atau **Anypoint** secara instan.
- **Optimasi Resolusi:** Berjalan pada **1600x1200** untuk keseimbangan performa CPU dan kualitas visual.
- **UI Interaktif:** Parameter lensa (Alpha, Beta, Zoom) dapat disesuaikan via *slider* dengan transisi mode otomatis.
- **Auto-Config UI:** Label konfigurasi berubah secara dinamis (contoh: Alpha menjadi Alpha Max pada mode Panorama).
- **Hardware Stats:** Monitoring penggunaan CPU aplikasi secara akurat (sinkron dengan System Monitor) serta RAM dan GPU.

---

## 🔧 Troubleshooting

- **Error: `Yocto SDK environment belum diaktifkan!`**
  Pastikan Anda sudah menjalankan perintah `source` pada file `environment-setup-...` milik SDK Renesas Anda sebelum menjalankan `build.sh`.
- **Error: `undefined reference to moildev::...`**
  Periksa folder `lib/moil/`. Pastikan library `.so` tersedia untuk arsitektur yang sedang dibangun.
- **Error: `cannot open shared object file` (Di Board)**
  Pastikan `libmoildev.so` versi `aarch64` sudah dikopi ke `/usr/lib` atau folder yang terdaftar di `LD_LIBRARY_PATH` pada board Renesas.
- **Kamera Lag/Freeze:**
  Pastikan user memiliki izin akses ke device: `sudo chmod 666 /dev/video0` atau jalankan sebagai root.