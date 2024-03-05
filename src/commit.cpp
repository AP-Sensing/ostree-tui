#include <algorithm>
#include <iostream>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <memory> // for shared_ptr, allocator, __shared_ptr_access
#include <stdexcept> 
#include <string>
#include <array>
#include <vector>
#include <unordered_map>
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/component_options.hpp"   // for ButtonOption
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/dom/table.hpp"

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive

#include "commit.h"
#include "commandline.cpp"

using namespace ftxui;


std::vector<Commit> parseCommits(std::string ostreeLogOutput, std::string branch){
  	std::vector<Commit> commitList;
  
  	std::stringstream log(ostreeLogOutput);
  	std::string word;

	Commit cur = {"error", "couldn't read commit", "", "", "", branch};

  	size_t mode = 0;
	bool ready = false;
  	while (log >> word) {
    	if (word == "commit") {
			if (ready) {
				commitList.push_back(std::move(cur));
			}
			ready = true;
			// create new commit
			cur = {"", "", "", "", "", branch};
			log >> word;
			cur.hash = word;
		} else if (word == "Parent:") {
			log >> word;
			cur.parent = word;
		} else if (word == "ContentChecksum:") {
			log >> word;
			cur.contentChecksum = word;
		} else if (word == "Date:") {
			log >> word;
			cur.date = word;
		}
  	}
	commitList.push_back(std::move(cur));

	return commitList;
}

/*|brnchs||-----------commits-----------|       not shown, comment

     top  commit: 0934afg1                      // TODO top commit of branch
      |     "some commit message cut o..."
      |
      O   commit: 3b34afg1                      // one branch only
      |    "some different commit mess..."
      |
    O |   commit: 2734aaa5                      // multiple branches shown
    | |    "some commit message cut o..."
    | |
    | O   commit: 09fee6g2	                    // TODO parent on other branch
    | |    "a commit message that is c..."        -> set used_branches false
    |/                                            -> maybe not even possible?
    O     commit: 09fee6g2
    |      "a commit message that is c..."
    |
*/
auto commitRender(std::vector<Commit> commits, std::vector<std::string> branches = {}, size_t* selected_commit = 0) {

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
	for (int i=0; i<branch_map.size(); i++) {
		used_branches[i] = false;
	}

	// left tree, right commits
	auto tree = Container::Vertical({});
	auto comm = Container::Vertical({});

    // sort commits by parents (TODO validate)
    std::sort(commits.begin(), commits.end(), [](const Commit& lhs, const Commit& rhs) {
      return lhs.hash.compare(rhs.hash) > 0;
    });
    std::stable_sort(commits.begin(), commits.end(), [](const Commit& lhs, const Commit& rhs) {
      return lhs.parent == rhs.hash;
    });

	for (auto commit : commits) {
		// mark branch as now used
		used_branches.at(branch_map[commit.branch]) = true;
		// render branches
		std::string tree_root;
		for (auto branch : used_branches) {
			if (branch) {
				tree_root += "  |";
			} else {
				tree_root += "   ";
			}
		}

		std::string tree_top = tree_root;
		if (tree_top.size() > 0) {
			tree_top.at(3 * branch_map[commit.branch] + 2) = 'O';
		}

        tree->Add(Renderer([tree_top] { return text(tree_top); }));
        tree->Add(Renderer([tree_root] { return text(tree_root); }));
		tree->Add(Renderer([tree_root] { return text(tree_root); })); // TODO check parent commit

		// render commit
        comm->Add(Button(
            " commit " + commit.hash.substr(commit.hash.size() - 8) + " ",
            [&] { std::cout << "test"; }, ButtonOption::Ascii()));
        comm->Add(Renderer([commit] { return text("   " + commit.date); }));
        comm->Add(Renderer([] { return text(""); }));
  	}
	auto commitrender = hbox({
		tree->Render(),
		comm->Render()
	});
	// TODO this doesn't allow for button usage, fix this
	return commitrender;
}

std::vector<Commit> parseCommitsAllBranches(std::string repo) {
	// get all branches
	auto command = "ostree refs --repo=" + repo;
	std::string branches = exec(command.c_str());
	std::stringstream branches_ss(branches);
	std::string branch;

	std::vector<Commit> commitList;

	while (branches_ss >> branch) {
		// get log TODO make generic repo path
		auto command = "ostree log --repo=" + repo + " " + branch;
  		std::string ostreeLogOutput = exec(command.c_str());
		// parse commits
		auto commits = parseCommits(ostreeLogOutput, branch);
		commitList.insert(commitList.end(), commits.begin(), commits.end());
	}
  	
	return commitList;
}
