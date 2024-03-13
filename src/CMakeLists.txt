cmake_minimum_required(VERSION 3.27)

add_subdirectory(util)
add_subdirectory(core)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
add_executable("${PROJECT_NAME}" main.cpp)

# TODO this is dirty: fix & make it work on 'any' system
include_directories(
    /usr/include/ostree-1
    /usr/include/glib-2.0/
    /usr/lib/x86_64-linux-gnu/glib-2.0/include
    /usr/lib64/glib-2.0/include/
)

target_link_libraries(ostree-tui
    PRIVATE ostui::core
    PRIVATE ostui::util
    PRIVATE ostree-1
    PRIVATE glib-2.0
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
    PRIVATE clip
)
