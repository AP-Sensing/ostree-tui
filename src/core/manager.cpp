/*
 * Manager Window
 */

#include <iostream>
#include <cstdio>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

#include "commit.h"
#include "manager.h"

using namespace ftxui;

/* TODOs 
* - implement different modes (log, rebase, ...)
* - refactor into own method
* - add bottom part of menu
*/

void Manager::init() {
    std::stringstream br_ss(branches);
    std::string branch;
    while (br_ss >> branch) { // TODO don't reuse variables (cleaner code)
    	branch_boxes->Add(Checkbox(branch, &branch_visibility_map[branch]));
    }
}

Manager::Manager(Component bb, std::string b, std::unordered_map<std::string, bool> bm, std::vector<Commit> c, size_t sc):
            branch_boxes(bb), branches(b), branch_visibility_map(bm), commits(c), selected_commit(sc) {
    init();
}

Component Manager::render() {
    return Renderer(branch_boxes, [&] {
	    // branch filter
	    std::vector<Element> bfb_elements = {
				text(L"filter branches") | bold,
				filler(),
				branch_boxes->Render() | vscroll_indicator | frame |
								size(HEIGHT, LESS_THAN, 10) | border,
			};
	    auto branch_filter_box = vbox(bfb_elements);
	    // TODO selected commit info
	    auto commit_info_box = vbox({
				text("commit info") | bold,
				filler(),
				text("hash:     " + commits[selected_commit].hash),
				filler(),
				text("subject:  " + commits[selected_commit].subject),
				filler(),
				text("date:     " + commits[selected_commit].date),
				filler(),
				text("parent:   " + commits[selected_commit].parent),
				filler(),
				text("checksum: " + commits[selected_commit].parent),
				filler(),
			});
	    // unify boxes
	    return vbox({
				branch_filter_box,
				separator(),
				commit_info_box,
			});
    });
}
