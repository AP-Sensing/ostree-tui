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

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include "ftxui/component/component_base.hpp"

#include "../util/cpplibostree.hpp"

class OSTreeTUI;

namespace CommitRender {
// UI characters
constexpr std::string COMMIT_NODE{" ☐"};
constexpr std::string COMMIT_TREE{" │"};
constexpr std::string COMMIT_NONE{"  "};
// window dimensions
constexpr int COMMIT_WINDOW_HEIGHT{4};
constexpr int COMMIT_WINDOW_WIDTH{32};
constexpr int PROMOTION_WINDOW_HEIGHT{COMMIT_WINDOW_HEIGHT + 11};
constexpr int PROMOTION_WINDOW_WIDTH{COMMIT_WINDOW_WIDTH + 8};
constexpr int DELETION_WINDOW_HEIGHT{COMMIT_WINDOW_HEIGHT + 8};
constexpr int DELETION_WINDOW_WIDTH{COMMIT_WINDOW_WIDTH + 8};
// render tree types
enum RenderTree : uint8_t {
    TREE_LINE_NODE,          // ☐ | |
    TREE_LINE_TREE,          // | | |
    TREE_LINE_IGNORE_BRANCH  //   | |
};

/**
 * @brief Creates a window, containing a hash and some of its details.
 *        The window has a pre-defined position and snaps back to it,
 *        after being dragged & let go. When hovering over a branch,
 *        defined in `columnToBranchMap`, the window expands to a commit
 *        promotion window.
 *        To be used with other windows, use a `ftxui::Component::Stacked`.
 *
 * @return UI Component
 */
[[nodiscard]] ftxui::Component CommitComponent(size_t position,
                                               const std::string& commit,
                                               OSTreeTUI& ostreetui);

/**
 * @brief Creates a Renderer for the commit section.
 *
 * @param ostreetui OSTreeTUI containing OSTreeRepo and UI info.
 * @param branchColorMap Map from branch to its display color.
 * @param selectedCommit Commit that should be marked as selected.
 * @return UI Element
 */
[[nodiscard]] ftxui::Element commitRender(
    OSTreeTUI& ostreetui,
    const std::unordered_map<std::string, ftxui::Color>& branchColorMap);

/**
 * @brief Builds a commit-tree line.
 *
 * @param treeLineType      Type of commit-tree.
 * @param commit            Commit to get the information from (to render).
 * @param usedBranches      Branches, that should be rendered (visible).
 * @param branchColorMap    Branch colors to use.
 * @return UI Element, one commit-tree line.
 */
[[nodiscard]] ftxui::Element addTreeLine(
    const RenderTree& treeLineType,
    const cpplibostree::Commit& commit,
    const std::unordered_map<std::string, int>& usedBranches,
    const std::unordered_map<std::string, ftxui::Color>& branchColorMap);

}  // namespace CommitRender
