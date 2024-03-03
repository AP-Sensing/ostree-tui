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

auto footerRender() {
	return Renderer([] {
		return hbox({
			text("OSTree TUI") | bold,
			separator(),
			text("  || exit: q || rebase_mode: r || apply changes: s ||  "),
		});
	});
}

int main(void) {
  	auto screen = ScreenInteractive::Fullscreen();
	
	// - STATES -
	std::unordered_map<std::string, bool> branch_visibility_map = {};
	branch_visibility_map["foo"] = true;
	branch_visibility_map["oof"] = true;
	bool foo_bool = true;
	bool oof_bool = true;

	// - LOG ---------- ----------
	auto commits = parseCommitsAllBranches();	
  	commitRender(commits, excluded_branches);

	auto log_renderer = Renderer([&] {
		// update shown branches
		excluded_branches = {};
		std::for_each(branch_visibility_map.begin(), branch_visibility_map.end(),
				[&](std::pair<std::string, bool> key_value) {
					if (! key_value.second) {
						excluded_branches.push_back(key_value.first);
					}
		});
		// render commit log
		return commitRender(commits, excluded_branches);
	});

	// - MANAGER ---------- ----------
		/* TODOs 
		 * - make generic for branches
		 * - implement different modes (log, rebase, ...)
		 * - refactor into own method
		 * - add bottom part of menu
		 */
	auto foo_checkbox = Checkbox("foo", &branch_visibility_map["foo"]);
	auto oof_checkbox = Checkbox("oof", &branch_visibility_map["oof"]);
	auto options = Container::Vertical({foo_checkbox, oof_checkbox});
  	auto manager_renderer = Renderer(options, [&] {
		auto branch_filter_box = vbox({
					text(L"filter branches") | bold,
					filler(),
					foo_checkbox->Render(),
					oof_checkbox->Render(),
				});
		return branch_filter_box;
	});

	// - FOOTER ---------- ----------
  	auto footer_renderer = footerRender();

  	int right_size = 30;
  	int footer_size = 1;
  	auto container = log_renderer;
  	container = ResizableSplitRight(manager_renderer, container, &right_size);
  	container = ResizableSplitBottom(footer_renderer, container, &footer_size);
	
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
