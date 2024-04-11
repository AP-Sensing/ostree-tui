cmake_minimum_required(VERSION 3.27)

add_subdirectory(util)
add_subdirectory(core)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
add_executable("${PROJECT_NAME}" main.cpp)

target_link_libraries(ostree-tui
    PRIVATE ostui::core
    PRIVATE ostui::util
    PRIVATE ftxui::screen
    PRIVATE ftxui::dom
    PRIVATE ftxui::component
    PRIVATE clip
)
