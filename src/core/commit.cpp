#include "commit.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <format>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ftxui/component/component_base.hpp>	// for Component, ComponentBase
#include "ftxui/component/component.hpp"// for Make
#include <ftxui/component/event.hpp> 			// for Event, Event::ArrowDown, Event::ArrowUp, Event::End, Event::Home, Event::PageDown, Event::PageUp
#include "ftxui/component/mouse.hpp"	// for Mouse, Mouse::WheelDown, Mouse::WheelUp
#include "ftxui/dom/elements.hpp"		// for operator|, Element, size, vbox, EQUAL, HEIGHT, dbox, reflect, focus, inverted, nothing, select, vscroll_indicator, yflex, yframe
#include "ftxui/dom/node.hpp"			// for Node
#include "ftxui/screen/box.hpp"			// for Box
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>  // for ScreenInteractive
#include "ftxui/screen/color.hpp"      	// for Color

#include "../util/cpplibostree.hpp"

#include "OSTreeTUI.hpp"

namespace CommitRender {

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

/// Partially inspired from https://github.com/ArthurSonzogni/FTXUI/blob/main/src/ftxui/component/window.cpp
Element DefaultRenderState(const WindowRenderState& state) {
	Element element = state.inner;
	//if (!state.active) {
	//	element |= dim;
	//}

	element = window(text(state.title), element);
	element |= clear_under;

	return element;
}

/// Draggable commit window, including ostree-tui logic for overlap detection, etc.
/// Partially inspired from https://github.com/ArthurSonzogni/FTXUI/blob/main/src/ftxui/component/window.cpp
class CommitComponentImpl : public ComponentBase, public WindowOptions {
public:
	explicit CommitComponentImpl(int position, std::string commit, OSTreeTUI& ostreetui) :
        hash(std::move(commit)),
		ostreetui(ostreetui),
		commit(ostreetui.getOstreeRepo().getCommitList().at(hash)),
		newVersion(this->commit.version),
		drag_initial_y(position * COMMIT_WINDOW_HEIGHT),
		drag_initial_x(1)
	{
    	inner = Renderer([&] {
    	  return vbox({
    	    text(ostreetui.getOstreeRepo().getCommitList().at(hash).subject),
					text(std::format("{:%Y-%m-%d %T %Ez}", std::chrono::time_point_cast<std::chrono::seconds>(ostreetui.getOstreeRepo().getCommitList().at(hash).timestamp))),
    	  });
    	});
    	simpleCommit = inner;
    	Add(inner);

    	title = hash.substr(0, 8);
    	top = drag_initial_y;
		left = drag_initial_x;
    	width = COMMIT_WINDOW_WIDTH;
    	height = COMMIT_WINDOW_HEIGHT;
	}

private:
	void resetWindow() {
    	left() = drag_initial_x;
    	top() = drag_initial_y;
    	width() = width_initial;
    	height() = height_initial;
    	// reset window contents
    	DetachAllChildren();
    	Add(simpleCommit);
	}
	
	void startPromotionWindow() {
    	left() = std::max(left(),-2);
    	width() = PROMOTION_WINDOW_WIDTH;
    	height() = PROMOTION_WINDOW_HEIGHT;
    	// change inner to promotion layout
    	simpleCommit = inner;
    	DetachAllChildren();
    	Add(promotionView);
		TakeFocus();
	}

	void executePromotion() {
		// promote on the ostree repo
		std::vector<std::string> metadataStrings;
		if (! newVersion.empty()) {
			metadataStrings.push_back("version=" + newVersion);
		}
	    ostreetui.promoteCommit(hash, ostreetui.getPromotionBranch(), metadataStrings, newSubject, true);
	    resetWindow();
	}

	void cancelPromotion() {
		ostreetui.setPromotionMode(false);
	    resetWindow();
	}

	Element Render() final {
		// check if promotion was started not from drag & drop, but from ostreetui
		if (ostreetui.getInPromotionSelection() && ostreetui.getPromotionHash() == hash) {
			startPromotionWindow();
		}

    	auto element = ComponentBase::Render();

    	const WindowRenderState state = {
    	    element,
    	    title(),
    	    Active(),
    	    drag_
    	};

    	element = render ? render(state) : DefaultRenderState(state);

    	// Position and record the drawn area of the window.
    	element |= reflect(box_window_);
    	element |= PositionAndSize(left(), top() + ostreetui.getScrollOffset(), width(), height());
    	element |= reflect(box_);

    	return element;
	}

