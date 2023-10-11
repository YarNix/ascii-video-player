cmake_minimum_required(VERSION "3.13.4")

project(asciiVP CXX)
# Find the library
find_path(FFMPEG_EXEC_PATH "ffmpeg.exe")
if(NOT ${FFMPEG_EXEC_PATH} STREQUAL "FFMPEG_EXEC_PATH-NOTFOUND")
    set(FFMPEG_DIR "${FFMPEG_EXEC_PATH}/..")
    set(FFMPEG_INCLUDE_DIRS "${FFMPEG_DIR}/include")
    set(FFMPEG_LIB_DIRS "${FFMPEG_DIR}/lib")
    set(FFMPEG_LIBRARIES "avcodec" "avformat" "avutil" "swscale")
    message(STATUS "Found ffmpeg at ${FFMPEG_EXEC_PATH}")
else()
    message(FATAL_ERROR "Can't find ffmpeg. Make sure it's added to PATH")
endif()
# Adding sources
add_executable(asciiVP "video_player/main.cpp")
target_sources(asciiVP PRIVATE "video_player/asciiPlayer.cpp")
# Configure the project
target_include_directories(asciiVP PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(asciiVP PRIVATE ${FFMPEG_LIB_DIRS})
target_link_options(asciiVP PRIVATE)
target_link_libraries(asciiVP PRIVATE ${FFMPEG_LIBRARIES})
set_target_properties(asciiVP PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE})

if(MSVC)
    add_compile_options(/W4 /permissive- /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wcast-align -Wsign-conversion -Wnull-dereference)
endif()

set(CMAKE_SKIP_INSTALL_RULES ON)
set(CMAKE_SKIP_PACKAGE_RULS ON)