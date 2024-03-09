OSTree TUI
-------------

[![Build Status](https://travis-ci.com/ArthurSonzogni/ftxui-starter.svg?branch=master)](https://travis-ci.com/ArthurSonzogni/ftxui-starter)

Minimal starter project using the [FTXUI library](https://github.com/ArthurSonzogni/ftxui)


# Build instructions:
```
mkdir build
cd build
cmake ..
make -j
./ostree-log
```

## Webassembly build:
```
mkdir build_emscripten && cd build_emscripten
emcmake cmake ..
make -j
./run_webassembly.py
(visit localhost:8000)
```

## Structure
main
- startup & screen
other
- other.init() = initialize
- other.render() = get Renderer([&]{...})
