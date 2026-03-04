#!/bin/bash

# Pastikan script berhenti jika ada perintah yang error
set -e

echo "==============================================="
echo "MEMULAI PROSES CROSS-COMPILE UNTUK RENESAS..."
echo "Target: AArch64 (DRM + Evdev)"
echo "==============================================="

BUILD_DIR="build_renesas"

# Hapus folder build lama untuk memastikan cache bersih
if [ -d "$BUILD_DIR" ]; then
    echo "Membersihkan cache build lama..."
    rm -rf $BUILD_DIR
fi

mkdir -p $BUILD_DIR
cd $BUILD_DIR

# 1. PENGAMANAN PKG-CONFIG (Mencegah nyasar ke library x86_64 PC Anda)
export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig
export PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=/

# 2. Jalankan CMake dengan PENGAMANAN OPENCV
cmake -DTARGET_RENESAS=ON \
      -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
      -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
      -DCMAKE_SYSTEM_NAME=Linux \
      -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
      -DCMAKE_C_FLAGS="-march=armv8-a" \
      -DCMAKE_CXX_FLAGS="-march=armv8-a" \
      -DOpenCV_DIR=/usr/lib/aarch64-linux-gnu/cmake/opencv4 \
      ..

# Kompilasi
make -j$(nproc)

echo "==============================================="
echo "BUILD SUKSES!"
echo "==============================================="