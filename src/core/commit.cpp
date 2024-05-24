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

	// set all branches to not displayed yet
	std::unordered_map<std::string, bool> used_branches{};
	for (const auto& branch_pair : visible_branches) {
		if (branch_pair.second) {
			used_branches[branch_pair.first] = false;
		}
	}

	// - RENDER -
	// left tree, right commits
	Elements tree_elements{};
	Elements comm_elements{};

	std::string marked_string = repo.getCommitList().at(visible_commit_map.at(selected_commit)).hash;
	for (const auto& visible_commit_index : visible_commit_map) {
		cpplibostree::Commit commit = repo.getCommitList().at(visible_commit_index);
		bool highlight = marked_string == commit.hash;
		// branch head if it is first branch usage
		std::string relevant_branch = commit.branches.at(0);
		if (! used_branches.at(relevant_branch)) {
			used_branches.at(relevant_branch) = true;
			addLine(RenderTree::TREE_LINE_IGNORE_BRANCH, RenderLine::BRANCH_HEAD,
					tree_elements, comm_elements, commit, highlight, used_branches, branch_color_map);
		}
		// commit
		addLine(RenderTree::TREE_LINE_NODE, RenderLine::COMMIT_HASH,
					tree_elements, comm_elements, commit, highlight, used_branches, branch_color_map);
		addLine(RenderTree::TREE_LINE_TREE, RenderLine::COMMIT_DATE,
					tree_elements, comm_elements, commit, highlight, used_branches, branch_color_map);
		addLine(RenderTree::TREE_LINE_TREE, RenderLine::EMPTY,
					tree_elements, comm_elements, commit, highlight, used_branches, branch_color_map);
  	}

	return hbox({
		vbox(std::move(tree_elements)),
		vbox(std::move(comm_elements))
	});
}


void addLine(const RenderTree& treeLineType, const RenderLine& lineType,
			 ftxui::Elements& tree_elements, ftxui::Elements& comm_elements,
			 const cpplibostree::Commit& commit,
			 const bool& highlight,
			 const std::unordered_map<std::string, bool>& used_branches,
			 const std::unordered_map<std::string, ftxui::Color>& branch_color_map) {
	tree_elements.push_back(addTreeLine(treeLineType, commit, used_branches, branch_color_map));
	comm_elements.push_back(addCommLine(lineType, commit, highlight, branch_color_map));
}


ftxui::Element addTreeLine(const RenderTree& treeLineType,
				 const cpplibostree::Commit& commit,
				 const std::unordered_map<std::string, bool>& used_branches,
				 const std::unordered_map<std::string, ftxui::Color>& branch_color_map) {
	using namespace ftxui;

	std::string relevant_branch = commit.branches.at(0);
	Elements tree;
	
	// build branch by branch from left to right
	for (const auto& branch : used_branches) {
		if (treeLineType == RenderTree::TREE_LINE_IGNORE_BRANCH) {
			if (branch.second && branch.first != relevant_branch) {
				tree.push_back(text(COMMIT_TREE) | color(branch_color_map.at(branch.first)));
			} else {
				tree.push_back(text(COMMIT_NONE) | color(branch_color_map.at(branch.first)));
			}
		} else if (treeLineType == RenderTree::TREE_LINE_NODE) {
			if (branch.second) {
				if (branch.first == relevant_branch) {
					tree.push_back(text(COMMIT_NODE) | color(branch_color_map.at(branch.first)));
				} else {
					tree.push_back(text(COMMIT_TREE) | color(branch_color_map.at(branch.first)));
				}
			} else {
				tree.push_back(text(COMMIT_NONE) | color(branch_color_map.at(branch.first)));
			}
		} else if (treeLineType == RenderTree::TREE_LINE_TREE) {
			if (branch.second) {
				tree.push_back(text(COMMIT_TREE) | color(branch_color_map.at(branch.first)));
			} else {
				tree.push_back(text(COMMIT_NONE) | color(branch_color_map.at(branch.first)));
			}
		} else {
			tree.push_back(text("error") | color(Color::Red));
		}
    }

    return hbox(std::move(tree));
}


ftxui::Element addCommLine(RenderLine lineType,
				 const cpplibostree::Commit& commit,
				 const bool& highlight,
				 const std::unordered_map<std::string, ftxui::Color>& branch_color_map) {
	using namespace ftxui;

	std::string relevant_branch = commit.branches.at(0);
	Elements comm;

	switch (lineType) {
		case EMPTY: {
			comm.push_back(text(""));
			break;
		}
    	case BRANCH_HEAD: {
			comm.push_back(text(relevant_branch) | color(branch_color_map.at(relevant_branch)));
			break;
		}
    	case COMMIT_HASH: {
			// length adapted hash
			std::string commit_top_text = commit.hash;
			if (commit_top_text.size() > 8) {
				commit_top_text = GAP_TREE_COMMITS + commit.hash.substr(commit.hash.size() - 8);
			}
			Element commit_top_text_element = text(commit_top_text);
			// highlighted / selected
			if (highlight) {
				commit_top_text_element = commit_top_text_element | bold | inverted;
			}
			// signed
			if (cpplibostree::OSTreeRepo::isCommitSigned(commit)) {
				std::string signed_text = " signed " + (commit.signatures.size() > 1 ? std::to_string(commit.signatures.size()) + "x" : "");
				commit_top_text_element = hbox(commit_top_text_element, text(signed_text) | color(Color::Green));
			}
			comm.push_back(commit_top_text_element);
			break;
		}
    	case COMMIT_DATE: {
			std::string ts = std::format("{:%Y-%m-%d %T %Ez}",
								std::chrono::time_point_cast<std::chrono::seconds>(commit.timestamp));
			comm.push_back(text(GAP_TREE_COMMITS + ts));
			break;
		}
    	case COMMIT_SUBJ: {
			std::string ts = std::format("{:%Y-%m-%d %T %Ez}",
								std::chrono::time_point_cast<std::chrono::seconds>(commit.timestamp));
			comm.push_back(paragraph(GAP_TREE_COMMITS + commit.subject));
			break;
		}
    }

	return hbox(std::move(comm));
}

}  // namespace CommitRender
