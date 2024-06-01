#include "manager.hpp"

#include <cstdio>
#include <string>

#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "../util/cpplibostree.hpp"


Manager::Manager(cpplibostree::OSTreeRepo& repo, std::unordered_map<std::string, bool>& visible_branches) {
    using namespace ftxui;

	// branch visibility
	for (const auto& branch : repo.getBranches()) {
		branch_boxes->Add(Checkbox(branch, &(visible_branches.at(branch))));
	}
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
			window(text("subject"), paragraph(display_commit.subject)),
			filler(),
			text("hash:       " + display_commit.hash),
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

ftxui::Component ContentPromotionManager::composePromotionComponent(ftxui::Component& branch_selection, ftxui::Component& apply_button) {
	using namespace ftxui;

	return Container::Vertical({
		branch_selection,
		Container::Horizontal({
  	    	flags,
  	    	Container::Vertical({
  	    	    subject_component,
  	    	    Container::Horizontal({
  	    	        metadata_add,
  	    	        metadata_input,
  	    	    }),
  	    	}),
  		}),
		apply_button
	});
}

ftxui::Elements ContentPromotionManager::renderPromotionCommand(cpplibostree::OSTreeRepo& ostree_repo, const std::string& selected_commit_hash) {
    using namespace ftxui;
	
	Elements line;
	line.push_back(text("ostree commit") | bold);
    line.push_back(text(" --repo=" + ostree_repo.getRepoPath()) | bold);
	line.push_back(text(" -b " + ostree_repo.getBranches().at(branch_selected)) | bold);
    // flags
    for (int i = 0; i < 8; ++i) {
      	if (options_state[i]) {
        	line.push_back(text(" "));
        	line.push_back(text(options_label[i]) | dim);
     	}
    }
    // optional subject
    if (!new_subject.empty()) {
    	line.push_back(text(" -s ") | bold);
    	line.push_back(text(new_subject) | color(Color::BlueLight) | bold);
    }
    // metadata additions
	if (!metadata_entries.empty()) {
		line.push_back(text(" --add-metadata-string=\"") | bold);
    	for (auto& it : metadata_entries) {
    		line.push_back(text(" " + it) | color(Color::RedLight));
    	}
		line.push_back(text("\"") | bold);
	}
	// commit
	line.push_back(text(" --tree=ref=" + selected_commit_hash) | bold);

    return line;
}

ftxui::Element ContentPromotionManager::renderPromotionView(cpplibostree::OSTreeRepo& ostree_repo, cpplibostree::Commit& display_commit, ftxui::Component& branch_selection, ftxui::Component& apply_button) {
	using namespace ftxui;

	auto commit_hash =
		window(text("Commit"), text(display_commit.hash) | flex) | size(HEIGHT, LESS_THAN, 3);
	auto branch_win =
		window(text("New Branch"), branch_selection->Render() | vscroll_indicator | frame);
    auto flags_win =
        window(text("Flags"), flags->Render() | vscroll_indicator | frame);
    auto subject_win =
		window(text("Subject"), subject_component->Render());
    auto metadata_win =
        window(text("Metadata Strings"), hbox({
                                  vbox({
                                      hbox({
                                          text("Add: "),
                                          metadata_add->Render(),
                                      }) | size(WIDTH, EQUAL, 20) |
                                          size(HEIGHT, EQUAL, 1),
                                      filler(),
                                  }),
                                  separator(),
                                  metadata_input->Render() | vscroll_indicator | frame |
                                      size(HEIGHT, EQUAL, 3) | flex,
                              }));
	auto aButton_win =
		apply_button->Render() | color(Color::Green);
	
    return vbox({
			commit_hash,
			branch_win,
            hbox({
                flags_win,
                vbox({
                    subject_win | size(WIDTH, EQUAL, 20),
                    metadata_win | size(WIDTH, EQUAL, 60),
                }),
                filler(),
            }) | size(HEIGHT, LESS_THAN, 8),
            hflow(renderPromotionCommand(ostree_repo, display_commit.hash)) | flex_grow,
			aButton_win,
    }) | flex_grow;
}
