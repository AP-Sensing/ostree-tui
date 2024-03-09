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
#include "manager.cpp"
#include "footer.cpp"

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
	// commits
	auto commits = parseCommitsAllBranches(repo);
	size_t selected_commit{0};

// - MANAGER ---------- ----------
	Manager manager = Manager(Container::Vertical({}), br, branch_visibility_map, commits, selected_commit);
	auto manager_renderer = manager.render();

// - LOG ---------- ----------
  	commitRender(commits, branches);

	auto log_renderer = Renderer([&] {
		// update shown branches
		branch_visibility_map = manager.branch_visibility_map;
		branches = {};
		std::for_each(branch_visibility_map.begin(), branch_visibility_map.end(),
				[&](std::pair<std::string, bool> key_value) {
					if (key_value.second) {
						branches.push_back(key_value.first);
					}
		});
		// render commit log
		return commitRender(commits, branches, selected_commit);
	});

// - FOOTER ---------- ----------
  	auto footer_renderer = footerRender();

// - FINALIZE ---------- ----------
  	int log_size = 30;
  	int footer_size = 1;
  	auto container = log_renderer;
  	container = ResizableSplitRight(manager_renderer, container, &log_size);
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
		// switch through commits (may be temporary)
    	if (event == Event::Character('+')) {
    	  if (selected_commit > 0)
		  	--selected_commit;
		  manager.selected_commit = selected_commit;
    	  return true;
    	}
    	if (event == Event::Character('-')) {
    	  if (selected_commit + 1 < commits.size())
		  	++selected_commit;
		  manager.selected_commit = selected_commit;
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
