/*
 * This SnappyWindow Component is a modified version of the Window in the following repository
 *    Title: ftxui
 *    Author: Arthur Sonzogni
 *    Date: 2023
 *    Availability: https://github.com/ArthurSonzogni/FTXUI/blob/main/src/ftxui/component/window.cpp
 *
 * TODO - This should probably be set to inheritance or something.
 *        Currently, this is A LOT of duplicate code and only for testing.
 */

#include "commitComponent.hpp"

#include <algorithm>                         	// for max, min
#include <utility>					
#include <memory>								// for move

#include <ftxui/component/component_base.hpp>	// for Component, ComponentBase
#include "ftxui/component/component.hpp"// for Make
#include <ftxui/component/event.hpp> 			// for Event, Event::ArrowDown, Event::ArrowUp, Event::End, Event::Home, Event::PageDown, Event::PageUp
#include "ftxui/component/mouse.hpp"	// for Mouse, Mouse::WheelDown, Mouse::WheelUp
#include "ftxui/dom/deprecated.hpp"		// for text
#include "ftxui/dom/elements.hpp"		// for operator|, Element, size, vbox, EQUAL, HEIGHT, dbox, reflect, focus, inverted, nothing, select, vscroll_indicator, yflex, yframe
#include "ftxui/dom/node.hpp"			// for Node
#include "ftxui/dom/requirement.hpp"	// for Requirement
#include "ftxui/screen/box.hpp"			// for Box
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  		// for text, window, hbox, vbox, size, clear_under, reflect, emptyElement
#include "ftxui/screen/color.hpp"      	// for Color
#include "ftxui/screen/screen.hpp"     	// for Screen

#include "../util/cpplibostree.hpp"

namespace ftxui {

namespace {

/// unmodified copy from https://github.com/ArthurSonzogni/FTXUI/blob/main/src/ftxui/component/window.cpp
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

/// unmodified copy from https://github.com/ArthurSonzogni/FTXUI/blob/main/src/ftxui/component/window.cpp
Element DefaultRenderState(const WindowRenderState& state) {
  Element element = state.inner;
  if (!state.active) {
    element |= dim;
  }

  element = window(text(state.title), element);
  element |= clear_under;

  const Color color = Color::Red;

  return element;
}

/// @brief Draggable commit window, including ostree-tui logic for overlap detection, etc.
class CommitComponentImpl : public ComponentBase, public WindowOptions {
 public:
  explicit CommitComponentImpl(int& scroll_offset, cpplibostree::Commit& commit, WindowOptions option) : scroll_offset(scroll_offset), WindowOptions(std::move(option)) {
    //inner = Renderer([&] {
    //  return vbox({
    //    text(commit.subject),
		//		text(std::format("{:%Y-%m-%d %T %Ez}", std::chrono::time_point_cast<std::chrono::seconds>(commit.timestamp))),
    //  });
    //});
    if (!inner) {
      inner = Make<ComponentBase>();
    }
    Add(inner);
  }

 private:
  Element Render() final {
    auto element = ComponentBase::Render();

    const bool captureable =
        captured_mouse_ || ScreenInteractive::Active()->CaptureMouse();

    const WindowRenderState state = {
        element,
        title(),
        Active(),
        drag_,
        resize_left_ || resize_right_ || resize_down_ || resize_top_,
        (resize_left_hover_ || resize_left_) && captureable,
        (resize_right_hover_ || resize_right_) && captureable,
        (resize_top_hover_ || resize_top_) && captureable,
        (resize_down_hover_ || resize_down_) && captureable,
    };

    element = render ? render(state) : DefaultRenderState(state);

    // Position and record the drawn area of the window.
    element |= reflect(box_window_);
    element |= PositionAndSize(left(), top(), width(), height());
    element |= reflect(box_);

    return element;
  }

