#include "manager.hpp"

#include <cstdio>
#include <string>

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
	
	top_text_box = Renderer([&] { return text("Commit... ") | bold; });

	manager_renderer = Container::Vertical({
      	Container::Horizontal({
			top_text_box,
          	tab_selection
      	}),
      	tab_content,
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
			text("    " + signature.pubkey_algorithm + " " + signature.fingerprint)
		);
	}
	return vbox({
			window(text("Subject:"),
				paragraph(display_commit.subject) | color(Color::White)
			) | color(Color::Green),
			filler(),
			text(" Hash:        ") | color(Color::Green), 
			text(display_commit.hash),
			filler(),
			text(" Date:        ") | color(Color::Green),
			text(std::format("{:%Y-%m-%d %T %Ez}",
								std::chrono::time_point_cast<std::chrono::seconds>(display_commit.timestamp))),
			filler(),
			text(" Parent:      ") | color(Color::Green),
			text(display_commit.parent),
			filler(),
			text(" C.-Checksum: ") | color(Color::Green),
			text(display_commit.contentChecksum),
			filler(),
			display_commit.signatures.size() > 0 ? text("signatures: ") | color(Color::Green) : text(""),
			vbox(signatures),
			filler()
		});
}

// ContentPromotionManager

ContentPromotionManager::ContentPromotionManager() {
	using namespace ftxui;

	metadata_option.on_enter = [&] {
  	  	metadata_entries.push_back(metadata_add_content);
  	  	metadata_add_content = "";
  	};

	metadata_input = Menu(&metadata_entries, &input_selected);
	metadata_add = Input(&metadata_add_content, "metadata string", metadata_option);

	subject_component = Input(&new_subject, "subject");

	flags = Container::Vertical({
  	    Checkbox(&options_label[0], &options_state[0]),
  	});
}

ftxui::Elements ContentPromotionManager::renderPromotionCommand(cpplibostree::OSTreeRepo& ostree_repo, const std::string& selected_commit_hash) {
    using namespace ftxui;
	
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
    // metadata additions
	if (!metadata_entries.empty()) {
    	for (auto& it : metadata_entries) {
			line.push_back(text(" --add-metadata-string=\"") | bold);
    		line.push_back(text(it) | color(Color::RedLight));
			line.push_back(text("\"") | bold);
    	}
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
  	    	    Container::Horizontal({
  	    	        metadata_add,
  	    	        metadata_input,
  	    	    }),
				Container::Horizontal({
  	    	        subject_component,
  	    	        apply_button,
  	    	    }),
  	    	}),
  		}),
	});
}

ftxui::Element ContentPromotionManager::renderPromotionView(cpplibostree::OSTreeRepo& ostree_repo, cpplibostree::Commit& display_commit) {
	using namespace ftxui;

	auto commit_hash	= window(text("Commit"), text(display_commit.hash) | flex) | size(HEIGHT, LESS_THAN, 3);
	auto branch_win 	= window(text("New Branch"), branch_selection->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 2));
    auto flags_win 		= window(text("Flags"), flags->Render() | vscroll_indicator | frame);
    auto subject_win 	= window(text("Subject"), subject_component->Render()) | flex;
    auto metadata_win 	= 
		window(text("Metadata Strings"),
			hbox({
                hbox({
                    text("Add: "),
                    metadata_add->Render(),
                }) | size(WIDTH, EQUAL, 20) | size(HEIGHT, EQUAL, 1),
                separator(),
                metadata_input->Render() | vscroll_indicator | frame | size(HEIGHT, EQUAL, 2) | flex,
            })
		);
	auto aButton_win 	= apply_button->Render() | color(Color::Green) | size(WIDTH, GREATER_THAN, 9) | flex;
	
    return vbox({
			commit_hash,
			branch_win,
            hbox({
                flags_win,
                vbox({
                    metadata_win | size(WIDTH, EQUAL, 60),
                    hbox({
						subject_win | size(WIDTH, EQUAL, 20),
						aButton_win,
					}),
                }),
                filler(),
            }) | size(HEIGHT, LESS_THAN, 8),
            hflow(renderPromotionCommand(ostree_repo, display_commit.hash)) | flex_grow,
    }) | flex_grow;
}
