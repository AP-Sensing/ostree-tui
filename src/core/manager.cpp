#include "manager.hpp"

#include <cstdio>
#include <string>
#include <assert.h>

#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/event.hpp"      // for Event
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "../util/cpplibostree.hpp"

// Manager

Manager::Manager(const ftxui::Component& infoView, const ftxui::Component& filterView) {
	using namespace ftxui;

	tabSelection = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());

	tabContent = Container::Tab({
        	infoView,
			filterView
    	},
    	&tab_index);

	managerRenderer = Container::Vertical({
        tabSelection,
      	tabContent
  	});
}

// BranchBoxManager

BranchBoxManager::BranchBoxManager(cpplibostree::OSTreeRepo& repo, std::unordered_map<std::string, bool>& visibleBranches) {
    using namespace ftxui;

	// branch visibility
	for (const auto& branch : repo.getBranches()) {
		branchBoxes->Add(Checkbox(branch, &(visibleBranches.at(branch))));
	}
}

ftxui::Element BranchBoxManager::branchBoxRender(){
	using namespace ftxui;
	
	// branch filter
	Elements bfb_elements = {
			text(L"branches:") | bold,
			filler(),
			branchBoxes->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10),
		};
	return vbox(bfb_elements);
}

// CommitInfoManager

ftxui::Element CommitInfoManager::renderInfoView(const cpplibostree::Commit& displayCommit) {
	using namespace ftxui;
	
	// selected commit info
	Elements signatures;
	for (const auto& signature : displayCommit.signatures) {
		std::string ts = std::format("{:%Y-%m-%d %T %Ez}",
								std::chrono::time_point_cast<std::chrono::seconds>(signature.timestamp));
		signatures.push_back(vbox({
			hbox({
				text("‣ "),
				text(signature.pubkeyAlgorithm) | bold,
				text(" signature")
			}),
			text("  with key ID " + signature.fingerprint),
			text("  made " + ts)
		}));
	}
	return vbox({
			text(" Subject:") | color(Color::Green),
			paragraph(displayCommit.subject) | color(Color::White),
			filler(),
			text(" Hash: ") | color(Color::Green), 
			text(displayCommit.hash),
			filler(),
			text(" Date: ") | color(Color::Green),
			text(std::format("{:%Y-%m-%d %T %Ez}",
								std::chrono::time_point_cast<std::chrono::seconds>(displayCommit.timestamp))),
			filler(),
			// TODO insert version, only if exists
			displayCommit.version.empty()
				? filler()
				: text(" Version: ") | color(Color::Green),
			displayCommit.version.empty()
				? filler()
				: text(displayCommit.version),
			text(" Parent: ") | color(Color::Green),
			text(displayCommit.parent),
			filler(),
			text(" Checksum: ") | color(Color::Green),
			text(displayCommit.contentChecksum),
			filler(),
			displayCommit.signatures.size() > 0 ? text(" Signatures: ") | color(Color::Green) : text(""),
			vbox(signatures),
			filler()
		});
}
