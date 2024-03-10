#include <string>
#include <unordered_map>
#include <vector>

#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop

#include "commit.h"

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
