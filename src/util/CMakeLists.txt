cmake_minimum_required(VERSION 3.27)

find_package(PkgConfig REQUIRED)
pkg_check_modules(glib-2.0 REQUIRED IMPORTED_TARGET glib-2.0)

file(GLOB_RECURSE util_sources *.cpp *.h)

add_library(util STATIC ${util_sources})

target_include_directories(util
    PUBLIC
    ${glib-2.0_INCLUDE_DIRS}
    # TODO this is dirty:
    /usr/include/ostree-1
)

target_link_libraries(util
  PUBLIC PkgConfig::glib-2.0
  # PkgConfig::glib-2.0 not working properly -> gio & gobject manually added
  PUBLIC /usr/lib64/libgio-2.0.so
  PUBLIC /usr/lib64/libgobject-2.0.so

  PRIVATE ostree-1
  PRIVATE clip
)

add_library(ostui::util ALIAS util)
