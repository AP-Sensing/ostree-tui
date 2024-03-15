/*_____________________________________________________________
 | Manager Render
 |   Right portion of main window, includes branch filter &
 |   detailed commit info of the selected commit.
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
    Component branch_boxes = Container::Vertical({});
    std::string branches;
    std::unordered_map<std::string, bool> branch_visibility_map;
    std::vector<Commit> commits;
    size_t selected_commit;

public:
    void init();
    Manager(Component bb, std::string b, std::unordered_map<std::string, bool> bm, std::vector<Commit> c, size_t sc);
    Component render();
};
