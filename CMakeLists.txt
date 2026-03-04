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

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/moil)

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
file(GLOB UI_ALL "ui/*.c" "ui/*.cpp")
set(UI_SOURCES "")
foreach(_f ${UI_ALL})
    # Exclude the old ui.c (we will use ui_new.c as the generated UI)
    if(NOT _f MATCHES ".*/ui\.c$")
        list(APPEND UI_SOURCES ${_f})
    endif()
endforeach()

set(SRC_APP src/main_stats.cpp src/platform_stats.cpp)

# Buat executable
add_executable(lvglkamera ${SRC_APP} ${UI_SOURCES})

# ==============================================================================
# 4. LINKING & DIRECTORIES
# ==============================================================================

# [TAMBAHAN BARU] Cari library moildev di dalam folder moil
find_library(MOILDEV_LIB 
    NAMES moildev 
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/moil 
    NO_DEFAULT_PATH # Paksa CMake mencari hanya di folder moil Anda
)

if(NOT MOILDEV_LIB)
    message(FATAL_ERROR "Library Moildev (libmoildev.so / libmoildev.a) tidak ditemukan di folder moil!")
endif()

target_link_libraries(lvglkamera 
    lvgl 
    lvgl::examples 
    ${PLATFORM_LIBS} 
    ${GST_LIBRARIES}
    ${OpenCV_LIBS}
    ${MOILDEV_LIB}  # <--- Tautkan library moildev yang ditemukan
    pthread
    dl
)

target_include_directories(lvglkamera PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/moil  # <--- Pastikan folder header moil ikut disertakan
)

set_target_properties(lvglkamera PROPERTIES BUILD_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/moil")