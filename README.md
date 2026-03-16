# lvglkamera — LVGL + Kamera Fisheye + GStreamer + OpenGL/GLSL Shader

Proyek ini adalah aplikasi C++ performa tinggi yang menggabungkan antarmuka **LVGL (Light and Versatile Graphics Library)**, **GStreamer** untuk penangkapan *pipeline* kamera, **OpenCV** untuk pemrosesan gambar, dan **OpenGL/GLSL Shader** untuk de-warping lensa *fisheye* (mode **Panorama** & **Anypoint**).

Versi terbaru proyek ini **tidak lagi bergantung pada Moildev** untuk proses dewarping. Transformasi *fisheye* sekarang dikerjakan oleh **shader GLSL** melalui kelas `ShaderRenderer`, lalu hasilnya ditampilkan ke widget image LVGL.

Aplikasi ini mendukung dua mode *build* utama:

1. **PC Desktop Simulator:** Menggunakan SDL2 untuk *rendering* UI di lingkungan lokal, dengan OpenGL context untuk shader.
2. **Embedded Linux (Renesas/Poky):** Menggunakan DRM/Evdev untuk *rendering* langsung ke layar perangkat keras pada distribusi berbasis **Yocto (aarch64-poky-linux)**.

---

## 📂 Ringkasan Struktur Direktori

* `CMakeLists.txt` — Konfigurasi build utama.
* `src/`

  * `main.cpp` — *Entry point* aplikasi.
  * `platform_stats.cpp` — Logika pembacaan statistik CPU, MEM, GPU.
* `ui/`

  * `ui_new.c` — Inisialisasi UI LVGL.
  * `screens.c` — Layout tampilan utama.
  * `ui_events.cpp` — Logika kamera, *event handler*, kontrol Panorama/Anypoint, FPS, dan interaksi mouse.
* `utils/`

  * `shader_render.cpp`
  * `shader_render.hpp` — Wrapper OpenGL untuk kompilasi shader, render ke framebuffer, dan output buffer untuk LVGL.
* `moil/`

  * `anypoint_gl.frag` — Shader GLSL utama untuk transformasi Panorama & Anypoint.
* `lib/lvgl/` — Sumber *library* LVGL.
* `lv_conf.h` — Konfigurasi LVGL untuk backend SDL2 / DRM.

---

## 🎨 Editor GUI: EEZ Studio

Aplikasi ini didesain menggunakan **EEZ Studio**. Editor ini memungkinkan pengembangan UI LVGL secara visual (*drag-and-drop*), lalu hasilnya digabungkan dengan logika C/C++ di proyek.

### Instalasi EEZ Studio (Linux)

