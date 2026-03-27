cmake_minimum_required(VERSION 3.26.3)
project(stoat_for_desktop_kbhook)

set(CMAKE_CXX_STANDARD 20)

add_library(kbhook_hook SHARED hook.cpp)
add_executable(kbhook_app WIN32 main.cpp)
target_link_libraries(kbhook_hook "Ws2_32.lib")
target_link_libraries(kbhook_app kbhook_hook)
