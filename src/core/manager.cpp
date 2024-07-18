#include "manager.hpp"

#include <cstdio>
#include <string>
#include <assert.h>

#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/event.hpp"      // for Event
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "../util/cpplibostree.hpp"

// Manager

Manager::Manager(const ftxui::Component& infoView, const ftxui::Component& filterView, const ftxui::Component& promotionView) {
	using namespace ftxui;

	tabSelection = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());

	tabContent = Container::Tab({
        	infoView,
			filterView,
			promotionView
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

// ContentPromotionManager

ContentPromotionManager::ContentPromotionManager(bool show_tooltips): show_tooltips(show_tooltips) {
	using namespace ftxui;

	subjectComponent = Input(&newSubject, "subject");
}

void ContentPromotionManager::setBranchRadiobox(ftxui::Component radiobox) {
    using namespace ftxui;

    branchSelection = CatchEvent(radiobox, [&](const Event& event) {
    	// copy commit id
    	if (event == Event::Return) {
    		subjectComponent->TakeFocus();
    	}
    	return false;
    });
}

void ContentPromotionManager::setApplyButton(ftxui::Component button) {
	applyButton = button; 
}

ftxui::Elements ContentPromotionManager::renderPromotionCommand(cpplibostree::OSTreeRepo& ostreeRepo, const std::string& selectedCommitHash) {
    using namespace ftxui;

	assert(branchSelection);
	assert(applyButton);
	
	Elements line;
	line.push_back(text("ostree commit") | bold);
    line.push_back(text(" --repo=" + ostreeRepo.getRepoPath()) | bold);
	line.push_back(text(" -b " + ostreeRepo.getBranches().at(static_cast<size_t>(selectedBranch))) | bold);
    line.push_back(text(" --keep-metadata") | bold);
	// optional subject
    if (!newSubject.empty()) {
    	line.push_back(text(" -s \"") | bold);
    	line.push_back(text(newSubject) | color(Color::BlueLight) | bold);
		line.push_back(text("\"") | bold);
    }
	// commit
	line.push_back(text(" --tree=ref=" + selectedCommitHash) | bold);

    return line;
}

ftxui::Component ContentPromotionManager::composePromotionComponent() {
	using namespace ftxui;

	return Container::Vertical({
		branchSelection,
		Container::Vertical({
			subjectComponent,
  	        applyButton,
  	    }),
	});
}

ftxui::Element ContentPromotionManager::renderPromotionView(cpplibostree::OSTreeRepo& ostreeRepo, int screenHeight, const cpplibostree::Commit& displayCommit) {
	using namespace ftxui;

	assert(branchSelection);
	assert(applyButton);

	// compute screen element sizes
	int screenOverhead		{8}; // borders, footer, etc.
	int commitWinHeight 	{3};
	int apsectWinHeight 	{8};
	int tooltipsWinHeight	{2};
	int branchSelectWinHeight = screenHeight - screenOverhead - commitWinHeight - apsectWinHeight - tooltipsWinHeight;
	// tooltips only get shown, if the window is sufficiently large
	if (branchSelectWinHeight < 4) {
		tooltipsWinHeight = 0;
		branchSelectWinHeight = 4;
	}

	// build elements
	auto commitHashElem	= vbox({text(" Commit: ") | bold | color(Color::Green), text(" " + displayCommit.hash)}) | flex;
	auto branchWin 		= window(text("New Branch"), branchSelection->Render() | vscroll_indicator | frame);
    auto subjectWin 	= window(text("Subject"), subjectComponent->Render()) | flex;
	auto applyButtonWin = applyButton->Render() | color(Color::Green) | size(WIDTH, GREATER_THAN, 9) | flex;

	auto toolTipContent = [&](size_t tip) {
		return vbox({
			separatorCharacter("⎯"),
			text(" 🛈 " + tool_tip_strings.at(tip)),
		});
	};
	auto toolTipsWin	= !show_tooltips || tooltipsWinHeight < 2 ? filler() : // only show if screen is reasonable size
						  branchSelection->Focused()	? toolTipContent(0) :
						  subjectComponent->Focused()	? toolTipContent(1) :
						  applyButton->Focused()		? toolTipContent(2) :
						  filler();

	// build element composition
    return vbox({
			commitHashElem | size(HEIGHT, EQUAL, commitWinHeight),
			branchWin | size(HEIGHT, LESS_THAN, branchSelectWinHeight),
            vbox({
				subjectWin,
				applyButtonWin,
            }) | flex | size(HEIGHT, LESS_THAN, apsectWinHeight),
            hflow(renderPromotionCommand(ostreeRepo, displayCommit.hash)) | flex_grow,
			filler(),
			toolTipsWin,
    }) | flex_grow;
}