1. Unduh file AppImage terbaru dari [EEZ Studio GitHub Releases](https://github.com/eez-open/studio/releases).
2. Berikan izin eksekusi pada file tersebut:

   ```bash
   chmod +x EEZ-Studio-x86_64.AppImage
   ```
3. Pindahkan ke folder `/opt` (opsional, agar rapi):

   ```bash
   sudo mkdir -p /opt/eez-studio
   sudo mv EEZ-Studio-x86_64.AppImage /opt/eez-studio/eez-studio.AppImage
   ```

### Menjalankan EEZ Studio dari Terminal

Agar Anda bisa memanggil `eez-studio` langsung dari direktori mana pun di terminal, buatlah *symbolic link*:

```bash
sudo ln -s /opt/eez-studio/eez-studio.AppImage /usr/local/bin/eez-studio
```

Sekarang Anda cukup mengetik perintah ini untuk membuka editor:

```bash
eez-studio
```

---

## 🛠️ Dependensi Utama

Proyek ini membutuhkan pustaka berikut:

* **LVGL v9** (Termasuk dalam *source*)
* **SDL2** (Hanya untuk PC Simulator)
* **libdrm** (Hanya untuk target embedded/Renesas)
* **GStreamer 1.0** & `gst-plugins-base`
* **OpenCV 4**
* **OpenGL / GLES**
* **GLEW**
* **Shader GLSL internal** di `moil/anypoint_gl.frag`

### Instalasi Dependensi PC (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config git \
    libsdl2-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-good gstreamer1.0-tools \
    libopencv-dev \
    libglew-dev \
    libgl1-mesa-dev libegl1-mesa-dev
```

### Dependensi Tambahan untuk Target Embedded

```bash
sudo apt install -y libdrm-dev
```

---

## 🚀 Cara Build & Menjalankan

### Opsi 1: Build untuk PC Simulator (x86_64)

Gunakan mode ini untuk menguji UI, shader, dan logika Panorama/Anypoint di PC Anda.

```bash
mkdir -p build && cd build
cmake ..
cmake --build . -j$(nproc)
./lvglkamera
```

### Opsi 2: Build untuk Renesas Embedded (Yocto/Poky)

Gunakan Yocto SDK Toolchain untuk *cross-compile*. Script `build.sh` dapat digunakan jika proyek Anda memang menyediakannya.

1. **Aktifkan SDK Yocto:**

   ```bash
   source /opt/poky/x.y.z/environment-setup-aarch64-poky-linux
   ```

2. **Jalankan Build Script / CMake:**

   ```bash
   chmod +x build.sh
   ./build.sh
   ```

Atau build manual:

```bash
mkdir -p build && cd build
cmake ..
cmake --build . -j$(nproc)
```

**⚠️ Penting saat Deploy ke Embedded Board:**

1. Salin *executable* `lvglkamera` hasil build ke board.
2. Pastikan seluruh dependency runtime tersedia di board.
3. Pastikan file shader `moil/anypoint_gl.frag` ikut tersedia pada path yang sesuai dengan aplikasi.

---

## 🧠 Fitur Utama

* **Fisheye Dewarping Real-time:** Transformasi gambar *fisheye* menjadi **Panorama** atau **Anypoint** secara instan menggunakan shader GLSL.
* **Optimasi Resolusi:** Default kamera berjalan pada **1600x1200** untuk keseimbangan performa dan kualitas visual.
* **UI Interaktif:** Parameter lensa (`Alpha`, `Beta`, `Zoom`) dapat disesuaikan via *slider*.
* **Mode Dinamis:** Label konfigurasi berubah otomatis sesuai mode aktif.
* **Multi-Anypoint:** Mendukung **4 anypoint view** sekaligus.
* **Interaksi Langsung dengan Mouse:**

  * **Klik** untuk memilih anypoint aktif
  * **Drag** untuk mengubah arah pandang (`Alpha/Beta`)
  * **Scroll wheel** untuk *zoom in / zoom out*
* **Sidebar Preview:** Preview kecil mode aktif di panel kiri.
* **Hardware Stats:** Monitoring penggunaan **CPU**, **MEM**, **GPU**, dan **FPS**.
* **GPU Best-Effort Monitoring:** Statistik GPU dibaca sebisa mungkin tergantung driver/platform, dan dapat menampilkan `N/A`.

---

## 🖼️ Arsitektur Alur Data

Aplikasi ini menggunakan alur pemrosesan berikut:

```text
Kamera (GStreamer)
-> frame raw
-> diproses OpenGL / GLSL shader
-> hasil dibaca ke CPU (glReadPixels)
-> dikonversi OpenCV ke RGB565
-> ditampilkan di LVGL image widget
```

Jadi:

* **LVGL** digunakan untuk UI
* **GStreamer** digunakan untuk capture kamera
* **OpenCV** digunakan untuk resize / flip / konversi warna
* **OpenGL/GLSL** digunakan untuk pemrosesan Panorama & Anypoint

---

## 🖱️ Kontrol Pengguna

### Mode Tampilan

* **Original** — Menampilkan preview asli
* **Panorama** — Mengaktifkan mode panorama
* **Anypoint** — Mengaktifkan mode anypoint

### Interaksi pada Panel Anypoint

* **Klik panel anypoint** → memilih anypoint aktif
* **Drag horizontal** → mengubah `Beta`
* **Drag vertikal** → mengubah `Alpha`
* **Scroll mouse** → mengubah `Zoom`

Slider konfigurasi akan mengikuti anypoint yang sedang dipilih.

---

## 📊 Monitoring Statistik

Aplikasi menampilkan:

* **CPU**
* **MEM**
* **GPU**
* **FPS**

Posisi statistik di UI saat ini diletakkan di sidebar, di bawah panel konfigurasi.

### Catatan penting untuk GPU

Label `GPU` saat ini menggunakan pembacaan **best effort** melalui backend Linux/NVIDIA.
Karena aplikasi menggunakan **OpenGL graphics workload**, bukan CUDA compute, maka pada beberapa sistem:

* nilai GPU bisa tampil normal
* nilai GPU bisa tampil `N/A`
* ini **bukan berarti shader GPU tidak digunakan**

Backend pembacaan GPU saat ini mencoba:

* `nvidia-smi pmon`
* `nvidia-smi --query-compute-apps`
* fallback `nvidia-smi --query-gpu=utilization.gpu`

Jadi:

> `GPU: N/A` **bukan berarti shader GPU tidak dipakai**.
> Itu hanya berarti backend monitoring GPU tidak dapat membaca utilitas GPU pada sistem tersebut.

---

## 🔧 Troubleshooting

* **Error: `Yocto SDK environment belum diaktifkan!`**
  Pastikan Anda sudah menjalankan perintah `source` pada file `environment-setup-...` milik SDK Renesas Anda sebelum menjalankan `build.sh`.

* **Error: Shader gagal di-load / `Gagal init shader renderer`**
  Pastikan file shader tersedia pada path:

  ```bash
  ls moil/anypoint_gl.frag
  ```

* **Error: `free(): double free detected`**
  Periksa implementasi `platform_stats.cpp`, khususnya penggunaan `popen()` dan `pclose()`. Pastikan `FILE*` dari `popen()` tidak ditutup dua kali.

* **GPU tampil `N/A`**
  Ini normal jika:

  * sistem bukan NVIDIA
  * `nvidia-smi` tidak tersedia
  * workload OpenGL tidak terdeteksi sebagai compute app

  Cek dengan:

  ```bash
  which nvidia-smi
  nvidia-smi
  ```

* **Kamera Lag / Freeze**
  Pastikan user memiliki izin akses ke device:

  ```bash
  sudo chmod 666 /dev/video0
  ```

  atau jalankan sebagai root.

* **Kamera tidak muncul**
  Cek daftar device:

  ```bash
  ls /dev/video*
  v4l2-ctl --list-devices
  ```

  Coba pipeline dasar:

  ```bash
  gst-launch-1.0 v4l2src device=/dev/video0 ! videoconvert ! autovideosink
  ```

* **Anypoint terlihat pecah / blur**
  Penyebab umum:

  * resolusi source terlalu kecil
  * zoom terlalu tinggi
  * ukuran widget terlalu besar

  Solusi:

  * turunkan zoom
  * naikkan resolusi kamera jika tersedia
  * gunakan oversampling render shader

* **Scroll mouse tidak bekerja untuk zoom**
  Pastikan build desktop sudah memasang bridge event SDL mouse wheel ke handler anypoint.

---

## 🧪 Catatan Platform

### Desktop Linux

Mode desktop menggunakan:

* **SDL2** untuk window & input
* **OpenGL** untuk shader
* cocok untuk debugging dan tuning UI

### Renesas / Embedded Linux

Mode embedded menggunakan:

* **DRM / Evdev**
* cocok untuk board tanpa desktop environment

### NVIDIA Orin Nano / Jetson

Arsitektur aplikasi ini juga cocok untuk diuji di Jetson, dengan beberapa penyesuaian:

* backend display
* stack kamera
* monitoring GPU
* kemungkinan migrasi dari SDL ke EGL/DRM jika dibutuhkan

---

## 🛣️ Rencana Pengembangan Selanjutnya

Beberapa pengembangan yang dapat dilakukan:

* backend monitoring GPU khusus Jetson / embedded
* port ke EGL/DRM penuh tanpa SDL
* dukungan CSI camera
* preset posisi anypoint yang bisa disimpan
* fullscreen / kiosk mode
* dukungan input sentuh
* peningkatan kualitas render anypoint melalui oversampling yang dapat dikonfigurasi
