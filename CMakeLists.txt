cmake_minimum_required(VERSION "3.19.2")

project(asciiVP)
add_executable(asciiVP "video_player/main.cpp")
target_sources(asciiVP PRIVATE "video_player/asciiPlayer.cpp")
target_include_directories(asciiVP PRIVATE "dependencies/ffmpeg-n6.0-gpl-shared/include")
target_link_directories(asciiVP PRIVATE "dependencies/ffmpeg-n6.0-gpl-shared/lib")
target_link_libraries(asciiVP PRIVATE "libavcodec" "libavformat" "libswscale" "avutil")
set_target_properties(asciiVP PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_NAME})

if(MSVC)
    add_compile_options(/W4 /permissive- /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wformat=2 -Wcast-align -Wsign-conversion -Wnull-dereference)
endif()

set(CMAKE_SKIP_BUILD_RULES ON)
set(CMAKE_SKIP_INSTALL_RULES ON)
set(CMAKE_SKIP_PACKAGE_RULS ON)