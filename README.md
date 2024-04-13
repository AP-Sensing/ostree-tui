# OSTree TUI
A terminal user interface for OSTree.

-------------
**Warning**
This project is in its **earliest stages** and therefor missing many features, as well as some critical bug cleansing. Please refrain from posting issues while this setup is in process and rather contact one of the maintainers instead!
-------------

# Build instructions:
```
mkdir build
cd build
cmake ..
cmake --build .
./bin/ostree-tui
```

**Webassembly build:**
```
mkdir build_emscripten && cd build_emscripten
emcmake cmake ..
make -j
./run_webassembly.py
(visit localhost:8000)
```
