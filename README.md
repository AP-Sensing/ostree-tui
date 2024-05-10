# OSTree-TUI
*A terminal user interface for OSTree.*

![ostree-tui preview](https://github.com/AP-Sensing/ostree-tui/assets/88790311/3dc0e86f-ba43-4cc6-aef3-5a0e5d4a9d82)

<p align="center">
  <a href="#"><img src="https://img.shields.io/badge/c++-%2300599C.svg?style=flat&logo=c%2B%2B&logoColor=white"></img></a>
  <a href="https://opensource.org/license/gpl-3-0"><img src="https://img.shields.io/github/license/AP-Sensing/ostree-tui?color=black"></img></a>
  <a href="#"><img src="https://img.shields.io/github/stars/AP-Sensing/ostree-tui"></img></a>
  <a href="#"><img src="https://img.shields.io/github/forks/AP-Sensing/ostree-tui"></img></a>
  <a href="#"><img src="https://img.shields.io/github/repo-size/AP-Sensing/ostree-tui"></img></a>
  <a href="https://github.com/AP-Sensing/ostree-tui/graphs/contributors"><img src="https://img.shields.io/github/contributors/AP-Sensing/ostree-tui?color=blue"></img></a>
  <a href="https://github.com/AP-Sensing/ostree-tui/issues"><img src="https://img.shields.io/github/issues/AP-Sensing/ostree-tui"></img></a>
<br/>
  <a href="https://github.com/AP-Sensing/ostree-tui/issues/new">Report a Bug</a> ·
  <a href="https://github.com/AP-Sensing/ostree-tui/issues/new">Request a Feature</a> ·
  <a href="https://github.com/AP-Sensing/ostree-tui/fork">Fork the Repo</a> ·
  <a href="https://github.com/AP-Sensing/ostree-tui/compare">Submit a Pull Request</a>
</br>
</p>

-------------
Welcome to OSTree-TUI. This project provides a more user friendly approach to a OSTree interface. It's purpose is to assist developers when using the command line to interact with OSTree (not to replace the command line interface completely).

## Usage & Features
To start the OSTree-TUI, simply type `ostree-tui <repo_path>` (replace `<repo_path>` with the path to the desired repository). Navigating the application is possible with the arrow keys, or mouse input. Special actions are described in the bottom-bar.

The features currently include:
 * Display a commit tree on the left
 * Branch filter options on the top right
 * Display details to the selected commit on the right

Upcoming features can be viewed in the [issues](https://github.com/AP-Sensing/ostree-tui/issues)!

## Installation / Build instructions

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
cmake --build . --parallel
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
