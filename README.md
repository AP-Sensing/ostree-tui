# OSTree-TUI
A terminal user interface for OSTree.

-------------
Welcome to OSTree-TUI. This project provides a more user friendly approach to a OSTree interface. It's purpose is to assist developers when using the command line to interact with OSTree (not to replace the command line interface completely).

OSTree-TUI displays the commit history in a human-friendly way. Like OSTree itself, the design is partially inspired by git.
![ostree-tui preview](https://github.com/AP-Sensing/ostree-tui/assets/88790311/3dc0e86f-ba43-4cc6-aef3-5a0e5d4a9d82)

# Usage
To start the OSTree-TUI, simply type `ostree-tui <repo_path>` (replace `<repo_path>` with the path to the desired repository). Navigating the application is possible with the arrow keys, special actions are described in the bottom-bar.

# Installation / Build instructions

**Normal build:**

To build OSTree-TUI on your system, just execute the following steps:
1. Clone the repository:
```bash
git clone git@github.com:AP-Sensing/ostree-tui.git
# or use https://github.com/AP-Sensing/ostree-tui.git if you haven't set up your ssh-key
cd ostree-tui
```
2. Build with CMake (requires you to have ostree installed on your system, just follow the error messages):
```bash
mkdir build
cd build
cmake ..
cmake --build .
# The binary will be located in `./bin/ostree-tui`.
# To install, use `make install DESTDIR=<target_destination>`
```

**Webassembly build:**

The Webassembly build has not been tested, or confirmed yet.
```bash
mkdir build_emscripten && cd build_emscripten
emcmake cmake ..
make -j
./run_webassembly.py
(visit localhost:8000)
```
