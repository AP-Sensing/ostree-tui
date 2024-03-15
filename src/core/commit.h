#include <memory>
#include <vector>
#include <string>

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/dom/table.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

#include "../util/cl_ostree.h"

#pragma once

using namespace ftxui;

std::vector<Commit> parseCommits(std::string ostreeLogOutput, std::string branch);

std::shared_ptr<Node> commitRender(cl_ostree::OSTreeRepo repo, std::vector<Commit> commits, std::vector<std::string> branches = {}, size_t selected_commit = 0);

std::vector<Commit> parseCommitsAllBranches(cl_ostree::OSTreeRepo repo);
