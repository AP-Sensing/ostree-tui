#include "commit.hpp"

#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "../util/cpplibostree.hpp"

namespace CommitRender {

void addLine(const RenderTree& treeLineType, const RenderLine& lineType,
			 ftxui::Elements& treeElements, ftxui::Elements& commElements,
			 const cpplibostree::Commit& commit,
			 const bool& highlight,
			 const std::unordered_map<std::string, bool>& usedBranches,
			 const std::unordered_map<std::string, ftxui::Color>& branchColorMap) {
	treeElements.push_back(addTreeLine(treeLineType, commit, usedBranches, branchColorMap));
	commElements.push_back(addCommLine(lineType, commit, highlight, branchColorMap));
}

ftxui::Element commitRender(cpplibostree::OSTreeRepo& repo,
                            const std::vector<std::string>& visibleCommitMap,
                            const std::unordered_map<std::string, bool>& visibleBranches,
                            const std::unordered_map<std::string, ftxui::Color>& branchColorMap,
                            size_t selectedCommit) {
	using namespace ftxui;

	// check empty commit list
	if (visibleCommitMap.size() <= 0 || visibleBranches.size() <= 0) {
		return color(Color::RedLight, text(" no commits to be shown ") | bold | center);
	}

	// set all branches to not displayed yet
	std::unordered_map<std::string, bool> usedBranches{};
	for (const auto& branchPair : visibleBranches) {
		if (branchPair.second) {
			usedBranches[branchPair.first] = false;
		}
	}

	// - RENDER -
	// left tree, right commits
	Elements treeElements{};
	Elements commElements{};

	std::string markedString = repo.getCommitList().at(visibleCommitMap.at(selectedCommit)).hash;
	for (const auto& visibleCommitIndex : visibleCommitMap) {
		cpplibostree::Commit commit = repo.getCommitList().at(visibleCommitIndex);
		bool highlight = markedString == commit.hash;
		// branch head if it is first branch usage
		std::string relevantBranch = commit.branches.at(0);
		if (! usedBranches.at(relevantBranch)) {
			usedBranches.at(relevantBranch) = true;
			addLine(RenderTree::TREE_LINE_IGNORE_BRANCH, RenderLine::BRANCH_HEAD,
					treeElements, commElements, commit, highlight, usedBranches, branchColorMap);
		}
		// commit
		addLine(RenderTree::TREE_LINE_NODE, RenderLine::COMMIT_HASH,
					treeElements, commElements, commit, highlight, usedBranches, branchColorMap);
		addLine(RenderTree::TREE_LINE_TREE, RenderLine::COMMIT_DATE,
					treeElements, commElements, commit, highlight, usedBranches, branchColorMap);
		addLine(RenderTree::TREE_LINE_TREE, RenderLine::EMPTY,
					treeElements, commElements, commit, highlight, usedBranches, branchColorMap);
  	}

	return hbox({
		vbox(std::move(treeElements)),
		vbox(std::move(commElements))
	});
}

ftxui::Element addTreeLine(const RenderTree& treeLineType,
				 const cpplibostree::Commit& commit,
				 const std::unordered_map<std::string, bool>& usedBranches,
				 const std::unordered_map<std::string, ftxui::Color>& branchColorMap) {
	using namespace ftxui;

	std::string relevantBranch = commit.branches.at(0);
	Elements tree;
	
	// build branch by branch from left to right
	for (const auto& branch : usedBranches) {
		if (treeLineType == RenderTree::TREE_LINE_IGNORE_BRANCH) {
			if (branch.second && branch.first != relevantBranch) {
				tree.push_back(text(COMMIT_TREE) | color(branchColorMap.at(branch.first)));
			} else {
				tree.push_back(text(COMMIT_NONE) | color(branchColorMap.at(branch.first)));
			}
		} else if (treeLineType == RenderTree::TREE_LINE_NODE) {
			if (branch.second) {
				if (branch.first == relevantBranch) {
					tree.push_back(text(COMMIT_NODE) | color(branchColorMap.at(branch.first)));
				} else {
					tree.push_back(text(COMMIT_TREE) | color(branchColorMap.at(branch.first)));
				}
			} else {
				tree.push_back(text(COMMIT_NONE) | color(branchColorMap.at(branch.first)));
			}
		} else if (treeLineType == RenderTree::TREE_LINE_TREE) {
			if (branch.second) {
				tree.push_back(text(COMMIT_TREE) | color(branchColorMap.at(branch.first)));
			} else {
				tree.push_back(text(COMMIT_NONE) | color(branchColorMap.at(branch.first)));
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
				 const std::unordered_map<std::string, ftxui::Color>& branchColorMap) {
	using namespace ftxui;

	std::string relevantBranch = commit.branches.at(0);
	Elements comm;

	switch (lineType) {
		case EMPTY: {
			comm.push_back(text(""));
			break;
		}
    	case BRANCH_HEAD: {
			comm.push_back(text(relevantBranch) | color(branchColorMap.at(relevantBranch)));
			break;
		}
    	case COMMIT_HASH: {
			// length adapted hash
			std::string commitTopText = commit.hash;
			if (commitTopText.size() > 8) {
				commitTopText = GAP_TREE_COMMITS + commit.hash.substr(0, 8);
			}
			Element commitTopTextElement = text(commitTopText);
			// highlighted / selected
			if (highlight) {
				commitTopTextElement = commitTopTextElement | bold | inverted;
			}
			// signed
			if (cpplibostree::OSTreeRepo::isCommitSigned(commit)) {
				std::string signedText = " signed " + (commit.signatures.size() > 1 ? std::to_string(commit.signatures.size()) + "x" : "");
				commitTopTextElement = hbox(commitTopTextElement, text(signedText) | color(Color::Green));
			}
			comm.push_back(commitTopTextElement);
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
