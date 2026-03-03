cmake_minimum_required(VERSION 3.10)
project(lvglkamera C CXX)

set(CMAKE_CXX_STANDARD 14)

# Cari SDL2 dan (GStreamer)
find_package(SDL2 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)

include_directories(${SDL2_INCLUDE_DIRS} ${GST_INCLUDE_DIRS})
link_directories(${GST_LIBRARY_DIRS})
add_definitions(${GST_CFLAGS_OTHER})

add_subdirectory(lvgl)

file(GLOB UI_SOURCES "ui/*.c" "ui/*.cpp")

add_executable(lvglkamera src/main.cpp ${UI_SOURCES})

# Tambahkan ${OpenCV_LIBS} di baris ini
target_link_libraries(lvglkamera lvgl lvgl::examples ${SDL2_LIBRARIES} ${GST_LIBRARIES})
target_include_directories(lvglkamera PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lvglkamera PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ui)