  bool OnEvent(Event event) final {
    if (ComponentBase::OnEvent(event)) {
      return true;
    }

    if (!event.is_mouse()) {
      return false;
    }

    mouse_hover_ = box_window_.Contain(event.mouse().x, event.mouse().y);

    resize_down_hover_ = false;
    resize_top_hover_ = false;
    resize_left_hover_ = false;
    resize_right_hover_ = false;

    if (mouse_hover_) {
      resize_left_hover_ = event.mouse().x == left() + box_.x_min;
      resize_right_hover_ =
          event.mouse().x == left() + width() - 1 + box_.x_min;
      resize_top_hover_ = event.mouse().y == top() + box_.y_min;
      resize_down_hover_ = event.mouse().y == top() + height() - 1 + box_.y_min;

      // Apply the component options:
      resize_top_hover_ &= resize_top();
      resize_left_hover_ &= resize_left();
      resize_down_hover_ &= resize_down();
      resize_right_hover_ &= resize_right();
    }

    if (captured_mouse_) {
      if (event.mouse().motion == Mouse::Released) {
        captured_mouse_ = nullptr;
        left() = drag_initial_x;
        top() = drag_initial_y;
        return true;
      }

      if (resize_left_) {
        width() = left() + width() - event.mouse().x + box_.x_min;
        left() = event.mouse().x - box_.x_min;
      }

      if (resize_right_) {
        width() = event.mouse().x - resize_start_x - box_.x_min;
      }

      if (resize_top_) {
        height() = top() + height() - event.mouse().y + box_.y_min;
        top() = event.mouse().y - box_.y_min;
      }

      if (resize_down_) {
        height() = event.mouse().y - resize_start_y - box_.y_min;
      }

      if (drag_) {
        left() = event.mouse().x - drag_start_x - box_.x_min;
        top() = event.mouse().y - drag_start_y - box_.y_min;
      }

      // Clamp the window size.
      width() = std::max<int>(width(), static_cast<int>(title().size() + 2));
      height() = std::max<int>(height(), 2);

      return true;
    }

    resize_left_ = false;
    resize_right_ = false;
    resize_top_ = false;
    resize_down_ = false;

    if (!mouse_hover_) {
      return false;
    }

    if (!CaptureMouse(event)) {
      return true;
    }

    if (event.mouse().button != Mouse::Left) {
      return true;
    }
    if (event.mouse().motion != Mouse::Pressed) {
      return true;
    }

    TakeFocus();

    captured_mouse_ = CaptureMouse(event);
    if (!captured_mouse_) {
      return true;
    }

    resize_left_ = resize_left_hover_;
    resize_right_ = resize_right_hover_;
    resize_top_ = resize_top_hover_;
    resize_down_ = resize_down_hover_;

    resize_start_x = event.mouse().x - width() - box_.x_min;
    resize_start_y = event.mouse().y - height() - box_.y_min;
    drag_start_x = event.mouse().x - left() - box_.x_min;
    drag_start_y = event.mouse().y - top() - box_.y_min;

    // Drag only if we are not resizeing a border yet:
    bool drag_old = drag_;
    drag_ = !resize_right_ && !resize_down_ && !resize_top_ && !resize_left_;
    if (!drag_old && drag_) { // if we start dragging
      drag_initial_x = left();
      drag_initial_y = top();
    }
    return true;
  }

  // window specific members
  Box box_;
  Box box_window_;

  CapturedMouse captured_mouse_;
  int drag_start_x = 0;
  int drag_start_y = 0;
  int resize_start_x = 0;
  int resize_start_y = 0;

  int drag_initial_x = 0;
  int drag_initial_y = 0;

  bool mouse_hover_ = false;
  bool drag_ = false;
  bool resize_top_ = false;
  bool resize_left_ = false;
  bool resize_down_ = false;
  bool resize_right_ = false;

  bool resize_top_hover_ = false;
  bool resize_left_hover_ = false;
  bool resize_down_hover_ = false;
  bool resize_right_hover_ = false;

  int& scroll_offset;

  // ostree-tui specific members
  
};

}  // namespace


Component CommitComponent(int& scroll_offset, cpplibostree::Commit& commit, WindowOptions option) {
  return Make<CommitComponentImpl>(scroll_offset, commit, std::move(option));
}

};  // namespace ftxui

