#!/bin/bash

# Pastikan script berhenti jika ada perintah yang error
set -e

echo "==============================================="
echo "MEMULAI PROSES CROSS-COMPILE UNTUK RENESAS..."
echo "Target: Yocto SDK (aarch64-poky-linux)"
echo "==============================================="

# 1. PENGAMANAN: Cek apakah Yocto SDK environment sudah di-source
if [ -z "$OECORE_NATIVE_SYSROOT" ]; then
    echo "==============================================="
    echo "[ERROR] Lingkungan Yocto SDK belum diaktifkan!"
    echo "Silakan jalankan perintah source SDK Anda terlebih dahulu."
    echo "Contoh: source /opt/poky/3.1.x/environment-setup-aarch64-poky-linux"
    echo "Lalu jalankan ulang ./build.sh"
    echo "==============================================="
    exit 1
fi

BUILD_DIR="build_renesas"

# Hapus folder build lama untuk memastikan cache bersih
if [ -d "$BUILD_DIR" ]; then
    echo "Membersihkan cache build lama..."
    rm -rf $BUILD_DIR
fi

mkdir -p $BUILD_DIR
cd $BUILD_DIR

# 2. JALANKAN CMAKE
# Karena Yocto SDK sudah otomatis mengatur $CC, $CXX, sysroot, dan pkg-config,
# kita tidak perlu lagi mendefinisikannya secara manual. Cukup panggil cmake!
cmake -DTARGET_RENESAS=ON ..

# 3. KOMPILASI
make -j$(nproc)

echo "==============================================="
echo "BUILD SUKSES!"
echo "==============================================="