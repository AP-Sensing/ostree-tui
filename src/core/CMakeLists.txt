cmake_minimum_required(VERSION 3.27)

file(GLOB_RECURSE ostree_sources *.cpp *.h)
set(ostree-tui_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(ostree-tui_core STATIC ${ostree_sources})

target_link_libraries(ostree-tui_core
  PRIVATE ostui::util
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE clip
)
target_include_directories(ostree-tui_core PUBLIC ${ostree-tui_include_dirs})
add_library(ostui::core ALIAS ostree-tui_core)
