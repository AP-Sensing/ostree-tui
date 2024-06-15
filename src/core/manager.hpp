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

    ftxui::Component tabSelection;
    ftxui::Component tabContent;

    // because the combination of all interchangeable views is very simple,
    // we can (in contrast to the other ones) render this one immediately
    ftxui::Component managerRenderer;

public:
    Manager(const ftxui::Component& infoView, const ftxui::Component& filterView, const ftxui::Component& promotionView);
};

class CommitInfoManager {
public:
    /**
     * @brief Build the info view Element.
     * 
     * @param displayCommit Commit to display the information of.
     * @return ftxui::Element 
     */
    static ftxui::Element renderInfoView(const cpplibostree::Commit& displayCommit);
};

class BranchBoxManager {
public:
    ftxui::Component branchBoxes = ftxui::Container::Vertical({});

public:
    BranchBoxManager(cpplibostree::OSTreeRepo& repo, std::unordered_map<std::string, bool>& visibleBranches);
    
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
    ftxui::Component branchSelection; // must be set from OSTreeTUI
    int selectedBranch{0};

    // subject
    ftxui::Component subjectComponent;

    std::string newSubject{""};

    // apply button
    ftxui::Component applyButton; // must be set from OSTreeTUI

    // tool-tips
    bool show_tooltips{true};
    ftxui::Component tool_tips_comp;
    const std::vector<std::string> tool_tip_strings = {
        "Branch to promote the Commit to.",
        "New subject for promoted Commit (optional).",
        "Apply the Commit Promotion (write to repository).",
    };

public:
    /**
     * @brief Constructor for the Promotion Manager.
     * 
     * @warning The branchSelection and applyButton have to be set
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
     * @warning branchSelection & applyButton have to be set first (checked through assert)
     * @return ftxui::Component 
     */
    ftxui::Component composePromotionComponent();

    /// renders the promotion command resulting from the current user settings (ostree commit ...) 
    ftxui::Elements renderPromotionCommand(cpplibostree::OSTreeRepo& ostreeRepo, const std::string& selectedCommitHash);

    /**
     * @brief Build the promotion view Element
     * 
     * @warning branchSelection & applyButton have to be set first (checked through assert)
     * @return ftxui::Element
     */
    ftxui::Element renderPromotionView(cpplibostree::OSTreeRepo& ostreeRepo, int screenHeight, const cpplibostree::Commit& displayCommit);
};
