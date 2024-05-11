#include "commit.hpp"

#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "../util/cpplibostree.hpp"

namespace CommitRender {

ftxui::Element commitRender(cpplibostree::OSTreeRepo& repo,
                            const std::vector<std::string>& visible_commit_map,
                            const std::unordered_map<std::string, bool>& visible_branches,
                            const std::unordered_map<std::string, ftxui::Color>& branch_color_map,
                            size_t selected_commit) {
	using namespace ftxui;

	// check empty commit list
	if (visible_commit_map.size() <= 0 || visible_branches.size() <= 0) {
		return color(Color::RedLight, text(" no commits to be shown ") | bold | center);
	}

	// divide all visible branches into 'branch slots' & determine max size
	// set all branches to not displayed yet
	std::unordered_map<std::string, size_t> branch_map{};
	std::unordered_map<std::string, bool> used_branches{};
	size_t branch_map_size{0};
	for (const auto& branch_pair : visible_branches) {
		if (branch_pair.second) {
			branch_map[branch_pair.first] = branch_map_size;
			used_branches[branch_pair.first] = false;
			branch_map_size++;
		}
	}

	// - RENDER -
	// left tree, right commits
	Elements tree_elements{};
	Elements comm_elements{};

	std::string marked_string = repo.getCommitList().at(visible_commit_map.at(selected_commit)).hash;

	for (const auto& visible_commit_index : visible_commit_map) {
		
		cpplibostree::Commit commit = repo.getCommitList().at(visible_commit_index);

		// TODO properly use all branches
		std::string relevant_branch = commit.branches.at(0);
		
		// render branches
		Elements tree_branch_elements{};
		Elements tree_top_elements{};
		Elements tree_root_elements{};
		Elements tree_bottom_elements{};
		Elements sign_elements{};

		// check if it is first branch usage
		if (! used_branches.at(relevant_branch)) {
			for (const auto& branch : used_branches) {
				if (branch.second) {
					tree_branch_elements.push_back(text(COMMIT_TREE) | color(branch_color_map.at(branch.first)));
				} else {
					tree_branch_elements.push_back(text(COMMIT_NONE) | color(branch_color_map.at(branch.first)));
				}
			}
			tree_elements.push_back(hbox(std::move(tree_branch_elements)));
			comm_elements.push_back(text(relevant_branch) | color(branch_color_map.at(relevant_branch)));
			// set branch as used
			used_branches.at(relevant_branch) = true;
		}

		// build branches from left to right for this commit
		for (const auto& branch : used_branches) {
			if (branch.second) {
				if (branch.first == relevant_branch) {
					tree_top_elements.push_back(text(COMMIT_NODE) | color(branch_color_map.at(branch.first)));
				} else {
					tree_top_elements.push_back(text(COMMIT_TREE) | color(branch_color_map.at(branch.first)));
				}
				tree_root_elements.push_back(text(COMMIT_TREE) | color(branch_color_map.at(branch.first)));
				tree_bottom_elements.push_back(text(COMMIT_TREE) | color(branch_color_map.at(branch.first)));
				sign_elements.push_back(text(COMMIT_TREE) | color(branch_color_map.at(branch.first)));
			} else {
				tree_top_elements.push_back(text(COMMIT_NONE) | color(branch_color_map.at(branch.first)));
				tree_root_elements.push_back(text(COMMIT_NONE) | color(branch_color_map.at(branch.first)));
				tree_bottom_elements.push_back(text(COMMIT_NONE) | color(branch_color_map.at(branch.first)));
				sign_elements.push_back(text(COMMIT_NONE) | color(branch_color_map.at(branch.first)));
			}
		}

		tree_elements.push_back(hbox(std::move(tree_top_elements)));
		tree_elements.push_back(hbox(std::move(tree_root_elements)));
		tree_elements.push_back(hbox(std::move(tree_bottom_elements)));

		// render commit

		std::string commit_top_text = commit.hash;
		if (commit_top_text.size() > 8) {
			commit_top_text = GAP_TREE_COMMITS + commit.hash.substr(commit.hash.size() - 8);
		}
		Element commit_top_text_element = text(commit_top_text);
		// selected
		if (marked_string == commit.hash) {
			commit_top_text_element = commit_top_text_element | bold | inverted;
		}
		comm_elements.push_back(commit_top_text_element);
		comm_elements.push_back(text(GAP_TREE_COMMITS + commit.date));
		// signed
		if (cpplibostree::OSTreeRepo::isCommitSigned(commit)) {
			tree_elements.push_back(hbox(std::move(sign_elements)));
			std::string signed_text = GAP_TREE_COMMITS + "signed " + (commit.signatures.size() > 1 ? std::to_string(commit.signatures.size()) + "x" : "");
			comm_elements.push_back(text(signed_text) | color(Color::Green));
		}
		comm_elements.push_back(text(""));
  	}
	auto commitrender = hbox({
		vbox(std::move(tree_elements)),
		vbox(std::move(comm_elements))
	});
	return commitrender;
}

}  // namespace CommitRender
