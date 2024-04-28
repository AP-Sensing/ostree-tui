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
	// Model
	cpplibostree::OSTreeRepo ostree_repo(repo);
	// View
	size_t selected_commit{0}; 								 // view-index
	std::unordered_map<std::string, bool>  visible_branches; // map branch visibility to branch
	std::vector<std::string> visible_commit_view_map;		 // map from view-index to commit-hash
	std::unordered_map<std::string, Color> branch_color_map; // map branch to color
	//
	auto screen = ScreenInteractive::Fullscreen();

// - INIT -
	for (auto branch : ostree_repo.getBranches()) {
		visible_branches[branch] = true;
		// color
		std::hash<std::string> name_hash;
		branch_color_map[branch] = Color::Palette256((name_hash(branch) + 10) % 256);
	}

// - UPDATES -
	// TODO probably should be in a separate method, not a lambda
	auto parse_visible_commit_map = [&] {
		// get filtered commits
		visible_commit_view_map = {};
		for (auto& commit_pair : ostree_repo.getCommitList()) {
			// filter branches
			for (auto branch : commit_pair.second.branches) {
				if (visible_branches[branch]) {
					visible_commit_view_map.push_back(commit_pair.first);
				}
			}
		}
		// sort by date
		std::sort(visible_commit_view_map.begin(), visible_commit_view_map.end(), [&](std::string a, std::string b) {
			return ostree_repo.getCommitList().at(a).timestamp
				 > ostree_repo.getCommitList().at(b).timestamp;
		});
	};
	parse_visible_commit_map();

	auto update_data = [&] {
		parse_visible_commit_map();
		return true;
	};

	auto next_commit = [&] {
		if (selected_commit + 1 == visible_commit_view_map.size()) {
			return false;
		}
		++selected_commit;
		return true;
	};
	auto prev_commit = [&] {
		if (selected_commit == 0) {
			return false;
		}
		--selected_commit;
		return true;
	};

// - ELEMENTS ---------- ----------
	//Manager manager = Manager(&ostree_repo, Container::Vertical({}), selected_commit);
	//auto manager_renderer = manager.render();

  	commitRender(ostree_repo, visible_commit_view_map, visible_branches, branch_color_map);

	auto log_renderer = Scroller(&selected_commit, Renderer([&] {
		parse_visible_commit_map();
		return commitRender(ostree_repo, visible_commit_view_map, visible_branches, branch_color_map, selected_commit);
	}));

  	auto footer_renderer = footer::footerRender();

// - FINALIZE ---------- ----------
	// window specific shortcuts
	log_renderer = CatchEvent(log_renderer | border, [&](Event event) {
		// switch through commits
    	if (event == Event::ArrowUp || (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
    	  	return prev_commit();
    	}
    	if (event == Event::ArrowDown || (event.is_mouse() && event.mouse().button == Mouse::WheelDown)) {
    	  	return next_commit();
    	}
		return false;
	});

  	int log_size = 45;
  	int footer_size = 1;
  	//auto container = manager_renderer;
  	//container = ResizableSplitLeft(log_renderer, container, &log_size);
	auto container = log_renderer;
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
			//std::string cmd = "gnome-terminal -- bash -c \"echo " + ostree_repo.getCommitList().at(selected_commit).hash + " | xclip -selection clipboard; sleep .01\"";
			//commandline::exec(cmd.c_str());
    	  	return true;
    	}
		// switch through commits
		// TODO rethink if this is necessary (additionally to the log_renderer shortcuts)
    	if (event == Event::Character('+')) {
    	  	return prev_commit();
    	}
    	if (event == Event::Character('-')) {
    	  	return next_commit();
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
