#include "OSTreeTUI.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <unordered_map>

#include <fcntl.h>

//#include "clip.h"

#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "scroller.h"

#include "footer.h"
#include "manager.h"

#include "../util/cpplibostree.h"


int OSTreeTUI::main(const std::string& repo) {
	using namespace ftxui;

	std::cout << "OSTree TUI on '" << repo << "'";

// - STATES -
	// Model
	cpplibostree::OSTreeRepo ostree_repo(repo);
	// View
	size_t selected_commit{0}; 								 // view-index
	std::unordered_map<std::string, bool>  visible_branches{}; // map branch visibility to branch
	std::vector<std::string> visible_commit_view_map{};		 // map from view-index to commit-hash
	std::unordered_map<std::string, Color> branch_color_map{}; // map branch to color
	//
	auto screen = ScreenInteractive::Fullscreen();

// - INIT -
	for (const auto& branch : ostree_repo.getBranches()) {
		visible_branches[branch] = true;
		// color
		std::hash<std::string> name_hash{};
		branch_color_map[branch] = Color::Palette256((name_hash(branch) + 10) % 256);
	}

// - UPDATES -
	// TODO probably should be in a separate method, not a lambda
	auto parse_visible_commit_map = [&] {
		// get filtered commits
		visible_commit_view_map = {};
		for (const auto& commit_pair : ostree_repo.getCommitList()) {
			// filter branches
			for (const auto& branch : commit_pair.second.branches) {
				if (visible_branches[branch]) {
					visible_commit_view_map.push_back(commit_pair.first);
				}
			}
		}
		// sort by date
		std::sort(visible_commit_view_map.begin(), visible_commit_view_map.end(), [&](const std::string& a, const std::string& b) {
			return ostree_repo.getCommitList().at(a).timestamp
				 > ostree_repo.getCommitList().at(b).timestamp;
		});
	};
	parse_visible_commit_map();

	auto refresh_repository = [&] {
		ostree_repo.updateData();
		parse_visible_commit_map();
		return true;
	};

	auto next_commit = [&] {
		if (selected_commit + 1 >= visible_commit_view_map.size()) {
			selected_commit = visible_commit_view_map.size() - 1;
			return false;
		}
		++selected_commit;
		return true;
	};
	auto prev_commit = [&] {
		if (selected_commit <= 0) {
			selected_commit = 0;
			return false;
		}
		--selected_commit;
		return true;
	};

// - ELEMENTS ---------- ----------
	Manager manager(ostree_repo, visible_branches);
	Component branch_boxes = manager.branch_boxes;
	Component manager_renderer = Renderer(branch_boxes, [&] {

		Element commit_info;
		if (visible_commit_view_map.size() <= 0) {
			commit_info = text(" no commit info available ") | color(Color::RedLight) | bold | center;
		} else {
			cpplibostree::Commit display_commit = ostree_repo.getCommitList().at(visible_commit_view_map.at(selected_commit));
			commit_info = Manager::render(display_commit);
		}
		
	    return vbox({
				manager.branchBoxRender(),
				separator(),
				commit_info
			});
    });

	Component log_renderer = Scroller(&selected_commit, Renderer([&] {
		parse_visible_commit_map();
		selected_commit = std::min(selected_commit, visible_commit_view_map.size() - 1);
		return CommitRender::commitRender(ostree_repo, visible_commit_view_map, visible_branches, branch_color_map, selected_commit);
	}));

  	Component footer_renderer = footer::footerRender();

// - FINALIZE ---------- ----------
	// window specific shortcuts
	log_renderer = CatchEvent(log_renderer | border, [&](Event event) {
		// switch through commits
    	if (event == Event::ArrowUp || event == Event::Character('k') || (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
    	  	return prev_commit();
    	}
    	if (event == Event::ArrowDown || event == Event::Character('j') || (event.is_mouse() && event.mouse().button == Mouse::WheelDown)) {
    	  	return next_commit();
    	}
		return false;
	});

  	int log_size{45};
  	int footer_size{1};
  	Component container{manager_renderer};
  	container = ResizableSplitLeft(log_renderer, container, &log_size);
  	container = ResizableSplitBottom(footer_renderer, container, &footer_size);
	
	// add shortcuts
	Component main_container = CatchEvent(container | border, [&](const Event& event) {
		// apply changes
    	if (event == Event::Character('s')) {
    	  	std::cout << "apply not implemented yet" << std::endl;
    	  	return true;
    	}
		// enter rebase mode
    	if (event == Event::Character('b')) {
    	  	std::cout << "rebase not implemented yet" << std::endl;
    	  	return true;
    	}
		// copy commit id
    	if (event == Event::Character('c')) {
			// TODO add (working) clipboard library
			std::cout << "copy not implemented yet" << std::endl;
    	  	return true;
    	}
		// refresh repository
		if (event == Event::Character('r')) {
			refresh_repository();
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
