/*_____________________________________________________________
 | Manager Render
 |   Right portion of main window, includes branch filter &
 |   detailed commit info of the selected commit. In future
 |   different modes should be supported (like a rebase mode
 |   exchangeable with the commit info)
 |___________________________________________________________*/

#include <string>
#include <unordered_map>
#include <vector>

#include "ftxui/component/component.hpp"  // for Component

#include "commit.h"
#include "../util/cl_ostree.h"

/// Right postion of main window
class Manager {
public:
    // TODO keeping this data copy is very dirty -> refactor
    cl_ostree::OSTreeRepo* ostree_repo;
    Component branch_boxes = Container::Vertical({});
    std::string branches;
    std::unordered_map<std::string, bool> branch_visibility_map;
    std::vector<Commit> commits;
    size_t selected_commit;

public:
    void init();
    Manager(cl_ostree::OSTreeRepo* repo, Component bb, size_t sc);
    Component render();
};
