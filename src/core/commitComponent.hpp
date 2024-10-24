#ifndef COMMITCOMPONENT_H
#define COMMITCOMPONENT_H

#include <ftxui/component/component.hpp>

#include "ftxui/component/component_base.hpp"  // for Component

#include "../util/cpplibostree.hpp"

namespace ftxui {
    /**
     * @brief
     *
     * @return Component
     */
    Component CommitComponent(int& scroll_offset, cpplibostree::Commit& commit, WindowOptions option);

} // namespace ftxui
#endif /* end of include guard: COMMITCOMPONENT_H */
