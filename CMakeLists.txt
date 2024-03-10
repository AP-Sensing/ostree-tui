cmake_minimum_required(VERSION 3.27)

# Fetch FTXUI _____________________________________________TODO
# TODO switch out Fetch for conanfile

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v4.0.0
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
  FetchContent_Populate(ftxui)
  add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# Fetch Clip __________________________________________________
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(clip
  GIT_REPOSITORY https://github.com/dacap/clip.git
  GIT_TAG v1.7
)

FetchContent_GetProperties(clip)
if(NOT clip_POPULATED)
  FetchContent_Populate(clip)
  add_subdirectory(${clip_SOURCE_DIR} ${clip_BINARY_DIR})
endif()

# Project _____________________________________________________
set(PROJECT_NAME "ostree-tui")
set(PROJECT_DESCRIPTION "Terminal User Interface for ostree.")
set(PROJECT_HOMEPAGE_URL "https://github.com/AP-Sensing/ostree-tui")

project("${PROJECT_NAME}"
  DESCRIPTION "${PROJECT_DESCRIPTION}"
  HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}"
  LANGUAGES CXX
  VERSION 0.1.0
)

# C++ Standard ________________________________________________
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler Options
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -Werror=format-security -pipe -Wconversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror=format-security -pipe -Wconversion")

# conan dependencies ______________________________________TODO

# Project _____________________________________________________
add_subdirectory(src)

# Emscripten __________________________________________________
if (EMSCRIPTEN) 
  string(APPEND CMAKE_CXX_FLAGS " -s USE_PTHREADS")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s ASYNCIFY")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s PROXY_TO_PTHREAD")

  foreach(file "index.html" "run_webassembly.py")
    configure_file("src/${file}" ${file})
  endforeach(file)
endif() 
