#include "trashBin.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <format>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ftxui/component/component_base.hpp>  // for Component, ComponentBase
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>  // for Event, Event::ArrowDown, Event::ArrowUp, Event::End, Event::Home, Event::PageDown, Event::PageUp
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include "ftxui/component/component.hpp"           // for Make
#include "ftxui/component/mouse.hpp"               // for Mouse, Mouse::WheelDown, Mouse::WheelUp
#include "ftxui/dom/elements.hpp"  // for operator|, Element, size, vbox, EQUAL, HEIGHT, dbox, reflect, focus, inverted, nothing, select, vscroll_indicator, yflex, yframe
#include "ftxui/dom/node.hpp"      // for Node
#include "ftxui/screen/box.hpp"    // for Box
#include "ftxui/screen/color.hpp"  // for Color

#include "../util/cpplibostree.hpp"

#include "OSTreeTUI.hpp"

namespace TrashBin {

namespace {
using namespace ftxui;

/// From https://github.com/ArthurSonzogni/FTXUI/blob/main/src/ftxui/component/window.cpp
Decorator PositionAndSize(int left, int top, int width, int height) {
    return [=](Element element) {
        element |= size(WIDTH, EQUAL, width);
        element |= size(HEIGHT, EQUAL, height);

        auto padding_left = emptyElement() | size(WIDTH, EQUAL, left);
        auto padding_top = emptyElement() | size(HEIGHT, EQUAL, top);

        return vbox({
            padding_top,
            hbox({
                padding_left,
                element,
            }),
        });
    };
}

/// Trash-bin reacting to mouse overlap
class TrashBinComponentImpl : public ComponentBase, public WindowOptions {
   public:
    explicit TrashBinComponentImpl(OSTreeTUI& ostreetui) : ostreetui(ostreetui) {
        inner = content;
        Add(inner);

        top = 0;
        left = 0;
        width = BIN_WINDOW_WIDTH;
        height = BIN_WINDOW_HEIGHT;
    }

   private:
    void showBin() {
        top() = ostreetui.GetScreen().dimy() - 8;
        left() = -5;
    }

    void hideBin() { left() = -100; }

    Element Render() final {
        // check if in deletion or stuff
        if (ostreetui.GetViewMode() == ViewMode::COMMIT_DRAGGING) {
            showBin();
        } else {
            hideBin();
        }

        auto element = ComponentBase::Render();

        element = window(text(""), element) | bold;
        element |= color(Color::Red);
        element |= clear_under;
        element |= PositionAndSize(left(), top(), width(), height());

        return element;
    }

    bool OnEvent(Event /*event*/) final { return false; }

   private:
    OSTreeTUI& ostreetui;
    Component content = Renderer([] { return text(" Drop Commit ") | bold; });
};

}  // namespace

ftxui::Component TrashBinComponent(OSTreeTUI& ostreetui) {
    return ftxui::Make<TrashBinComponentImpl>(ostreetui);
}

}  // namespace TrashBin
