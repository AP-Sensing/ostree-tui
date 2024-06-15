#ifndef SCROLLER_H
#define SCROLLER_H

#include <ftxui/component/component.hpp>

#include "ftxui/component/component_base.hpp"  // for Component

namespace ftxui {
    /**
     * @brief This Scroller Element is a modified version of the Scroller in the following repository:
     *    Title: git-tui
     *    Author: Arthur Sonzogni
     *    Date: 2021
     *    Availability: https://github.com/ArthurSonzogni/git-tui/blob/master/src/scroller.cpp
     *
     * @param selectedCommit 
     * @param child 
     * @return Component
     */
    Component Scroller(size_t *selectedCommit, size_t elementLength, Component child);

} // namespace ftxui
#endif /* end of include guard: SCROLLER_H */
