#include <algorithm>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/component_options.hpp"   // for ButtonOption
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/dom/table.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

#include "commit.h"
#include "../util/commandline.h"
#include "../util/cpplibostree.h"

using namespace ftxui;


auto commitRender(cpplibostree::OSTreeRepo repo, std::vector<Commit> commits, std::vector<std::string> branches, size_t selected_commit) -> std::shared_ptr<Node> {

// prepare data
	// filter commits for excluded branches
	std::vector<Commit> filteredCommits = {};
	for(const auto & commit : commits) {
    	bool valid_branch = false;
		for(const auto & branch : branches) {
			if(commit.branch == branch) {
        		valid_branch = true;
    		}
		}
		if (valid_branch) {
			filteredCommits.push_back(commit);
		}
	}
	commits = filteredCommits;

	// check empty commit list
	if (commits.size() == 0) {
		return color(Color::RedLight, text(" no commits to be shown ") | bold | center);
	}

	// determine all branches, divide them into 'branch slots' & determine max size
	std::unordered_map<std::string,size_t> branch_map;
	size_t branch_map_size{0};
	for (auto commit : commits) {
		if (! branch_map.count(commit.branch)) {
			branch_map[commit.branch] = branch_map_size;
			branch_map_size++;
		}
	}

	std::vector<bool> used_branches(branch_map_size);
	for (size_t i=0; i<branch_map.size(); i++) {
		used_branches[i] = false;
	}

// render
	// left tree, right commits
	Elements tree_elements;
	Elements comm_elements;

	std::string marked_string = commits.at(selected_commit).hash;

	for (auto commit : commits) {
		// render branches
		Elements tree_branch_elements;
		Elements tree_top_elements;
		Elements tree_root_elements;
		Elements tree_bottom_elements;
		
		int color_chose_index{2};
		int branch_index{0};

		// check if it is first branch usage
		if (! used_branches.at(branch_map[commit.branch])) {
			int color_chose_index{2};
			for (auto branch : used_branches) {
				auto branch_color = Color::Palette256(color_chose_index++);
				if (branch) {
					tree_branch_elements.push_back(text("  │") | color(branch_color));
				} else {
					tree_branch_elements.push_back(text("   ") | color(branch_color));
				}
			}
			tree_elements.push_back(hbox(std::move(tree_branch_elements)));
			comm_elements.push_back(text(branches.at(branch_map[commit.branch])) | color(Color::Palette256(branch_map[commit.branch] + 2)));
		}
		// set branch as used
		used_branches.at(branch_map[commit.branch]) = true;

		for (auto branch : used_branches) {
			auto branch_color = Color::Palette256(color_chose_index);
			color_chose_index = (color_chose_index + 1) % 256;
			if (branch) {
				if (branch_index++ == branch_map[commit.branch]) {
					tree_top_elements.push_back(text("  ☐") | color(branch_color));
				} else {
					tree_top_elements.push_back(text("  │") | color(branch_color));
				}
				tree_root_elements.push_back(text("  │") | color(branch_color));
				tree_bottom_elements.push_back(text("  │") | color(branch_color));
			} else {
				tree_top_elements.push_back(text("   ") | color(branch_color));
				tree_root_elements.push_back(text("   ") | color(branch_color));
				tree_bottom_elements.push_back(text("   ") | color(branch_color));
			}
		}

		tree_elements.push_back(hbox(std::move(tree_top_elements)));
		tree_elements.push_back(hbox(std::move(tree_root_elements)));
		tree_elements.push_back(hbox(std::move(tree_bottom_elements)));

		// render commit

		std::string commit_top_text = commit.hash;
		if (commit_top_text.size() > 8)
			commit_top_text = "   " + commit.hash.substr(commit.hash.size() - 8);
		Element commit_top_text_element = text(commit_top_text);
		// selected
		if (marked_string.compare(commit.hash) == 0) {
			commit_top_text_element = commit_top_text_element | bold | inverted;
		}
		comm_elements.push_back(commit_top_text_element);
		comm_elements.push_back(text("   " + commit.date));
		// signed
		if (repo.isCommitSigned(commit)) {
			tree_elements.push_back(hbox(std::move(tree_bottom_elements)));
			comm_elements.push_back(text("   signed") | color(Color::Green));
		}
		comm_elements.push_back(text(""));
  	}
	auto commitrender = hbox({
		vbox(std::move(tree_elements)),
		vbox(std::move(comm_elements))
	});
	return commitrender;
}
