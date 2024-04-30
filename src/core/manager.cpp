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

#include "../util/cpplibostree.h"

using namespace ftxui;


Manager::Manager(cpplibostree::OSTreeRepo repo, std::unordered_map<std::string, bool> *visible_branches) {
    // branch visibility
	for (auto branch : repo.getBranches()) {
		branch_boxes->Add(Checkbox(branch, &(visible_branches->at(branch))));
	}
}

std::shared_ptr<Node> Manager::branchBoxRender(){
	// branch filter
	Elements bfb_elements = {
			text(L"filter branches") | bold,
			filler(),
			branch_boxes->Render() | vscroll_indicator,
		};
	return vbox(bfb_elements);
}

std::shared_ptr<Node> Manager::render(Commit display_commit) {
	// selected commit info
	Elements signatures;
	for (auto signature : display_commit.signatures) {
		signatures.push_back(
			text("    " + signature.pubkey_algorithm + " " + signature.fingerprint)
		);
	}
	return vbox({
			text("commit info") | bold,
			filler(),
			text("hash:       " + display_commit.hash),
			filler(),
			text("subject:    " + display_commit.subject),
			filler(),
			text("date:       " + display_commit.date),
			filler(),
			text("parent:     " + display_commit.parent),
			filler(),
			text("checksum:   " + display_commit.contentChecksum),
			filler(),
			display_commit.signatures.size() > 0 ? text("signatures: ") : text(""),
			vbox(signatures),
			filler(),
		});
}
