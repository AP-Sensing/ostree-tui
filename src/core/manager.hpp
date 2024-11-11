/*_____________________________________________________________
 | Manager Render
 |   Right portion of main window, includes branch filter &
 |   detailed commit info of the selected commit. In future
 |   different modes should be supported (like a rebase mode
 |   exchangeable with the commit info)
 |___________________________________________________________*/
#pragma once

#include <string>
#include <unordered_map>

#include "ftxui/component/component.hpp"  // for Component

#include "../util/cpplibostree.hpp"

class OSTreeTUI;

/// Interchangeable View
class Manager {
public:
    Manager(OSTreeTUI& ostreetui, const ftxui::Component& infoView, const ftxui::Component& filterView);

private:
    OSTreeTUI& ostreetui;

    int tab_index{0};
    std::vector<std::string> tab_entries = {
    	" Info ", " Filter "
  	};

    // because the combination of all interchangeable views is very simple,
    // we can (in contrast to the other ones) render this one here
    ftxui::Component managerRenderer;
    ftxui::Component tabSelection;
    ftxui::Component tabContent;

public:
    ftxui::Component getManagerRenderer();
    const int& getTabIndex() const;
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
