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
		//std::cout << "commit " << commit.contentChecksum << "\n";
		for(const auto & branch : branches) {
			//std::cout << "compare (" << commit.branch << ") with (" << branch << ") -> ";
			if(commit.branch == branch) {
        		valid_branch = true;
				//std::cout << "true\n";
    		}
			//std::cout << "false\n";
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
		// mark branch as now used
		used_branches.at(branch_map[commit.branch]) = true;
		// render branchesg
		std::string tree_root;
		for (auto branch : used_branches) {
			if (branch) {
				tree_root += "  |";//"  │";
			} else {
				tree_root += "   ";
			}
		}

		std::string tree_top = tree_root;
		if (tree_top.size() > 0) {
			tree_top.at(3 * branch_map[commit.branch] + 2) = 'O';
		}

		tree_elements.push_back(text(tree_top));
		tree_elements.push_back(text(tree_root));
		tree_elements.push_back(text(tree_root));

		// render commit

		std::string commit_top_text = commit.hash;
		if (commit_top_text.size() > 8)
			commit_top_text = "   " + commit.hash.substr(commit.hash.size() - 8);
		Element commit_top_text_element = text(commit_top_text);
		// selected
		if (marked_string.compare(commit.hash) == 0) {
			commit_top_text_element = commit_top_text_element | bold;
		}
		comm_elements.push_back(commit_top_text_element);
		comm_elements.push_back(text("   " + commit.date));
		// signed
		if (repo.isCommitSigned(commit)) {
			tree_elements.push_back(text(tree_root));
			comm_elements.push_back(text("   signed") | color(Color::Green));
		}
		comm_elements.push_back(text(""));
  	}
	auto commitrender = hbox({
		vbox(std::move(tree_elements)),
		vbox(std::move(comm_elements))
	});
	// TODO this doesn't allow for button usage, fix this
	return commitrender;
}
