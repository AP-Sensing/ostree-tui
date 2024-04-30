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

using namespace ftxui;

// Text Elements
static const std::string COMMIT_NODE    = " ☐";
static const std::string COMMIT_NODE_GIT= " *";
static const std::string COMMIT_TREE    = " │";
static const std::string COMMIT_NONE    = "  ";
static const std::string INDENT         = " ";
static const std::string GAP            = "  ";

// Style
enum GRAPHSTYLE {
    DEFAULT,
    GIT,
};

/**
 * @brief create a Renderer for the commit section
 * 
 * @param repo ostree repository
 * @param branches branches to include
 * @param selected_commit selected commit
 * @return std::shared_ptr<Node> 
 */
std::shared_ptr<Node> commitRender(cpplibostree::OSTreeRepo repo,
                                std::vector<std::string> visible_commit_map,
                                std::unordered_map<std::string, bool> visible_branches,
                                std::unordered_map<std::string, Color> branch_color_map,
                                size_t selected_commit,
				                GRAPHSTYLE style = GRAPHSTYLE::DEFAULT);

/**
 * @brief create a Renderer for the commit section
 * 
 * @param repo ostree repository
 * @param branches branches to include
 * @param selected_commit selected commit
 * @return std::shared_ptr<Node> 
 */
std::shared_ptr<Node> renderDefault(cpplibostree::OSTreeRepo repo,
                                std::vector<std::string> visible_commit_map,
                                std::unordered_map<std::string, bool> visible_branches,
                                std::unordered_map<std::string, Color> branch_color_map,
                                size_t selected_commit);

/**
 * @brief create a Renderer for the commit section in the style of Git
 * 
 * @param repo ostree repository
 * @param branches branches to include
 * @param selected_commit selected commit
 * @return std::shared_ptr<Node> 
 */
std::shared_ptr<Node> renderGitStyle(cpplibostree::OSTreeRepo repo,
                                std::vector<std::string> visible_commit_map,
                                std::unordered_map<std::string, bool> visible_branches,
                                std::unordered_map<std::string, Color> branch_color_map,
                                size_t selected_commit);
