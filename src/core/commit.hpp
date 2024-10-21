/*_____________________________________________________________
 | Commit Render
 |   Left portion of main window, includes a commit-tree &
 |   general commit-info next to it.
 |   A commit should look something like this:
 |     ☐ │  2734aaa5
 |     │ │  <commit-date>
 |     │ │  <signed>
 |     │ │
 |___________________________________________________________*/

#pragma once

#include <string>
#include <vector>

#include <ftxui/dom/elements.hpp>
#include <ftxui/component/component.hpp>
#include "ftxui/component/component_base.hpp"

#include "../util/cpplibostree.hpp"

namespace CommitRender {

    constexpr size_t COMMIT_DETAIL_LEVEL     {4}; // lines per commit

    constexpr std::string COMMIT_NODE        {" ☐"};
    constexpr std::string COMMIT_TREE        {" │"};
    constexpr std::string COMMIT_NONE        {"  "};
    constexpr std::string INDENT             {" "};
    constexpr std::string GAP_TREE_COMMITS   {"  "};

    enum RenderTree {
    	TREE_LINE_NODE,         // ☐ | |
    	TREE_LINE_TREE,         // | | |
    	TREE_LINE_IGNORE_BRANCH //   | |
    };

    enum RenderLine {
    	EMPTY,                  // 
    	BRANCH_HEAD,            // foo/bar
    	COMMIT_HASH,            //   a0f33cd9
    	COMMIT_DATE,            //   2024-04-27 09:12:55 +00:00
    	COMMIT_SUBJ             //   Some Subject
    };

    /**
     * @brief Creates a window, containing a hash and some of its details.
     *        The window has a pre-defined position and snaps back to it,
     *        after being dragged & let go. When hovering over a branch,
     *        defined in `columnToBranchMap`, the window expands to a commit
     *        promotion window.
     *        To be used with other windows, use a ftxui::Component::Stacked.
     *
     * @return Component
     */
    ftxui::Component CommitComponent(int position,
                            int& scrollOffset,
                            bool& inPromotionSelection,
                            std::string& promotionHash,
                            std::string& promotionBranch,
                            std::unordered_map<std::string, bool>& visibleBranches,
                            std::vector<std::string>& columnToBranchMap,
                            std::string commit,
                            cpplibostree::OSTreeRepo& ostreerepo,
                            bool& refresh);

    /**
     * @brief create a Renderer for the commit section
     * 
     * @param repo OSTree repository
     * @param visibleCommitMap List of visible commit hashes
     * @param visibleBranches List of visible branches
     * @param branchColorMap Map from branch to its display color
     * @param selectedCommit Commit that should be marked as selected
     * @return ftxui::Element
     */
    ftxui::Element commitRender(cpplibostree::OSTreeRepo& repo,
                                const std::vector<std::string>& visibleCommitMap,
                                const std::unordered_map<std::string, bool>& visibleBranches,
                                std::vector<std::string>& columnToBranchMap,
                                const std::unordered_map<std::string, ftxui::Color>& branchColorMap,
                                int scrollOffset,
                                size_t selectedCommit = 0);

    /**
     * @brief build a commit-tree line
     * 
     * @param treeLineType      type of commit-tree
     * @param commit            commit to render / get info from
     * @param usedBranches     branches to render
     * @param branchColorMap  branch colors
     * @return ftxui::Element   commit-tree line
     */
    ftxui::Element addTreeLine(const RenderTree& treeLineType,
				 const cpplibostree::Commit& commit,
				 const std::unordered_map<std::string, int>& usedBranches,
				 const std::unordered_map<std::string, ftxui::Color>& branchColorMap);

} // namespace CommitRender
