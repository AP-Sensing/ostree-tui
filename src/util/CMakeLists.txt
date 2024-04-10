cmake_minimum_required(VERSION 3.27)

file(GLOB_RECURSE util_sources *.cpp *.h)

add_library(util STATIC ${util_sources})

# TODO this is dirty: fix & make it work on 'any' system
include_directories(
    /usr/include/ostree-1
    /usr/include/glib-2.0/
    /usr/lib/x86_64-linux-gnu/glib-2.0/include
    /usr/lib64/glib-2.0/include/
)
target_link_libraries(util
  PRIVATE ostree-1
  PRIVATE glib-2.0
  PRIVATE clip
)

#target_include_directories(util
#  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#  /usr/include/ostree-1
#  /usr/include/glib-2.0/
#  /usr/lib/x86_64-linux-gnu/glib-2.0/include
#  /usr/lib64/glib-2.0/include/
#)

add_library(ostui::util ALIAS util)
