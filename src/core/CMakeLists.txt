cmake_minimum_required(VERSION 3.27)
                
set(ostree-tui_include_dirs "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(ostree-tui_core commit.cpp  
                            commit.hpp
                            footer.cpp
                            footer.hpp
                            manager.cpp
                            manager.hpp
                            OSTreeTUI.cpp
                            OSTreeTUI.hpp)

target_link_libraries(ostree-tui_core
  PRIVATE clip
  PRIVATE ostui::util
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)
target_include_directories(ostree-tui_core
  PUBLIC ${ostree-tui_include_dirs}
  # TODO dirty
  PUBLIC ../../build/_deps/clip-src
)

add_library(ostui::core ALIAS ostree-tui_core)
