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

auto ostreeLog() {
  	// parse commits
	auto commits = parseCommitsAllBranches();

	return commitRender(commits);
}

auto manager(){ // TODO implement different modes (log, rebase, ...)
	return Renderer([] { return text("manager") | center; });
}

int main(void) {
  	auto screen = ScreenInteractive::Fullscreen();
	
	std::string input;

  	auto log = ostreeLog();
  	auto right = manager();
  	auto footer = Renderer([] { return text("OSTree TUI") | center; });

	/* shell	
	std::vector<std::string> input_entries;
  	int input_selected = 0;
  	Component shell_in = Menu(&input_entries, &input_selected);
 
  	auto input_option = InputOption();
  	std::string input_add_content;
  	input_option.on_enter = [&] {
    	input_entries.push_back(input_add_content);
		input_entries.push_back(exec(input_add_content.c_str()));
    	input_add_content = "";
  	};
  	Component shell = Input(&input_add_content, "input files", input_option);
	*/
	
  	int right_size = 30;
  	int top_size = 1;
  	int bottom_size = 1;
	
  	auto container = log;
  	container = ResizableSplitRight(right, container, &right_size);
  	container = ResizableSplitBottom(footer, container, &top_size);
	//container = ResizableSplitBottom(shell_in, container, &bottom_size);
  	//container = ResizableSplitBottom(shell, container, &bottom_size);
	
  	auto renderer =
  	    Renderer(container, [&] { return container->Render() | border; });
	
  	screen.Loop(renderer);

  	return EXIT_SUCCESS;
}
