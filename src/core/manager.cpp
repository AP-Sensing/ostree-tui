#include "manager.hpp"

#include <cstdio>
#include <string>
#include <assert.h>

#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/event.hpp"      // for Event
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "../util/cpplibostree.hpp"

// Manager

Manager::Manager(const ftxui::Component& info_view, const ftxui::Component& filter_view, const ftxui::Component& promotion_view) {
	using namespace ftxui;

	tab_selection = Menu(&tab_entries, &tab_index, MenuOption::HorizontalAnimated());

	tab_content = Container::Tab({
        	info_view,
			filter_view,
			promotion_view
    	},
    	&tab_index);

	manager_renderer = Container::Vertical({
        tab_selection,
      	tab_content
  	});
}

// BranchBoxManager

BranchBoxManager::BranchBoxManager(cpplibostree::OSTreeRepo& repo, std::unordered_map<std::string, bool>& visible_branches) {
    using namespace ftxui;

	// branch visibility
	for (const auto& branch : repo.getBranches()) {
		branch_boxes->Add(Checkbox(branch, &(visible_branches.at(branch))));
	}
}

ftxui::Element BranchBoxManager::branchBoxRender(){
	using namespace ftxui;
	
	// branch filter
	Elements bfb_elements = {
			text(L"branches:") | bold,
			filler(),
			branch_boxes->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10),
		};
	return vbox(bfb_elements);
}

// CommitInfoManager

ftxui::Element CommitInfoManager::renderInfoView(const cpplibostree::Commit& display_commit) {
	using namespace ftxui;
	
	// selected commit info
	Elements signatures;
	for (const auto& signature : display_commit.signatures) {
		signatures.push_back(
			text("• " + signature.pubkey_algorithm + " " + signature.fingerprint)
		);
	}
	return vbox({
			text(" Subject:") | color(Color::Green),
			paragraph(display_commit.subject) | color(Color::White),
			filler(),
			text(" Hash: ") | color(Color::Green), 
			text(display_commit.hash),
			filler(),
			text(" Date: ") | color(Color::Green),
			text(std::format("{:%Y-%m-%d %T %Ez}",
								std::chrono::time_point_cast<std::chrono::seconds>(display_commit.timestamp))),
			filler(),
			text(" Parent: ") | color(Color::Green),
			text(display_commit.parent),
			filler(),
			text(" Checksum: ") | color(Color::Green),
			text(display_commit.contentChecksum),
			filler(),
			display_commit.signatures.size() > 0 ? text(" Signatures: ") | color(Color::Green) : text(""),
			vbox(signatures),
			filler()
		});
}

// ContentPromotionManager

ContentPromotionManager::ContentPromotionManager(bool show_tooltips): show_tooltips(show_tooltips) {
	using namespace ftxui;

	subject_component = Input(&new_subject, "subject");

	flags = Container::Vertical({
  	    Checkbox(&options_label[0], &options_state[0]),
  	});
}

void ContentPromotionManager::setBranchRadiobox(ftxui::Component radiobox) {
    using namespace ftxui;

    branch_selection = CatchEvent(radiobox, [&](const Event& event) {
    	// copy commit id
    	if (event == Event::Return) {
    		flags->TakeFocus();
    	}
    	return false;
    });
}

void ContentPromotionManager::setApplyButton(ftxui::Component button) {
	apply_button = button; 
}

ftxui::Elements ContentPromotionManager::renderPromotionCommand(cpplibostree::OSTreeRepo& ostree_repo, const std::string& selected_commit_hash) {
    using namespace ftxui;

	assert(branch_selection);
	assert(apply_button);
	
	Elements line;
	line.push_back(text("ostree commit") | bold);
    line.push_back(text(" --repo=" + ostree_repo.getRepoPath()) | bold);
	line.push_back(text(" -b " + ostree_repo.getBranches().at(static_cast<size_t>(branch_selected))) | bold);
    // flags
    for (size_t i = 0; i < 8; ++i) {
      	if (options_state[i]) {
        	line.push_back(text(" "));
        	line.push_back(text(options_label[i]) | dim);
     	}
    }
    // optional subject
    if (!new_subject.empty()) {
    	line.push_back(text(" -s \"") | bold);
    	line.push_back(text(new_subject) | color(Color::BlueLight) | bold);
		line.push_back(text("\"") | bold);
    }
	// commit
	line.push_back(text(" --tree=ref=" + selected_commit_hash) | bold);

    return line;
}

ftxui::Component ContentPromotionManager::composePromotionComponent() {
	using namespace ftxui;

	return Container::Vertical({
		branch_selection,
		Container::Horizontal({
  	    	flags,
  	    	Container::Vertical({
				subject_component,
  	    	    apply_button,
  	    	}),
  		}),
	});
}

ftxui::Element ContentPromotionManager::renderPromotionView(cpplibostree::OSTreeRepo& ostree_repo, int screenHeight, const cpplibostree::Commit& display_commit) {
	using namespace ftxui;

	assert(branch_selection);
	assert(apply_button);

	// compute screen element sizes
	int screen_overhead		{8}; // borders, footer, etc.
	int commit_win_height 	{3};
	int apsect_win_height 	{8};
	int tooltips_win_height {2};
	int branch_select_win_height = screenHeight - screen_overhead - commit_win_height - apsect_win_height - tooltips_win_height;
	// tooltips only get shown, if the window is sufficiently large
	if (branch_select_win_height < 4) {
		tooltips_win_height = 0;
		branch_select_win_height = 4;
	}

	// build elements
	auto commit_hash	= vbox({text(" Commit: ") | bold | color(Color::Green), text(" " + display_commit.hash)}) | flex;
	auto branch_win 	= window(text("New Branch"), branch_selection->Render() | vscroll_indicator | frame);
    auto flags_win 		= window(text("Flags"), flags->Render() | vscroll_indicator | frame);
    auto subject_win 	= window(text("Subject"), subject_component->Render()) | flex;
	auto aButton_win 	= apply_button->Render() | color(Color::Green) | size(WIDTH, GREATER_THAN, 9) | flex;

	auto toolTipContent = [&](size_t tip) {
		return vbox({
			separatorCharacter("⎯"),
			text(" 🛈 " + tool_tip_strings.at(tip)),
		});
	};
	auto tool_tips_win	= !show_tooltips || tooltips_win_height < 2 ? filler() : // only show if screen is reasonable size
						  branch_selection->Focused()	? toolTipContent(0) :
						  flags->Focused()				? toolTipContent(1) :
						  subject_component->Focused()	? toolTipContent(2) :
						  apply_button->Focused()		? toolTipContent(3) :
						  filler();

	// build element composition
    return vbox({
			commit_hash | size(HEIGHT, EQUAL, commit_win_height),
			branch_win | size(HEIGHT, LESS_THAN, branch_select_win_height),
            hbox({
                flags_win,
                vbox({
					subject_win,
					aButton_win,
                }) | flex,
            }) | size(HEIGHT, LESS_THAN, apsect_win_height),
            hflow(renderPromotionCommand(ostree_repo, display_commit.hash)) | flex_grow,
			filler(),
			tool_tips_win,
    }) | flex_grow;
}
