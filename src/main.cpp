#include <iostream>
#include <cstdio>
#include <iostream>
#include <memory> // for shared_ptr, allocator, __shared_ptr_access
#include <stdexcept> 
#include <string>
#include <array>
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
 

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include <ftxui/component/screen_interactive.hpp> // for ScreenInteractive

using namespace ftxui;

// https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
        result += " ";
    }
    return result;
}

auto ostreeLog() {
  // get log TODO make generic & for all refs at once
  std::string ostreeLogOutput = exec("ostree log --repo=testrepo foo");

  return Renderer([ostreeLogOutput] { return paragraph(ostreeLogOutput) | center; });
}

int main(void) {
  auto screen = ScreenInteractive::Fullscreen();
 
  auto log = ostreeLog();
  auto right = Renderer([] { return text("manager") | center; });
  auto header = Renderer([] { return text("OSTree TUI") | center; });
  //auto bottom = Renderer([] { return text("bottom") | center; });
 
  int right_size = 30;
  int top_size = 1;
  //int bottom_size = 1;
 
  auto container = log;
  container = ResizableSplitRight(right, container, &right_size);
  container = ResizableSplitTop(header, container, &top_size);
  //container = ResizableSplitBottom(bottom, container, &bottom_size);
 
  auto renderer =
      Renderer(container, [&] { return container->Render() | border; });
 
  screen.Loop(renderer);

  return EXIT_SUCCESS;
}