	bool OnEvent(Event event) final {
    	if (ComponentBase::OnEvent(event)) {
    		return true;
    	}

		if (ostreetui.getInPromotionSelection()) {
			// navigate promotion branches
			if (event == Event::ArrowLeft) {
				const long int it = std::find(ostreetui.getColumnToBranchMap().begin(), ostreetui.getColumnToBranchMap().end(), ostreetui.getPromotionBranch()) - ostreetui.getColumnToBranchMap().begin();
				ostreetui.setPromotionBranch(ostreetui.getColumnToBranchMap().at((it - 1) % ostreetui.getColumnToBranchMap().size()));
				return true;
			}
			if (event == Event::ArrowRight) {
				const long int it = std::find(ostreetui.getColumnToBranchMap().begin(), ostreetui.getColumnToBranchMap().end(), ostreetui.getPromotionBranch()) - ostreetui.getColumnToBranchMap().begin();
				ostreetui.setPromotionBranch(ostreetui.getColumnToBranchMap().at((it + 1) % ostreetui.getColumnToBranchMap().size()));
				return true;
			}
			// promote
			if (event == Event::Return) {
				executePromotion();
				return true;
			}
			// cancel
			if (event == Event::Escape) {
				cancelPromotion();
        	    return true;
			}
		}

    	if (!event.is_mouse()) {
			return false;
    	}

    	if (ostreetui.getInPromotionSelection() && ! drag_) {
    		return true;
    	}

    	mouse_hover_ = box_window_.Contain(event.mouse().x, event.mouse().y);
		// potentially indicate mouse hover
    	//if (box_window_.Contain(event.mouse().x, event.mouse().y)) {}

    	if (captured_mouse_) {
    		if (event.mouse().motion == Mouse::Released) {
    	    	// reset mouse
    	    	captured_mouse_ = nullptr;
    	    	// check if position matches branch & do something if it does
    	    	if (ostreetui.getPromotionBranch().empty()) {
    	    		ostreetui.setPromotionMode(false, hash);
    	    		resetWindow();
    	    	} else {
					ostreetui.setPromotionMode(true, hash);
    	    	}
    	    	return true;
    		}

    		if (drag_) {
    	    	left() = event.mouse().x - drag_start_x - box_.x_min;
    	    	top() = event.mouse().y - drag_start_y - box_.y_min;
    	    	// promotion
    	    	ostreetui.setPromotionMode(true, hash); // TODO switch to ostreetui call
    	    	// calculate which branch currently is hovered over
    	    	ostreetui.setPromotionBranch("");
    	    	const int branch_pos = event.mouse().x / 2;
    	    	int count{0};
    	    	for (const auto& [branch,visible] : ostreetui.getVisibleBranches()) {
    	    		if (visible) {
    	    			++count;
    	    		}
    	    		if (count == branch_pos) {
    	    			ostreetui.setPromotionBranch(ostreetui.getColumnToBranchMap().at(event.mouse().x / 2 - 1));
    	    			break;
    	    		}
    	    	}
    	  	} else {
    	    	// not promotion
    	    	ostreetui.setPromotionMode(false, hash);
    		}

    		// Clamp the window size.
    		width() = std::max<int>(width(), static_cast<int>(title().size() + 2));
    		height() = std::max<int>(height(), 2);

    		return true;
    	}

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

    	drag_start_x = event.mouse().x - left() - box_.x_min;
    	drag_start_y = event.mouse().y - top() - box_.y_min;

    	const bool drag_old = drag_;
    	drag_ = true;
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

	int drag_initial_x;
	int drag_initial_y;
	int width_initial = COMMIT_WINDOW_WIDTH;
	int height_initial = COMMIT_WINDOW_HEIGHT;

	bool mouse_hover_ = false;
	bool drag_ = false;

	// ostree-tui specific members
	std::string hash;
	OSTreeTUI& ostreetui;

	// promotion view
	cpplibostree::Commit commit;
	std::string newSubject;
	std::string newVersion;
	Component simpleCommit = Renderer([] {
	  	return text("error in commit window creation");
	});
	Component promotionView = Container::Vertical({
	    Renderer([&] {
	    	return vbox({
	    		text(""),
	    		text(" Promote Commit...") | bold,
	    		text(""),
	    		text(" ☐ todocommithash") | bold,
	    	});
	    }),
	    Container::Horizontal({
	    	Renderer([&] {
	    		return text(" ┆ subject: ");
	    	}),
	    	Input(&newSubject, "enter new subject...") | underlined
	    }),
	    // render version, if available
		commit.version.empty()
			? Renderer([]{return filler();})
			: Container::Horizontal({
	    		Renderer([&] {
	    			return text(" ┆ version: ");
	    		}),
	    		Input(&newVersion, commit.version) | underlined
	    	}),
	    Renderer([&] {
	    	return vbox({
	    		text(" ┆"),
	    		text(" ┆ to branch:"),
	    		text(" ☐ " + ostreetui.getPromotionBranch()) | bold,
	    		text(" │") | bold
	    	});
	    }),
	    Container::Horizontal({
	    	Button(" Cancel ", [&] {
	            cancelPromotion();
	        }) | color(Color::Red) | flex,
	    	Button(" Promote ", [&] {
	           executePromotion();
	        }) | color(Color::Green) | flex,
	    })
	});
};

}  // namespace

ftxui::Component CommitComponent(int position, const std::string& commit, OSTreeTUI& ostreetui) {
	return ftxui::Make<CommitComponentImpl>(position, commit, ostreetui);
}

ftxui::Element commitRender(OSTreeTUI& ostreetui, const std::unordered_map<std::string, ftxui::Color>& branchColorMap, size_t selectedCommit) {
	using namespace ftxui;

	int scrollOffset = ostreetui.getScrollOffset();

	// check empty commit list
	if (ostreetui.getVisibleCommitViewMap().empty() || ostreetui.getVisibleBranches().empty()) {
		return color(Color::RedLight, text(" no commits to be shown ") | bold | center);
	}

	// stores the dedicated tree-column of each branch, -1 meaning not displayed yet
	std::unordered_map<std::string, int> usedBranches{};
	for (const auto& branchPair : ostreetui.getVisibleBranches()) {
		if (branchPair.second) {
			usedBranches[branchPair.first] = -1;
		}
	}
	int nextAvailableSpace = static_cast<int>(usedBranches.size() - 1);

	// - RENDER -
	// left tree, right commits
	Elements treeElements{};
	Elements commElements{};

	ostreetui.getColumnToBranchMap().clear();
	for (const auto& visibleCommitIndex : ostreetui.getVisibleCommitViewMap()) {
		const cpplibostree::Commit commit = ostreetui.getOstreeRepo().getCommitList().at(visibleCommitIndex);
		// branch head if it is first branch usage
		const std::string relevantBranch = commit.branch;
		if (usedBranches.at(relevantBranch) == -1) {
			ostreetui.getColumnToBranchMap().push_back(relevantBranch);
			usedBranches.at(relevantBranch) = nextAvailableSpace--;
		}
		// commit
		if (scrollOffset++ >= 0) {
			treeElements.push_back(addTreeLine(RenderTree::TREE_LINE_NODE, commit, usedBranches, branchColorMap));
		}
		for (int i{0}; i < 3; i++) {
			if (scrollOffset++ >= 0) {
				treeElements.push_back(addTreeLine(RenderTree::TREE_LINE_TREE, commit, usedBranches, branchColorMap));
			}
		}
  	}
	std::reverse(ostreetui.getColumnToBranchMap().begin(), ostreetui.getColumnToBranchMap().end());

	return hbox({
		vbox(std::move(treeElements)),
		vbox(std::move(commElements))
	});
}

ftxui::Element addTreeLine(const RenderTree& treeLineType,
				 const cpplibostree::Commit& commit,
				 const std::unordered_map<std::string, int>& usedBranches,
				 const std::unordered_map<std::string, ftxui::Color>& branchColorMap) {
	using namespace ftxui;

	const std::string relevantBranch = commit.branch;
	// create an empty branch tree line
	Elements tree(usedBranches.size(), text(COMMIT_NONE));
	
	// populate tree with all displayed branches
	for (const auto& branch : usedBranches) {
		if (branch.second == -1) {
			continue;
		}

		if (treeLineType == RenderTree::TREE_LINE_TREE || (treeLineType == RenderTree::TREE_LINE_IGNORE_BRANCH && branch.first != relevantBranch)) {
			tree.at(branch.second) = (text(COMMIT_TREE) | color(branchColorMap.at(branch.first)));
		} else if (treeLineType == RenderTree::TREE_LINE_NODE) {
			if (branch.first == relevantBranch) {
				tree.at(branch.second) = (text(COMMIT_NODE) | color(branchColorMap.at(branch.first)));
			} else {
				tree.at(branch.second) = (text(COMMIT_TREE) | color(branchColorMap.at(branch.first)));
			}
		}
    }

    return hbox(std::move(tree));
}

}  // namespace CommitRender
