#include "OSTreeTUI.h"

#include <iostream>
#include <cstdio>
#include <sstream>
#include <string>
#include <unordered_map>
#include <algorithm>

#include <cstdio>
#include <fcntl.h>
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

#include "commit.h"
#include "manager.h"
#include "footer.h"

//#include "clip.h"

#include "../util/cl_ostree.h"
#include "../util/commandline.h"
#include "../util/cpplibostree.h"


auto OSTreeTUI::main(const std::string& repo) -> int {
	std::cout << "OSTree TUI on '" << repo << "'";

// - STATES -
	// OSTree Repo
	cl_ostree::OSTreeRepo ostree_repo(repo);
	size_t selected_commit{0};

	// new OSTree Repo - TODO replace
	cpplibostree::OSTreeRepo cpp_ostree_repo("tmp_repo");
	
	// Screen
	auto screen = ScreenInteractive::Fullscreen();

// - MANAGER ---------- ----------
	Manager manager = Manager(&ostree_repo, Container::Vertical({}), selected_commit);
	auto manager_renderer = manager.render();

// - LOG ---------- ----------
  	commitRender(ostree_repo,*ostree_repo.getCommitListSorted(), *ostree_repo.getBranches());

	auto log_renderer = Renderer([&] {
			// update shown branches
			ostree_repo.setBranches({});
			std::for_each(manager.branch_visibility_map.begin(), manager.branch_visibility_map.end(),
					[&](std::pair<std::string, bool> key_value) {
						if (key_value.second) {
							ostree_repo.getBranches()->push_back(key_value.first);
						}
			});
			// render commit log
		return commitRender(ostree_repo, *ostree_repo.getCommitListSorted(), *ostree_repo.getBranches(), selected_commit);
	});

// - FOOTER ---------- ----------
  	auto footer_renderer = footer::footerRender();

// - FINALIZE ---------- ----------
  	int log_size = 80;
  	int footer_size = 1;
  	auto container = manager_renderer;
  	container = ResizableSplitLeft(log_renderer, container, &log_size);
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
		// copy commit id
    	if (event == Event::Character('c')) {
			// TODO replace with (working) clipboard library
			std::string cmd = "gnome-terminal -- bash -c \"echo " + ostree_repo.getCommitListSorted()->at(selected_commit).hash + " | xclip -selection clipboard; sleep .01\"";
			commandline::exec(cmd.c_str());
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
    	  if (selected_commit + 1 < ostree_repo.getCommitListSorted()->size())
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
