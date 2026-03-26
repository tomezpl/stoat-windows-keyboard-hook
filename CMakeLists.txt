cmake_minimum_required(VERSION 3.26.3)
project(stoat_for_desktop_kbhook)

set(CMAKE_CXX_STANDARD 20)

add_library(kbhook_hook SHARED hook.cpp)
#add_library(librawinput STATIC 3rdparty/rawinput/rawinput_common.c 3rdparty/rawinput/rawinput_win32.c)
#target_include_directories(librawinput PUBLIC 3rdparty/rawinput)
add_executable(kbhook_app WIN32 main.cpp)
target_link_libraries(kbhook_app kbhook_hook)
#target_link_libraries(kbhook_app librawinput)
