#!/bin/bash

# Pastikan script berhenti jika ada perintah yang error
set -e

echo "==============================================="
echo "MEMULAI PROSES CROSS-COMPILE UNTUK RENESAS..."
echo "Target: Yocto SDK (aarch64-poky-linux) RZ/V2H"
echo "==============================================="

# 1. AKTIFKAN ENVIRONMENT YOCTO LANGSUNG DI SINI
# Menggunakan path dari script lama Anda
source /opt/poky/3.1.31/environment-setup-aarch64-poky-linux 

BUILD_DIR="build_renesas"

# 2. BERSIHKAN CACHE LAMA
if [ -d "$BUILD_DIR" ]; then
    echo "Membersihkan cache build lama..."
    rm -rf $BUILD_DIR
fi

mkdir -p $BUILD_DIR
cd $BUILD_DIR

# 3. JALANKAN CMAKE
# Memadukan flag lama Anda (-DV2H=ON & toolchain) dengan flag project ini (-DTARGET_RENESAS=ON)
# Catatan: Pastikan file runtime.cmake memang ada di folder toolchain/ di project ini.
# (Saya juga memperbaiki typo 'cmake cmake' dari script lama Anda menjadi 'cmake' saja)

cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain/runtime.cmake \
      -DTARGET_RENESAS=ON \
      -DV2H=ON \
      ..

# 4. KOMPILASI
make -j$(nproc)

echo "==============================================="
echo "BUILD SUKSES!"
echo "==============================================="