#include <iostream>
#include <cstdio>
#include <sstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <vector>

#include <cstdio>
#include <cerrno>
#include <fcntl.h>
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

#include "core/commit.h"
#include "core/manager.h"
#include "core/footer.h"
#include "util/cl_ostree.h"
#include "util/commandline.h"
//#include "util/cpplibostree.h"

using namespace ftxui;

int tui_application(std::string repo = "repo");

int main(int argc, const char** argv) {
// - PARSE arguments -
	// parse repository
	argc--;
	argv++;
	if (argc == 0) {
		std::cout << "no repository provided\n";
		std::cout << "usage: " << argv[-1] << " repository\n";
		return 0;
	}
	std::string repo = argv[0];
	//  TODO parse optional branch

	// open OSTree Repo
	//cpplibostree::OSTreeRepo osr(repo);

	return tui_application(repo);
}

/* main application
 * OSTree TUI
 */
int tui_application(std::string repo) {
	std::cout << "OSTree TUI on '" << repo << "'";

	cl_ostree::OSTreeRepo ostree_repo(repo);

  	auto screen = ScreenInteractive::Fullscreen();

// - STATES -
	std::unordered_map<std::string, bool> branch_visibility_map = {};
	// get all branches
	std::string br = cl_ostree::getAllBranches(repo);
	std::stringstream branches_ss(br);
	std::string branch;
	while (branches_ss >> branch) {
		branch_visibility_map[branch] = true;
	}
	// commits
	ostree_repo.setCommitList(parseCommitsAllBranches(*ostree_repo.getRepo()));
	size_t selected_commit{0};

// - MANAGER ---------- ----------
	Manager manager = Manager(Container::Vertical({}), br, branch_visibility_map, *ostree_repo.getCommitList(), selected_commit);
	auto manager_renderer = manager.render();

// - LOG ---------- ----------
  	commitRender(*ostree_repo.getCommitList(), *ostree_repo.getBranches());

	auto log_renderer = Renderer([&] {
			// update shown branches
			branch_visibility_map = manager.branch_visibility_map;
			ostree_repo.setBranches({});
			std::for_each(branch_visibility_map.begin(), branch_visibility_map.end(),
					[&](std::pair<std::string, bool> key_value) {
						if (key_value.second) {
							ostree_repo.getBranches()->push_back(key_value.first);
						}
			});
			// render commit log
		return commitRender(*ostree_repo.getCommitList(), *ostree_repo.getBranches(), selected_commit);
	});

// - FOOTER ---------- ----------
  	auto footer_renderer = footer::footerRender();

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
    	  if (selected_commit + 1 < ostree_repo.getCommitList()->size())
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
