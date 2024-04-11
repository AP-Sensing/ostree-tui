cmake_minimum_required(VERSION 3.27)

find_package(PkgConfig REQUIRED)
pkg_check_modules(glib-2.0 REQUIRED IMPORTED_TARGET glib-2.0)

file(GLOB_RECURSE util_sources *.cpp *.h)

add_library(util STATIC ${util_sources})

# TODO this is dirty: fix & make it work on 'any' system, e.g. PkgConfig
target_include_directories(util
    PUBLIC
    /usr/include/ostree-1
    ${glib-2.0_INCLUDE_DIRS}
    # includes: /usr/include/glib-2.0/
    #           /usr/include/glib-2.0/glib
    #           /usr/include/glib-2.0/gio
    #           /usr/lib64/glib-2.0/include
    #/usr/include/sysprof-6
)

target_link_libraries(util
  PUBLIC PkgConfig::glib-2.0
  # PkgConfig::glib-2.0 not working properly (missing link to gio & gobject)
  PUBLIC /usr/lib64/libgio-2.0.so
  PUBLIC /usr/lib64/libgobject-2.0.so

  PRIVATE ostree-1
  PRIVATE clip
)

add_library(ostui::util ALIAS util)
