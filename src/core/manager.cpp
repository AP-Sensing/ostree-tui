#include "manager.h"

#include <cstdio>
#include <sstream>
#include <string>

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

#include "../util/commandline.h"
#include "../util/cpplibostree.h"

using namespace ftxui;


void Manager::init() {
	// branch visibility
	branch_visibility_map = {};
	branches = ostree_repo->getBranchesAsString();
	std::stringstream branches_ss(branches);
	std::string branch;
	while (branches_ss >> branch) {
		branch_visibility_map[branch] = true;
		branch_boxes->Add(Checkbox(branch, &branch_visibility_map[branch]));
	}
}

Manager::Manager(cpplibostree::OSTreeRepo* repo, Component bb, size_t sc):
            ostree_repo(repo),
			branch_boxes(bb),
			commits(*repo->getCommitListSorted()),
			selected_commit(sc) {
    init();
}

Component Manager::render() {
    return Renderer(branch_boxes, [&] {
	    // branch filter
	    std::vector<Element> bfb_elements = {
				text(L"filter branches") | bold,
				filler(),
				branch_boxes->Render() | vscroll_indicator,
			};
	    auto branch_filter_box = vbox(bfb_elements);
	    // selected commit info
	    auto commit_info_box = vbox({
				text("commit info") | bold,
				filler(),
				text("hash:       " + commits[selected_commit].hash),
				filler(),
				text("subject:    " + commits[selected_commit].subject),
				filler(),
				text("date:       " + commits[selected_commit].date),
				filler(),
				//text("parent:     " + commits[selected_commit].parent),
				//filler(),
				text("checksum:   " + commits[selected_commit].parent),
				filler(),
				commits[selected_commit].signatures.size() > 0 ? text("signatures: ") : text(""),
				// TODO make dynamic
				text((commits[selected_commit].signatures.size() > 0 ? "    " + commits[selected_commit].signatures.at(0).pubkey_algorithm + " " + commits[selected_commit].signatures.at(0).fingerprint : "")),
				text((commits[selected_commit].signatures.size() > 1 ? "    " + commits[selected_commit].signatures.at(1).pubkey_algorithm + " " + commits[selected_commit].signatures.at(1).fingerprint : "")),
				text((commits[selected_commit].signatures.size() > 2 ? "    " + commits[selected_commit].signatures.at(2).pubkey_algorithm + " " + commits[selected_commit].signatures.at(2).fingerprint : "")),
				filler(),
			});
	    // unify boxes
	    return vbox({
				branch_filter_box,
				separator(),
				commit_info_box
			});
    });
}
