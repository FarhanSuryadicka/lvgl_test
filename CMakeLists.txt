cmake_minimum_required(VERSION 3.10)
project(lvglkamera C CXX)

# ==============================================================================
# 0. CROSS-COMPILE ISOLATION (WAJIB UNTUK TARGET EMBEDDED)
# ==============================================================================
option(TARGET_RENESAS "Build for Renesas Embedded Linux (DRM/Evdev)" OFF)
option(V2H "Build specifically for Renesas V2H" OFF) 

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
# 1. PENCARIAN LIBRARY GLOBAL (GStreamer & OpenCV)
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
# 2. KONFIGURASI KHUSUS PLATFORM & OPENGL (SHADER)
# ==============================================================================
if(TARGET_RENESAS)
    message(STATUS "=======================================================")
    message(STATUS "[Unicorn] MENGONFIGURASI BUILD UNTUK RENESAS (AARCH64)")
    if(V2H)
        message(STATUS "[Unicorn] Mode V2H Diaktifkan")
    endif()
    message(STATUS "=======================================================")
    
    add_compile_definitions(TARGET_RENESAS=1)
    
    # Cari library DRM target
    pkg_check_modules(DRM REQUIRED libdrm)
    include_directories(${DRM_INCLUDE_DIRS})
    
    # Cari library OpenGL ES untuk Shader di Embedded
    pkg_check_modules(GLES REQUIRED glesv2)
    include_directories(${GLES_INCLUDE_DIRS})
    
    set(PLATFORM_LIBS ${DRM_LIBRARIES} ${GLES_LIBRARIES})
else()
    message(STATUS "=======================================================")
    message(STATUS "[Unicorn] MENGONFIGURASI BUILD UNTUK PC SIMULATOR (SDL2)")
    message(STATUS "=======================================================")
    
    # Cari SDL2
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS})
    
    # Cari OpenGL dan GLEW untuk Shader di PC
    find_package(OpenGL REQUIRED)
    pkg_check_modules(GLEW REQUIRED glew)
    include_directories(${OpenGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
    
    set(PLATFORM_LIBS ${SDL2_LIBRARIES} ${OpenGL_LIBRARIES} ${GLEW_LIBRARIES})
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

# Pastikan ui_events.cpp dan shader_render.cpp masuk ke dalam SRC_APP
set(SRC_APP 
    src/main_stats.cpp 
    src/platform_stats.cpp 
    utils/shader_render.cpp
)

# Buat executable
add_executable(lvglkamera ${SRC_APP} ${UI_SOURCES})

# ==============================================================================
# 4. LINKING & DIRECTORIES
# ==============================================================================

# 1. Tentukan folder library dan nama library berdasarkan target arsitektur
if(TARGET_RENESAS)
    # Arahkan ke folder 'ren' untuk static library Renesas Anda
    set(MOIL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/moil/ren")
    set(MOIL_LIB_NAME "moildevren") # Mencari libmoildevren.a
    message(STATUS "[Info] Menggunakan library Static Moildev Renesas dari: ${MOIL_LIB_DIR}")
else()
    # Arahkan ke folder x86_64 untuk PC Simulator
    set(MOIL_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/moil/x86_64")
    set(MOIL_LIB_NAME "moildev") # Mencari libmoildev.so
    message(STATUS "[Info] Menggunakan library Moildev x86_64 dari: ${MOIL_LIB_DIR}")
endif()

# 2. Cari library moildev di dalam folder yang sudah ditentukan
find_library(MOILDEV_LIB 
    NAMES ${MOIL_LIB_NAME}
    PATHS ${MOIL_LIB_DIR} 
    NO_DEFAULT_PATH # Paksa CMake mencari hanya di folder spesifik tersebut
)

if(NOT MOILDEV_LIB)
    message(FATAL_ERROR "Library Moildev (${MOIL_LIB_NAME}) tidak ditemukan di ${MOIL_LIB_DIR}!")
endif()

# 3. Tautkan library ke aplikasi
target_link_libraries(lvglkamera 
    lvgl 
    lvgl::examples 
    ${PLATFORM_LIBS} 
    ${GST_LIBRARIES}
    ${OpenCV_LIBS}
    ${MOILDEV_LIB}  # <--- Tautkan static/shared library moildev yang ditemukan
    pthread
    dl
)

# 4. Tentukan direktori Include
target_include_directories(lvglkamera PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/moil
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# 5. Atur RPATH dengan aman
if(NOT TARGET_RENESAS)
    # Untuk PC Simulator (x86_64), izinkan RPATH membaca dari folder lokal agar bisa langsung di-run
    set_target_properties(lvglkamera PROPERTIES BUILD_RPATH "${MOIL_LIB_DIR}")
else()
    # Untuk Renesas (Static Library .a), RPATH tidak lagi dibutuhkan karena library sudah menyatu.
    set_target_properties(lvglkamera PROPERTIES SKIP_BUILD_RPATH TRUE)
endif()