# lvglkamera — LVGL + Kamera (desktop example)

Proyek ini adalah contoh aplikasi C++ yang menggabungkan LVGL (Light and Versatile Graphics Library) dengan backend desktop (SDL2) dan GStreamer untuk input/video pipeline. Nama target CMake: `lvglkamera` (executable: `lvglkamera`).

README ini fokus pada pengaturan lingkungan, dependensi konkret (sesuai `CMakeLists.txt`), dan cara menjalankan proyek pada Linux/macOS. Untuk target embedded, lihat bagian "Toolchain embedded" di bawah.

Ringkasan struktur penting

- `CMakeLists.txt` — konfigurasi build utama. Menemukan SDL2, pkg-config dan GStreamer, lalu menambahkan subdirectory `lib/lvgl/`.
- `src/main.cpp` — entry point aplikasi.
- `ui/` — sumber UI (generated or hand-written): `ui.c`, `ui.h`, `ui_events.cpp`.
- `lib/lvgl/` — sumber LVGL yang disertakan (library, header, utilitas). Periksa `lib/lvgl/lv_conf.h` atau `lib/lvgl/lv_conf_template.h` untuk konfigurasi LVGL.
- `build/` — direktori build CMake (artefak hasil kompilasi).

Dependensi utama (dipakai di CMakeLists.txt)

- SDL2 (penggunaan desktop backend)
- pkg-config (digunakan untuk menemukan GStreamer)
- GStreamer (gstreamer-1.0, gstreamer-app-1.0)
- CMake, compiler C++

Paket yang direkomendasikan (Debian/Ubuntu)

Instal paket berikut untuk memudahkan build dan menjalankan contoh ini:

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config git python3 python3-pip \
	libsdl2-dev libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
	gstreamer1.0-plugins-good gstreamer1.0-tools libpng-dev libjpeg-dev
```

- `libsdl2-dev` — header & libs SDL2
- `libgstreamer1.0-dev`, `libgstreamer-plugins-base1.0-dev` — header & libs GStreamer
- `gstreamer1.0-plugins-good` dan `gstreamer1.0-tools` — plugin & tools yang sering dibutuhkan untuk pipeline contoh

macOS (Homebrew)

```bash
brew update
brew install cmake pkg-config sdl2 gst-plugins-base gst-plugins-good gst-libav
```

Dependensi opsional: OpenCV

Jika Anda ingin memproses frame kamera menggunakan OpenCV, pasang `libopencv-dev` (Debian) atau `opencv` via Homebrew. Untuk mengaktifkan OpenCV di build, tambahkan di `CMakeLists.txt`:

```cmake
find_package(OpenCV REQUIRED)
target_link_libraries(lvgl_kamera PRIVATE ${OpenCV_LIBS})
```

Lalu rebuild. Pada CMakeLists saat ini ada komentar "Tambahkan ${OpenCV_LIBS} di baris ini" sebagai petunjuk.

Build (quick start)

1. Buat direktori build dan konfigurasikan:

```bash
cd /path/to/project/root
mkdir -p build
cd build
cmake ..
```

Jika CMake gagal menemukan SDL2 atau GStreamer, periksa bahwa `pkg-config` telah terpasang dan development package untuk SDL2/GStreamer tersedia.

2. Build:

```bash
cmake --build . -- -j$(nproc)
```

Executable yang dihasilkan biasanya bernama `lvglkamera` (lihat `add_executable` di `CMakeLists.txt`). Di workspace sebelumnya ada contoh `UnicornKamera` — pada repo Anda target saat ini adalah `lvglkamera`.

Menjalankan aplikasi

Jika build sukses, jalankan:

```bash
./build/lvglkamera
```

Catatan: jalankan dari direktori project root atau `build/` tergantung bagaimana relative asset paths dikelola.

Integrasi LVGL — hal-hal yang perlu diperhatikan

- Konfigurasi LVGL: file `lv_conf.h` mengontrol banyak opsi LVGL (biasanya berada di `lib/lvgl/`). Jika Anda perlu menyesuaikan driver display, font, atau fitur, edit `lib/lvgl/lv_conf.h` atau `lib/lvgl/lv_conf_template.h` sesuai dokumentasi LVGL.
- `add_subdirectory(lib/lvgl)` di `CMakeLists.txt` akan membangun LVGL sebagai target dan kemudian linked ke `lvglkamera`.
- UI: folder `ui/` berisi sumber antarmuka. `CMakeLists.txt` sudah mengambil `ui/*.c` & `ui/*.cpp` ke dalam target.

Menambahkan dukungan platform/driver baru

- Desktop (saat ini): backend SDL2 dipakai untuk menampilkan layar LVGL pada jendela desktop.
- Embedded: gunakan toolchain dan driver display/input spesifik; konfigurasi LVGL dan `lv_conf.h` harus diadaptasi.

Troubleshooting

- CMake tidak menemukan SDL2 atau GStreamer: pastikan `pkg-config` terpasang dan development package (mis. `libsdl2-dev`, `libgstreamer1.0-dev`) terinstall.
- Linking error terkait LVGL: pastikan subdirektori `lvgl/` dibangun dan target `lvgl` tersedia (CMake output akan menunjukkan targets yang dibuat).
- GStreamer pipeline errors: jalankan `gst-launch-1.0` atau `gst-inspect-1.0` untuk memverifikasi plugin yang tersedia.

Tips pengembangan

- Edit UI di `ui/` dan implementasikan event di `ui_events.cpp`.
- Tempatkan logika aplikasi utama di `src/main.cpp`.
- Jika menambahkan OpenCV atau library lain, pasang package `-dev` yang sesuai dan link dengan CMake.

Realtime system stats (CPU / Memory / GPU)

Proyek sekarang menyertakan tampilan statistik realtime di UI (CPU% dan Memory usage selalu ditampilkan; GPU% akan ditampilkan jika tersedia melalui `nvidia-smi`). Implementasi:

- Sampling CPU/memory: dibaca dari `/proc/stat` dan `/proc/meminfo` (Linux).
- Sampling GPU: program mencoba memanggil `nvidia-smi` dan membaca `utilization.gpu`; jika tidak ada `nvidia-smi` atau GPU tidak terdeteksi, UI akan menampilkan `GPU: N/A`.
- Kode terkait: `src/platform_stats.*` (sampling) dan `src/main_stats.cpp` (background sampler + LVGL labels).

Jika Anda ingin mengaktifkan dukungan GPU lain (AMD, vendor khusus), sesuaikan fungsi `read_gpu_via_nvidia_smi()` di `src/platform_stats.cpp` atau tambahkan deteksi/pengambilan data GPU lain.

Contributing

- Buat branch fitur lalu push perubahan. Sertakan deskripsi yang jelas di PR.

Lisensi

Periksa `lib/lvgl/LICENCE.txt` dan file lisensi lain yang menyertai pustaka untuk informasi lisensi lengkap.

---

Jika Anda mau, saya bisa:

- Menambahkan instruksi khusus platform (mis. ESP-IDF / ESP32) dan contoh CMake toolchain file.
- Menambahkan contoh modifikasi `CMakeLists.txt` untuk mengaktifkan OpenCV secara otomatis saat tersedia.

Beritahu saya fitur mana yang ingin Anda prioritaskan dan saya akan perbarui README lagi.
