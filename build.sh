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

# Set environment agar pkg-config mencari library ARM, bukan x86
# Pastikan Anda sudah menginstall libdrm-dev:arm64
export PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig

# Jalankan CMake
cmake -DTARGET_RENESAS=ON \
      -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
      -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
      -DCMAKE_SYSTEM_NAME=Linux \
      -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
      -DCMAKE_C_FLAGS="-march=armv8-a" \
      -DCMAKE_CXX_FLAGS="-march=armv8-a" \
      ..

# Kompilasi
make -j$(nproc)

echo "==============================================="
echo "BUILD SUKSES!"
echo "==============================================="