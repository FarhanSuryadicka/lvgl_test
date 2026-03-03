cmake_minimum_required(VERSION 3.10)
project(lvglkamera C CXX)

# ==============================================================================
# 0. CROSS-COMPILE ISOLATION (WAJIB UNTUK TARGET EMBEDDED)
# ==============================================================================
option(TARGET_RENESAS "Build for Renesas Embedded Linux (DRM/Evdev)" OFF)

if(TARGET_RENESAS)
    # Jangan biarkan CMake mencari program/library di folder host PC (x86_64)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
    
    # Gunakan pkg-config khusus cross-compile jika tersedia
    find_program(PKG_CONFIG_EXECUTABLE NAMES aarch64-linux-gnu-pkg-config pkg-config)
endif()

# Gunakan standar C++ modern
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# 1. PENCARIAN LIBRARY GLOBAL (GStreamer)
# ==============================================================================
find_package(PkgConfig REQUIRED)

pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)
include_directories(${GST_INCLUDE_DIRS})
link_directories(${GST_LIBRARY_DIRS})
add_definitions(${GST_CFLAGS_OTHER})

# ==============================================================================
# 2. KONFIGURASI KHUSUS PLATFORM
# ==============================================================================
if(TARGET_RENESAS)
    message(STATUS "=======================================================")
    message(STATUS "[Unicorn] MENGONFIGURASI BUILD UNTUK RENESAS (AARCH64)")
    message(STATUS "=======================================================")
    
    add_compile_definitions(TARGET_RENESAS=1)
    
    # Cari library DRM target
    pkg_check_modules(DRM REQUIRED libdrm)
    include_directories(${DRM_INCLUDE_DIRS})
    
    set(PLATFORM_LIBS ${DRM_LIBRARIES})
else()
    message(STATUS "=======================================================")
    message(STATUS "[Unicorn] MENGONFIGURASI BUILD UNTUK PC SIMULATOR (SDL2)")
    message(STATUS "=======================================================")
    
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    
    set(PLATFORM_LIBS ${SDL2_LIBRARIES})
endif()

# ==============================================================================
# 3. KONFIGURASI LVGL & SOURCE CODE UI
# ==============================================================================
# Pastikan folder lib/lvgl ada
add_subdirectory(lib/lvgl)

# Ambil semua file UI hasil ekspor
file(GLOB UI_SOURCES "ui/*.c" "ui/*.cpp")

# Buat executable
add_executable(lvglkamera src/main.cpp ${UI_SOURCES})

# ==============================================================================
# 4. LINKING & DIRECTORIES
# ==============================================================================
target_link_libraries(lvglkamera 
    lvgl 
    lvgl::examples 
    ${PLATFORM_LIBS} 
    ${GST_LIBRARIES}
    pthread  # Tambahkan pthread secara eksplisit untuk keamanan
    dl
)

target_include_directories(lvglkamera PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
)