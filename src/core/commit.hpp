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

#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

#include "../util/cpplibostree.hpp"

namespace CommitRender {

    constexpr size_t COMMIT_DETAIL_LEVEL     {3}; // lines per commit

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
                                const std::unordered_map<std::string, ftxui::Color>& branchColorMap,
                                size_t selectedCommit = 0);

    /**
     * @brief Add a line to a commit-tree-column and commit-info-column.
     *        Both are built and set using addTreeLine() and addTreeLine().
     * 
     * @param treeLineType      type of commit_tree
     * @param lineType          type of commit_info (e.g. hash, date,...)
     * @param treeElements     commit tree column
     * @param commElements     commit info column
     * @param commit            commit to render / get info from
     * @param highlight         should commit be highlighted (as selected)
     * @param usedBranches     branches to render
     * @param branchColorMap  branch colors
     */
    void addLine(const RenderTree& treeLineType, const RenderLine& lineType,
			 ftxui::Elements& treeElements, ftxui::Elements& commElements,
			 const cpplibostree::Commit& commit,
			 const bool& highlight,
			 const std::unordered_map<std::string, bool>& usedBranches,
			 const std::unordered_map<std::string, ftxui::Color>& branchColorMap);

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
				 const std::unordered_map<std::string, bool>& usedBranches,
				 const std::unordered_map<std::string, ftxui::Color>& branchColorMap);
    
    /**
     * @brief build a commit-info line
     * 
     * @param lineType          type of commit-info
     * @param commit            commit to render / get info from
     * @param highlight         should commit be highlighted (as selected)
     * @param branchColorMap  branch colors
     * @return ftxui::Element   commit-info line
     */
    ftxui::Element addCommLine(RenderLine lineType,
				 const cpplibostree::Commit& commit,
				 const bool& highlight,
				 const std::unordered_map<std::string, ftxui::Color>& branchColorMap);
    
} // namespace CommitRender
