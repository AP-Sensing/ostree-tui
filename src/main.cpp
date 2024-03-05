#include <iostream>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <memory> // for shared_ptr, allocator, __shared_ptr_access
#include <stdexcept> 
#include <string>
#include <array>
#include <string>
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
//#include "ftxui/dom/table.hpp"

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
// TODO fix veeeery dirty include...
#include "/home/timon/Workdir/ostree-tui/build/_deps/clip-src/clip.h" // for Clipboard

#include "commit.cpp"

using namespace ftxui;

std::vector<std::string> branches = {};
std::string repo{"repo"};

auto footerRender() {
	return Renderer([] {
		return hbox({
			text(" OSTree TUI ") | bold,
			separator(),
			text("  || exit: q || rebase_mode: r || apply changes: s ||  "),
		});
	});
}

int main(int argc, const char** argv) {

	// - PARSE arguments -
	// parse repository
	argc--;
	argv++;
	if (argc == 0) {
		std::cout << "no repository provided" << std::endl;
		std::cout << "usage: " << argv[-1] << " repository" << std::endl;
		return 0;
	}
	repo = argv[0];
	//  TODO parse optional branch

  	auto screen = ScreenInteractive::Fullscreen();

	// - STATES -
	std::unordered_map<std::string, bool> branch_visibility_map = {};
	// get all branches
	auto command = "ostree refs --repo=" + repo;
	std::string br = exec(command.c_str());
	std::stringstream branches_ss(br);
	std::string branch;
	while (branches_ss >> branch) {
		branch_visibility_map[branch] = true;
	}

	// - LOG ---------- ----------
	auto commits = parseCommitsAllBranches(repo);	
  	commitRender(commits, branches);

	auto log_renderer = Renderer([&] {
		// update shown branches
		branches = {};
		std::for_each(branch_visibility_map.begin(), branch_visibility_map.end(),
				[&](std::pair<std::string, bool> key_value) {
					if (key_value.second) {
						branches.push_back(key_value.first);
					}
		});
		// render commit log
		return commitRender(commits, branches);
	});

	// - MANAGER ---------- ----------
		/* TODOs 
		 * - make generic for branches
		 * - implement different modes (log, rebase, ...)
		 * - refactor into own method
		 * - add bottom part of menu
		 */
	// create branch checkboxes
	auto branch_boxes = Container::Vertical({});
	std::stringstream br_ss(br);
	while (br_ss >> branch) { // TODO don't reuse variables (cleaner code)
		branch_boxes->Add(Checkbox(branch, &branch_visibility_map[branch]));
	}
	// create manager
  	auto manager_renderer = Renderer(branch_boxes, [&] {
		// branch filter
		std::vector<Element> bfb_elements = {
					text(L"filter branches") | bold,
					filler(),
					branch_boxes->Render() | vscroll_indicator | frame |
									size(HEIGHT, LESS_THAN, 10) | border,
				};
		auto branch_filter_box = vbox(bfb_elements);
		// TODO selected commit info
		auto commit_info_box = vbox({
					text("commit info") | bold,
					filler(),
					text("to be implemented..."),
				});
		// unify boxes
		return vbox({
					branch_filter_box,
					separator(),
					commit_info_box,
				});
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
