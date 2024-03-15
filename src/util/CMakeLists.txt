cmake_minimum_required(VERSION 3.27)

file(GLOB_RECURSE util_sources *.cpp *.h)

add_library(util STATIC ${util_sources})

target_link_libraries(util
  PRIVATE clip
)

target_include_directories(util PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
add_library(ostui::util ALIAS util)