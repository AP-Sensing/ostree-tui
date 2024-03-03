#include <iostream>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <memory> // for shared_ptr, allocator, __shared_ptr_access
#include <stdexcept> 
#include <string>
#include <array>
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/dom/table.hpp"

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
// TODO fix veeeery dirty include...
#include "/home/timon/Workdir/ostree-tui/build/_deps/clip-src/clip.h" // for Clipboard

#include "commit.cpp"

using namespace ftxui;

std::vector<std::string> excluded_branches = {};

auto ostreeLog() {
  	// parse commits
	auto commits = parseCommitsAllBranches();

	return commitRender(commits, excluded_branches);
}

auto manager(){ // TODO implement different modes (log, rebase, ...)
	// TODO get branches
	auto container = Container::Vertical({});
	bool foo_shown = false;
	container->Add(Checkbox("foo", &foo_shown));
	if (! foo_shown) {
		excluded_branches = {"foo"};
	} else {
		excluded_branches = {};
	}
	container->Add(Renderer([foo_shown] { return text(foo_shown ? "true" : "false"); }));
	return container;
}

int main(void) {
  	auto screen = ScreenInteractive::Fullscreen();
	
	std::string input;

  	auto log = ostreeLog();
  	auto right = manager();
  	auto footer = Renderer([] { return text("OSTree TUI") | center; });

  	int right_size = 30;
  	int top_size = 1;
	
  	auto container = log;
  	container = ResizableSplitRight(right, container, &right_size);
  	container = ResizableSplitBottom(footer, container, &top_size);
	
	// add shortcuts
	auto main_container = CatchEvent(container | border, [&](Event event) {
		// apply changes
    	if (event == Event::Character('s')) {
    	  std::cout << "apply not implemented yet" << std::endl;
    	  return true;
    	}
		// enter rebase mode
    	if (event == Event::Character('r')) {
    	  std::cout << "rebase not implemented yet" << std::endl;
    	  return true;
    	}
		// exit
    	if (event == Event::Character('q') || event == Event::Escape) {
    	  screen.ExitLoopClosure()();
    	  return true;
    	}
    	return false;
  	});

  	screen.Loop(main_container);

  	return EXIT_SUCCESS;
}
