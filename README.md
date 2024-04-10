# OSTree TUI
A terminal user interface for OSTree.

-------------
**Warning**
This project is in its **earliest stages** and is **not ready** to be used, or reviewed in any form. Please refrain from posting issues while this setup is in process and rather contact one of the maintainers instead!

Here are the main ToDos, that would need to be fulfilled to consider it 'ready':
- integrate conan2 into the project (replace some of the FetchContent)
- cleanup CMakeLists
- replace command-line access with libostree
- *heavily* refactor code
- squash some of the early commits
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
