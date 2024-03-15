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

#include "../util/cl_ostree.h"

using namespace ftxui;

/**
 * @brief parse commits from a ostree log output
 * 
 * @param ostreeLogOutput ostree log
 * @param branch branch to parse
 * @return * std::vector<Commit> 
 */
std::vector<Commit> parseCommits(std::string ostreeLogOutput, std::string branch);

/**
 * @brief create a Renderer for the commit section
 * 
 * @param repo ostree repository
 * @param branches branches to include
 * @param selected_commit selected commit
 * @return std::shared_ptr<Node> 
 */
std::shared_ptr<Node> commitRender(cl_ostree::OSTreeRepo repo, std::vector<Commit> commits, std::vector<std::string> branches = {}, size_t selected_commit = 0);

/**
 * @brief same as parseCommits(), but on all available branches
 * 
 * @param repo ostree Repository
 * @return std::vector<Commit> 
 */
std::vector<Commit> parseCommitsAllBranches(cl_ostree::OSTreeRepo repo);
