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

namespace TrashBin {
// window dimensions
constexpr int BIN_WINDOW_HEIGHT{5};
constexpr int BIN_WINDOW_WIDTH{40};

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
[[nodiscard]] ftxui::Component TrashBinComponent(OSTreeTUI& ostreetui);

}  // namespace TrashBin
