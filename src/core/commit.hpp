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

    static const std::string COMMIT_NODE        = " ☐";
    static const std::string COMMIT_TREE        = " │";
    static const std::string COMMIT_NONE        = "  ";
    static const std::string INDENT             = " ";
    static const std::string GAP_TREE_COMMITS   = "  ";

    /**
     * @brief create a Renderer for the commit section
     * 
     * @param repo OSTree repository
     * @param visible_commit_map List of visible commit hashes
     * @param visible_branches List of visible branches
     * @param branch_color_map Map from branch to its display color
     * @param selected_commit Commit that should be marked as selected
     * @return ftxui::Element
     */
    ftxui::Element commitRender(cpplibostree::OSTreeRepo& repo,
                                const std::vector<std::string>& visible_commit_map,
                                const std::unordered_map<std::string, bool>& visible_branches,
                                const std::unordered_map<std::string, ftxui::Color>& branch_color_map,
                                size_t selected_commit = 0);
} // namespace CommitRender
