#include "manager.hpp"

#include <cstdio>
#include <string>

#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "../util/cpplibostree.hpp"


Manager::Manager(cpplibostree::OSTreeRepo& repo, std::unordered_map<std::string, bool>& visible_branches, std::vector<std::string>& branches) {
    using namespace ftxui;

	// branch visibility
	for (const auto& branch : repo.getBranches()) {
		branch_boxes->Add(Checkbox(branch, &(visible_branches.at(branch))));
	}

	promotionRefSelection = Radiobox(&branches, &selectedPromotionRef);
}

ftxui::Element Manager::branchBoxRender(){
	using namespace ftxui;
	
	// branch filter
	Elements bfb_elements = {
			text(L"branches:") | bold,
			filler(),
			branch_boxes->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10),
		};
	return vbox(bfb_elements);
}

ftxui::Element Manager::renderInfo(const cpplibostree::Commit& display_commit) {
	using namespace ftxui;
	
	// selected commit info
	Elements signatures;
	for (const auto& signature : display_commit.signatures) {
		signatures.push_back(
			text("    " + signature.pubkey_algorithm + " " + signature.fingerprint)
		);
	}
	return vbox({
			text("hash:       " + display_commit.hash),
			filler(),
			paragraph("subject:    " + display_commit.subject),
			filler(),
			text("date:       " + std::format("{:%Y-%m-%d %T %Ez}",
								std::chrono::time_point_cast<std::chrono::seconds>(display_commit.timestamp))),
			filler(),
			text("parent:     " + display_commit.parent),
			filler(),
			text("checksum:   " + display_commit.contentChecksum),
			filler(),
			display_commit.signatures.size() > 0 ? text("signatures: ") : text(""),
			vbox(signatures),
			filler()
		});
}

ftxui::Element Manager::renderPromotionWindow(const cpplibostree::Commit& display_commit, ftxui::Component& rb) {
	using namespace ftxui;

	return  window(text("Commit"), vbox({
			text("hash:       " + display_commit.hash),
		}));
}
