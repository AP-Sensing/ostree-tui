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

#include "scroller.h"

#include "commit.h"
#include "manager.h"
#include "footer.h"

//#include "clip.h"

#include "../util/commandline.h"
#include "../util/cpplibostree.h"


auto OSTreeTUI::main(const std::string& repo) -> int {
	std::cout << "OSTree TUI on '" << repo << "'";

// - STATES -
	// OSTree Repo data
	size_t selected_commit{0};
	cpplibostree::OSTreeRepo ostree_repo(repo);

	// Color support
	
	
	// Screen
	auto screen = ScreenInteractive::Fullscreen();

// - ELEMENTS ---------- ----------
	Manager manager = Manager(&ostree_repo, Container::Vertical({}), selected_commit);
	auto manager_renderer = manager.render();

  	commitRender(ostree_repo,*ostree_repo.getCommitList(), *ostree_repo.getBranches());

	auto log_renderer = Scroller(&selected_commit, Renderer([&] {
			// update shown branches
			ostree_repo.setBranches({});
			std::for_each(manager.branch_visibility_map.begin(), manager.branch_visibility_map.end(),
					[&](std::pair<std::string, bool> key_value) {
						if (key_value.second) {
							ostree_repo.getBranches()->push_back(key_value.first);
						}
			});
			// render commit log
		return commitRender(ostree_repo, *ostree_repo.getCommitList(), *ostree_repo.getBranches(), selected_commit);
	}));

  	auto footer_renderer = footer::footerRender();

// - FINALIZE ---------- ----------
	// window specific shortcuts
	log_renderer = CatchEvent(log_renderer | border, [&](Event event) {
		// switch through commits
    	if (event == Event::ArrowUp || (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
    	  	if (selected_commit > 0)
		  		--selected_commit;
		  	manager.selected_commit = selected_commit;
    	  	return true;
    	}
    	if (event == Event::ArrowDown || (event.is_mouse() && event.mouse().button == Mouse::WheelDown)) {
    	  	if (selected_commit + 1 < ostree_repo.getCommitListSorted()->size())
		  		++selected_commit;
		  	manager.selected_commit = selected_commit;
    	  	return true;
    	}
		return false;
	});

  	int log_size = 45;
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
		// switch through commits
		// TODO rethink if this is necessary (additionally to the log_renderer shortcuts)
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
