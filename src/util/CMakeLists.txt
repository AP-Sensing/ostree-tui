cmake_minimum_required(VERSION 3.27)

find_package(PkgConfig REQUIRED)

set(ENV{PKG_CONFIG_PATH} "/usr/lib/pkgconfig")
pkg_check_modules(glib-2.0 REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(gio-2.0 REQUIRED IMPORTED_TARGET gio-2.0)
pkg_check_modules(gobject-2.0 REQUIRED IMPORTED_TARGET gobject-2.0)

file(GLOB_RECURSE util_sources *.cpp *.h)

add_library(util STATIC ${util_sources})

target_include_directories(util
    PUBLIC
    ${glib-2.0_INCLUDE_DIRS}
)

target_link_libraries(util
  PUBLIC PkgConfig::glib-2.0
         libostree
  PRIVATE PkgConfig::gio-2.0
          PkgConfig::gobject-2.0
          clip 
)

add_library(ostui::util ALIAS util)
