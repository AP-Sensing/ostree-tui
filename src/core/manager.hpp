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

/// Interchangeable View
class Manager {
public:
    int tab_index{0};

    std::vector<std::string> tab_entries = {
    	" Info ", " Filter ", " Promote "
  	};

    ftxui::Component tab_selection;
    ftxui::Component tab_content;

    // because the combination of all interchangeable views is very simple,
    // we can (in contrast to the other ones) render this one immediately
    ftxui::Component manager_renderer;

public:
    Manager(const ftxui::Component& info_view, const ftxui::Component& filter_view, const ftxui::Component& promotion_view);
};

class CommitInfoManager {
public:
    /**
     * @brief Build the info view Element.
     * 
     * @param display_commit Commit to display the information of.
     * @return ftxui::Element 
     */
    static ftxui::Element renderInfoView(const cpplibostree::Commit& display_commit);
};

class BranchBoxManager {
public:
    ftxui::Component branch_boxes = ftxui::Container::Vertical({});

public:
    BranchBoxManager(cpplibostree::OSTreeRepo& repo, std::unordered_map<std::string, bool>& visible_branches);
    
    /**
     * @brief Build the branch box Element.
     * 
     * @return ftxui::Element 
     */
    ftxui::Element branchBoxRender();
};

class ContentPromotionManager {
public:
    // branch selection
    ftxui::Component branch_selection; // must be set from OSTreeTUI
    int branch_selected{0};

    // flag selection
    ftxui::Component flags;

    const std::array<std::string, 8> options_label = {
  	    "--keep-metadata",
  	};
  	std::array<bool, 8> options_state = {
  	    false,
  	};

    // subject
    ftxui::Component subject_component;

    std::string new_subject{""};

    // apply button
    ftxui::Component apply_button; // must be set from OSTreeTUI

    // tool-tips
    bool show_tooltips{true};
    ftxui::Component tool_tips_comp;
    const std::vector<std::string> tool_tip_strings = {
        "Branch to promote the Commit to.",
        "Additional Flags to set.",
        "New subject for promoted Commit (optional).",
        "Apply the Commit Promotion (write to repository).",
    };

public:
    /**
     * @brief Constructor for the Promotion Manager.
     * 
     * @warning The branch_selection and apply_button have to be set
     * using the respective set-methods AFTER construction, as they
     * have to be constructed in the OSTreeTUI::main
     */
    ContentPromotionManager(bool show_tooltips = true);

    /// Setter
    void setBranchRadiobox(ftxui::Component radiobox);
    /// Setter
    void setApplyButton(ftxui::Component button);

    /**
     * @brief Build the promotion view Component
     * 
     * @warning branch_selection & apply_button have to be set first (checked through assert)
     * @return ftxui::Component 
     */
    ftxui::Component composePromotionComponent();

    /// renders the promotion command resulting from the current user settings (ostree commit ...) 
    ftxui::Elements renderPromotionCommand(cpplibostree::OSTreeRepo& ostree_repo, const std::string& selected_commit_hash);

    /**
     * @brief Build the promotion view Element
     * 
     * @warning branch_selection & apply_button have to be set first (checked through assert)
     * @return ftxui::Element
     */
    ftxui::Element renderPromotionView(cpplibostree::OSTreeRepo& ostree_repo, int screenHeight, const cpplibostree::Commit& display_commit);
};
