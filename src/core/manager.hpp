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
    ftxui::Component promotionRefSelection;
    int selectedPromotionRef{0};

//public:
    Manager(cpplibostree::OSTreeRepo& repo, std::unordered_map<std::string, bool>& visible_branches, std::vector<std::string>& branches);
    
    ftxui::Element branchBoxRender();

    static ftxui::Element renderInfo(const cpplibostree::Commit& display_commit);
    static ftxui::Element renderPromotionWindow(const cpplibostree::Commit& display_commit, ftxui::Component& rb);
};
