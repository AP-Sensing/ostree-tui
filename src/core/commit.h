/*_____________________________________________________________
 | Commit Render
 |   Left portion of main window, includes a commit-tree &
 |   general commit-info next to it.
 |   A commit should look something like this:
 |     O |   commit: 2734aaa5
 |     | |    "some commit message cut o..."
 |     | |    [signed]
 |___________________________________________________________*/

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../util/cpplibostree.h"

/**
 * @brief create a Renderer for the commit section
 * 
 * @param repo ostree repository
 * @param branches branches to include
 * @param selected_commit selected commit
 * @return Element 
 */
ftxui::Element commitRender(cpplibostree::OSTreeRepo repo,
                                std::vector<std::string> visible_commit_map,
                                std::unordered_map<std::string, bool> visible_branches,
                                std::unordered_map<std::string, ftxui::Color> branch_color_map,
                                size_t selected_commit = 0);
