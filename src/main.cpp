#include <iostream>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <memory> // for shared_ptr, allocator, __shared_ptr_access
#include <stdexcept> 
#include <string>
#include <array>
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/dom/table.hpp"

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive

using namespace ftxui;

struct Commit {
    std::string hash;
    std::string parent;
    std::string contentChecksum;
    std::string date;
    std::string subject;
	std::string branch;
};

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

auto commitRender(std::vector<Commit> commits) {
	int selected = 0;
  	auto menu = Container::Vertical({},&selected);

	// TODO sort by date descending

	// TODO determine all branches, divide them into 'branch slots' & determine max size
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

	for (auto commit : commits) {
		// mark branch as now used
		used_branches.at(branch_map[commit.branch]) = true;
		// render branches
		std::string tree_root;
		for (auto branch : used_branches) {
			if (branch) {
				tree_root += " |";
			} else {
				tree_root += "  ";
			}
		}

		std::string tree_top = tree_root;
		if (tree_top.size() > 0) {
			tree_top.at(2 * branch_map[commit.branch] + 1) = 'O';
		}

		tree->Add(MenuEntry(tree_top));
		tree->Add(MenuEntry(tree_root));
		tree->Add(MenuEntry(tree_root)); // TODO check parent commit

		// render commit
		comm->Add(MenuEntry("commit " + commit.hash.substr(commit.hash.size() - 8)));
		comm->Add(MenuEntry("  " + commit.date));
		comm->Add(MenuEntry(""));

    	// TODO beautify render
		auto commitentry = MenuEntry(commit.hash);

		/*|brnchs||-----------commits-----------|       not shown, comment

		//   top  commit: 0934afg1                      // top commit of branch
		//    |     "some commit message cut o..."
		//    |
		//    O   commit: 3b34afg1                      // one branch only
		//    |    "some different commit mess..."
		//    |
		//  O |   commit: 2734aaa5                      // multiple branches shown
		//  | |    "some commit message cut o..."
		//  | |
		//  | O   commit: 09fee6g2	                    // parent on other branch
		//  | |    "a commit message that is c..."        -> set used_branches false
		//  |/
		//  O     commit: 09fee6g2                      // branc
		//  |      "a commit message that is c..."
		//  |

		*/
	
    	menu->Add(commitentry);
  	}
	auto commitrender = Container::Horizontal({
		tree, comm
	});
	return commitrender;
}

// https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
        result += buffer.data();
        result += " ";
    }
    return result;
}

std::vector<Commit> parseCommitsAllBranches(std::string repo = "testrepo") {
	// get all branches
	std::string branches = exec("ostree refs --repo=testrepo");
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

auto ostreeLog() {
  	// get log TODO make generic & for all refs at once
  	//std::string ostreeLogOutput = exec("ostree log --repo=testrepo foo");

  	// parse commits
	auto commits = parseCommitsAllBranches();

	return commitRender(commits);
  	//return Renderer([ostreeLogOutput] { return paragraph(ostreeLogOutput) | center; });
}

int main(void) {
  	auto screen = ScreenInteractive::Fullscreen();
	
	std::string input;

  	auto log = ostreeLog();
  	auto right = Renderer([] { return text("manager") | center; });
  	auto header = Renderer([] { return text("OSTree TUI") | center; });

	// shell	
	std::vector<std::string> input_entries;
  	int input_selected = 0;
  	Component shell_in = Menu(&input_entries, &input_selected);
 
  	auto input_option = InputOption();
  	std::string input_add_content;
  	input_option.on_enter = [&] {
    	input_entries.push_back(input_add_content);
		input_entries.push_back(exec(input_add_content.c_str()));
    	input_add_content = "";
  	};
  	Component shell = Input(&input_add_content, "input files", input_option);
	
  	int right_size = 30;
  	int top_size = 1;
  	int bottom_size = 1;
	
  	auto container = log;
  	container = ResizableSplitRight(right, container, &right_size);
  	container = ResizableSplitTop(header, container, &top_size);
	//container = ResizableSplitBottom(shell_in, container, &bottom_size);
  	//container = ResizableSplitBottom(shell, container, &bottom_size);
	
  	auto renderer =
  	    Renderer(container, [&] { return container->Render() | border; });
	
  	screen.Loop(renderer);

  	return EXIT_SUCCESS;
}
