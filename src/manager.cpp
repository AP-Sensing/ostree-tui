/*
 * Manager Window
 */

#include <iostream>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <memory> // for shared_ptr, allocator, __shared_ptr_access
#include <stdexcept> 
#include <string>
#include <array>
#include <string>
#include <vector>

#pragma once
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
//#include "ftxui/dom/table.hpp"

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "ftxui/component/screen_interactive.hpp" // for ScreenInteractive
// TODO fix veeeery dirty include...
#include "/home/timon/Workdir/ostree-tui/build/_deps/clip-src/clip.h" // for Clipboard

using namespace ftxui;

/* TODOs 
* - implement different modes (log, rebase, ...)
* - refactor into own method
* - add bottom part of menu
*/
class Manager {
public:
    // TODO keeping this data copy is very dirty -> refactor
    Component branch_boxes = Container::Vertical({});
    std::string branches;
    std::unordered_map<std::string, bool> branch_visibility_map;
    std::vector<Commit> commits;
    size_t selected_commit;

public:
    void init() {
        std::stringstream br_ss(branches);
        std::string branch;
	    while (br_ss >> branch) { // TODO don't reuse variables (cleaner code)
	    	branch_boxes->Add(Checkbox(branch, &branch_visibility_map[branch]));
	    }
    }

    Manager(Component bb, std::string b, std::unordered_map<std::string, bool> bm, std::vector<Commit> c, size_t sc):
                branch_boxes(bb), branches(b), branch_visibility_map(bm), commits(c), selected_commit(sc) {
        init();
    }

    auto render() {
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
};
