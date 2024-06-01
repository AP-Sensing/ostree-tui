/*_____________________________________________________________
 | Manager Render
 |   Right portion of main window, includes branch filter &
 |   detailed commit info of the selected commit. In future
 |   different modes should be supported (like a rebase mode
 |   exchangeable with the commit info)
 |___________________________________________________________*/

#include <string>
#include <unordered_map>

#include "ftxui/component/component.hpp"  // for Component

#include "../util/cpplibostree.hpp"
#include "commit.hpp"

/// Interchangeable View
class Manager {
public:
    int tab_index{0};

    std::vector<std::string> tab_entries = {
    	" Info ", " Filter ", " Promote "
  	};

    ftxui::Component tab_selection;
    ftxui::Component tab_content;

    ftxui::Component top_text_box;

    // because the combination of all interchangeable views is very simple,
    // we can (in contrast to the other ones) render this one immediately
    ftxui::Component manager_renderer;

public:
    Manager(const ftxui::Component& info_view, const ftxui::Component& filter_view, const ftxui::Component& promotion_view);
};

class CommitInfoManager {
public:
    static ftxui::Element renderInfoView(const cpplibostree::Commit& display_commit);
};

class BranchBoxManager {
public:
    ftxui::Component branch_boxes = ftxui::Container::Vertical({});

public:
    BranchBoxManager(cpplibostree::OSTreeRepo& repo, std::unordered_map<std::string, bool>& visible_branches);
    
    ftxui::Element branchBoxRender();
};

class ContentPromotionManager {
public:
    // branch selection
    ftxui::Component branch_selection; // must be set from OSTreeTUI
    int branch_selected{0};

    // flag selection
    ftxui::Component flags;

    std::array<std::string, 8> options_label = {
  	    "--keep-metadata",
  	};
  	std::array<bool, 8> options_state = {
  	    false,
  	};

    // metadata entries
    ftxui::Component metadata_add;

    std::vector<std::string> metadata_entries;
  	int input_selected{0};

    ftxui::Component metadata_input;
    
    ftxui::InputOption metadata_option = ftxui::InputOption();
  	std::string metadata_add_content;

    // subject
    ftxui::Component subject_component;

    std::string new_subject{""};

    // apply button
    ftxui::Component apply_button; // must be set from OSTreeTUI

public:
    ContentPromotionManager();

    /// Warning: branch_selection & apply_button have to be set first (TODO check in method)
    ftxui::Component composePromotionComponent();

    /// renders the promotion command resulting from the current user settings (ostree commit ...) 
    ftxui::Elements renderPromotionCommand(cpplibostree::OSTreeRepo& ostree_repo, const std::string& selected_commit_hash);

    /// Warning: branch_selection & apply_button have to be set first (TODO check in method)
    ftxui::Element renderPromotionView(cpplibostree::OSTreeRepo& ostree_repo, cpplibostree::Commit& display_commit);
};
