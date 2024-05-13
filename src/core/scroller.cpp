#include "scroller.hpp"

#include <algorithm>                           // for max, min
#include <ftxui/component/component_base.hpp>  // for Component, ComponentBase
#include <ftxui/component/event.hpp>  // for Event, Event::ArrowDown, Event::ArrowUp, Event::End, Event::Home, Event::PageDown, Event::PageUp
#include <utility>  // for move

#include "ftxui/component/component.hpp"  // for Make
#include "ftxui/component/mouse.hpp"  // for Mouse, Mouse::WheelDown, Mouse::WheelUp
#include "ftxui/dom/deprecated.hpp"  // for text
#include "ftxui/dom/elements.hpp"  // for operator|, Element, size, vbox, EQUAL, HEIGHT, dbox, reflect, focus, inverted, nothing, select, vscroll_indicator, yflex, yframe
#include "ftxui/dom/node.hpp"      // for Node
#include "ftxui/dom/requirement.hpp"  // for Requirement
#include "ftxui/screen/box.hpp"       // for Box


/*
 * This Scroller Element is a modified version of the Scroller in the following repository
 *    Title: git-tui
 *    Author: Arthur Sonzogni
 *    Date: 2021
 *    Availability: https://github.com/ArthurSonzogni/git-tui/blob/master/src/scroller.cpp
 */
namespace ftxui {

class ScrollerBase : public ComponentBase {
	public:
		ScrollerBase(size_t *selected_commit, Component child) : sc(selected_commit) { Add(child);  }

	private:
		size_t *sc{nullptr};

	Element Render() final {
    	auto focused = Focused() ? focus : ftxui::select;

    	Element background = ComponentBase::Render();
    	background->ComputeRequirement();
    	size_ = background->requirement().min_y;
    	return dbox({
    	        std::move(background),
    	        vbox({
    	            text(L"") | size(HEIGHT, EQUAL, static_cast<int>(*sc) * 3),
    	            text(L"") | focused,
    	        }),
    	    }) | vscroll_indicator | yframe | yflex | reflect(box_);
	}

	bool Focusable() const final { return true; }

	int size_ = 0;
	Box box_{};
};

Component Scroller(size_t *selected_commit, Component child) {
	return Make<ScrollerBase>(selected_commit, std::move(child));
}

}  // namespace ftxui
