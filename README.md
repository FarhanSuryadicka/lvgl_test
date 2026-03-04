# lvglkamera — LVGL + Kamera Fisheye (Moildev) + GStreamer

Proyek ini adalah aplikasi C++ performa tinggi yang menggabungkan antarmuka **LVGL (Light and Versatile Graphics Library)**, **GStreamer** untuk penangkapan *pipeline* kamera, **OpenCV** untuk pemrosesan gambar, dan **Moildev** untuk de-warping lensa *fisheye* (mode Panorama & Anypoint). 

Aplikasi ini mendukung dua mode *build* utama:
1. **PC Desktop Simulator:** Menggunakan SDL2 untuk *rendering* UI di lingkungan lokal.
2. **Embedded Linux (Renesas):** Menggunakan DRM/Evdev untuk *rendering* langsung ke layar perangkat keras (arsitektur AArch64).

---

## 📂 Ringkasan Struktur Direktori

- `CMakeLists.txt` — Konfigurasi build utama. Mendeteksi target platform (x86_64 vs AArch64) dan mengatur *linking* library secara otomatis.
- `src/` — Berisi *entry point* aplikasi (`main_stats.cpp`) dan logika pembacaan statistik perangkat keras (`platform_stats.cpp`).
- `ui/` — Sumber antarmuka LVGL. Logika pemrosesan Moildev, integrasi GStreamer, dan *event handler* UI berada di `ui_events.cpp`.
- `lib/lvgl/` — Sumber *library* LVGL. Konfigurasi LVGL dapat diubah melalui `lv_conf.h`.
- `lib/moil/` — Berisi *precompiled library* Moildev, dipisah berdasarkan arsitektur:
  - `aarch64/` -> Berisi `libmoildev.so` untuk target Renesas (ARM64).
  - `x86_64/` -> Berisi `libmoildev.so` untuk PC Simulator.

---

## 🛠️ Dependensi Utama

Proyek ini membutuhkan pustaka berikut:
- **LVGL** (Termasuk dalam *source*)
- **SDL2** (Hanya untuk PC Simulator)
- **libdrm** (Hanya untuk target Renesas)
- **GStreamer 1.0** & `gst-plugins-base`
- **OpenCV 4**
- **Moildev** (Disediakan internal di `lib/moil/`)

### Instalasi Dependensi PC (Ubuntu/Debian)

Gunakan perintah berikut untuk menginstal seluruh kebutuhan *build* di PC Simulator Anda (x86_64):

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config git \
    libsdl2-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-good gstreamer1.0-tools \
    libopencv-dev
```

### Instalasi Dependensi Cross-Compile (AArch64 / Renesas)

Jika Anda ingin melakukan *build* untuk board Renesas, Anda harus menginstal *toolchain cross-compile* dan pustaka versi `arm64` di PC Ubuntu host Anda:

```bash
# Tambahkan arsitektur arm64 ke sistem package manager
sudo dpkg --add-architecture arm64
sudo apt update

# Instal compiler dan library target
sudo apt install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
    libsdl2-dev:arm64 libgstreamer1.0-dev:arm64 libgstreamer-plugins-base1.0-dev:arm64 \
    libopencv-dev:arm64 libdrm-dev:arm64
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
*Catatan: RPATH telah diatur oleh CMake agar otomatis menemukan `libmoildev.so` di dalam folder `lib/moil/x86_64`.*

### Opsi 2: Build untuk Renesas Embedded (AArch64)

Gunakan script `build.sh` yang telah disediakan untuk mencegah bentrok antara library PC host (x86_64) dan library target (AArch64). Script ini akan mengarahkan `pkg-config` dan CMake untuk murni menggunakan ekosistem ARM64.

```bash
chmod +x build.sh
./build.sh
```

**⚠️ Penting saat Deploy ke Renesas:**
1. Salin *executable* `lvglkamera` hasil *build* ke board Renesas.
2. Salin file `lib/moil/aarch64/libmoildev.so` ke folder `/usr/lib` (atau folder library sistem lainnya) di dalam board Renesas Anda agar sistem dapat menemukannya saat aplikasi dijalankan.

---

## 🧠 Fitur Utama

- **Fisheye Dewarping Real-time:** Memanfaatkan library Moildev untuk mengubah gambar *fisheye* menjadi mode **Panorama** atau **Anypoint** secara *real-time*.
- **Optimasi Resolusi:** Menggunakan resolusi 1600x1200 untuk menjaga keseimbangan antara ketajaman detail kamera dan beban komputasi CPU.
- **UI Interaktif:** Parameter lensa (Alpha, Beta, Zoom) dapat disesuaikan langsung melalui *slider* LVGL secara instan. Tampilan panel konfigurasi akan menyesuaikan mode yang dipilih secara otomatis.
- **Hardware Stats Real-time:** Membaca status penggunaan CPU, Memory (RAM), dan GPU (via `nvidia-smi` jika tersedia) secara langsung.

---

## 🔧 Troubleshooting

- **Error: `undefined reference to moildev::...` saat linking**
  Pastikan `libmoildev.so` sudah berada di direktori arsitektur yang benar (`lib/moil/x86_64` atau `lib/moil/aarch64`) dan sesuai dengan target yang sedang di-*build*.
- **Error: `cannot open shared object file: No such file or directory` (Di Board Renesas)**
  Aplikasi tidak menemukan library Moildev. Pastikan Anda sudah memindahkan `libmoildev.so` (versi AArch64) ke direktori library sistem seperti `/usr/lib` pada *board* target Anda.
- **Kamera tidak muncul (Lag/Freeze)**
  Periksa *pipeline* GStreamer Anda. Pastikan `/dev/video0` tersedia dan memiliki akses baca/tulis (`sudo chmod 666 /dev/video0`).