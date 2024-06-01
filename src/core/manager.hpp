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

/// Right postion of main window
class Manager {
public:
    ftxui::Component branch_boxes = ftxui::Container::Vertical({});

//public:
    Manager(cpplibostree::OSTreeRepo& repo, std::unordered_map<std::string, bool>& visible_branches);
    
    ftxui::Element branchBoxRender();

    static ftxui::Element renderInfo(const cpplibostree::Commit& display_commit);
    static ftxui::Element renderPromotionWindow(const cpplibostree::Commit& display_commit, ftxui::Component& rb);
};

class CommitInfoManager {

};

class BranchBoxManager {

};

class ContentPromotionManager {
public:
    // branch selection
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

public:
    ContentPromotionManager();

    ftxui::Component composePromotionComponent(ftxui::Component& branch_selection, ftxui::Component& apply_button);

    ftxui::Elements renderPromotionCommand(cpplibostree::OSTreeRepo& ostree_repo, const std::string& selected_commit_hash);

    ftxui::Element renderPromotionView(cpplibostree::OSTreeRepo& ostree_repo, cpplibostree::Commit& display_commit, ftxui::Component& branch_selection, ftxui::Component& apply_button);
};
