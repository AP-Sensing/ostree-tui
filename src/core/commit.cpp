#include "commit.hpp"

#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

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

/// From https://github.com/ArthurSonzogni/FTXUI/blob/main/src/ftxui/component/window.cpp
Element DefaultRenderState(const WindowRenderState& state) {
  Element element = state.inner;
  if (!state.active) {
    element |= dim;
  }

  element = window(text(state.title), element);
  element |= clear_under;

  return element;
}

/// Draggable commit window, including ostree-tui logic for overlap detection, etc.
/// Partially inspired from https://github.com/ArthurSonzogni/FTXUI/blob/main/src/ftxui/component/window.cpp
class CommitComponentImpl : public ComponentBase, public WindowOptions {
 public:
  explicit CommitComponentImpl(int position,
                              int& scrollOffset,
                              bool& inPromotionSelection,
                              std::string& promotionHash,
                              std::string& promotionBranch,
                              std::unordered_map<std::string, bool>& visibleBranches,
                              std::vector<std::string>& columnToBranchMap,
                              std::string commit,
                              cpplibostree::OSTreeRepo& ostreerepo,
                              bool& refresh) :
            scrollOffset(scrollOffset),
            inPromotionSelection(inPromotionSelection),
            promotionHash(promotionHash),
            promotionBranch(promotionBranch),
            visibleBranches(visibleBranches),
            columnToBranchMap(columnToBranchMap),
            hash(commit),
            ostreerepo(ostreerepo),
            refresh(refresh) {
    inner = Renderer([&] {
      return vbox({
        text(ostreerepo.getCommitList().at(hash).subject),
				text(std::format("{:%Y-%m-%d %T %Ez}", std::chrono::time_point_cast<std::chrono::seconds>(ostreerepo.getCommitList().at(hash).timestamp))),
      });
    });
    simpleCommit = inner;
    Add(inner);

    title = hash.substr(0, 8);
    top = position * 4;
    left = 1;
    width = 30;
    height = 4;
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
    width() = width_initial + 8;
    height() = height_initial + 10;
    // change inner to promotion layout
    simpleCommit = inner;
    DetachAllChildren();
    Add(promotionView);
  }

  Element Render() final {
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
    element |= PositionAndSize(left(), top() + scrollOffset, width(), height());
    element |= reflect(box_);

    return element;
  }

  bool OnEvent(Event event) final {
    if (ComponentBase::OnEvent(event)) {
      return true;
    }

	if (inPromotionSelection) {
		if (event == Event::ArrowLeft) {
			int it = std::find(columnToBranchMap.begin(), columnToBranchMap.end(), promotionBranch) - columnToBranchMap.begin();
			promotionBranch = columnToBranchMap.at((it - 1) % columnToBranchMap.size());
			return true;
		}
		if (event == Event::ArrowRight) {
			int it = std::find(columnToBranchMap.begin(), columnToBranchMap.end(), promotionBranch) - columnToBranchMap.begin();
			promotionBranch = columnToBranchMap.at((it + 1) % columnToBranchMap.size());
			return true;
		}
		if (event == Event::Return) {
			ostreerepo.promoteCommit(hash, promotionBranch, {}, newSubject, true);
            resetWindow();
			inPromotionSelection = false;
            refresh = true;
			return true;
		}
	}

    if (!event.is_mouse()) {
	  return false;
    }

    if (inPromotionSelection && ! drag_) {
      return true;
    }

    mouse_hover_ = box_window_.Contain(event.mouse().x, event.mouse().y);

    if (mouse_hover_) {
      // TODO indicate window is draggable?
    }

    if (captured_mouse_) {
      if (event.mouse().motion == Mouse::Released) {
        // reset mouse
        captured_mouse_ = nullptr;
        // check if position matches branch & do something if it does
        if (promotionBranch.size() != 0) {
          // initiate promotion
          inPromotionSelection = true;
          promotionHash = hash;
          startPromotionWindow();
        } else {
          // not promotion
          inPromotionSelection = false;
          promotionBranch = "";
          resetWindow();
        }
        return true;
      }

      if (drag_) {
        left() = event.mouse().x - drag_start_x - box_.x_min;
        top() = event.mouse().y - drag_start_y - box_.y_min;
        // promotion
        inPromotionSelection = true;
        // calculate which branch currently is hovered over
        promotionBranch = "";
        int branch_pos = event.mouse().x / 2;
        int count{0};
        for (auto& [branch,visible] : visibleBranches) {
          if (visible) {
            ++count;
          }
          if (count == branch_pos) {
            // problem -> branch sorting not stored anywhere...
            // promotionBranch = branch;
            promotionBranch = columnToBranchMap.at(event.mouse().x / 2 - 1);
            break;
          }
        }
      } else {
        // not promotion
        inPromotionSelection = false;
        promotionBranch = "";
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

    bool drag_old = drag_;
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

  int drag_initial_x = 0;
  int drag_initial_y = 0;
  int width_initial = 30;
  int height_initial = 4;

  bool mouse_hover_ = false;
  bool drag_ = false;

  int& scrollOffset;

  // ostree-tui specific members
  // TODO store commit data
  std::string hash;
  cpplibostree::OSTreeRepo& ostreerepo;
  std::unordered_map<std::string, bool>& visibleBranches;
  std::vector<std::string>& columnToBranchMap;

  bool& refresh;
  
  // common / shared variables to set the promotion state
  bool& inPromotionSelection;
	std::string& promotionHash;
	std::string& promotionBranch;

  // promotion view
  std::string newSubject;
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
      // TODO render other metadata strings, if available (like version?)
      Renderer([&] {
        return vbox({
          text(" ┆"),
          text(" ┆ to branch:"),
          text(" ☐ " + promotionBranch) | bold,
          text(" │") | bold
        });
      }),
      Container::Horizontal({
        Button(" Cancel [n] ", [&] {
                inPromotionSelection = false;
                resetWindow();
                }) | color(Color::Red) | flex,
        Button(" Promote [y] ", [&] {
                inPromotionSelection = false;
                // promote on the ostree repo
                ostreerepo.promoteCommit(hash, promotionBranch, {}, newSubject, true);
                resetWindow();
                refresh = true;
                }) | color(Color::Green) | flex,
      })
  });
};

}  // namespace

ftxui::Component CommitComponent(int position,
                        int& scrollOffset,
                        bool& inPromotionSelection,
                        std::string& promotionHash,
                        std::string& promotionBranch,
                        std::unordered_map<std::string, bool>& visibleBranches,
                        std::vector<std::string>& columnToBranchMap,
                        std::string commit,
                        cpplibostree::OSTreeRepo& ostreerepo,
                        bool& refresh) {
	return ftxui::Make<CommitComponentImpl>(position, scrollOffset, inPromotionSelection, promotionHash, promotionBranch, visibleBranches, columnToBranchMap, commit, ostreerepo, refresh);
}

ftxui::Element commitRender(cpplibostree::OSTreeRepo& repo,
                            const std::vector<std::string>& visibleCommitMap,
                            const std::unordered_map<std::string, bool>& visibleBranches,
                            std::vector<std::string>& columnToBranchMap,
							const std::unordered_map<std::string, ftxui::Color>& branchColorMap,
                            int scrollOffset,
							size_t selectedCommit) {
	using namespace ftxui;

	// check empty commit list
	if (visibleCommitMap.size() <= 0 || visibleBranches.size() <= 0) {
		return color(Color::RedLight, text(" no commits to be shown ") | bold | center);
	}

	// stores the dedicated tree-column of each branch, -1 meaning not displayed yet
	std::unordered_map<std::string, int> usedBranches{};
	for (const auto& branchPair : visibleBranches) {
		if (branchPair.second) {
			usedBranches[branchPair.first] = -1;
		}
	}
	int nextAvailableSpace = usedBranches.size() - 1;

	// - RENDER -
	// left tree, right commits
	Elements treeElements{};
	Elements commElements{};

	std::string markedString = repo.getCommitList().at(visibleCommitMap.at(selectedCommit)).hash;
	columnToBranchMap.clear();
	for (const auto& visibleCommitIndex : visibleCommitMap) {
		cpplibostree::Commit commit = repo.getCommitList().at(visibleCommitIndex);
		// branch head if it is first branch usage
		std::string relevantBranch = commit.branch;
		if (usedBranches.at(relevantBranch) == -1) {
			columnToBranchMap.push_back(relevantBranch);
			usedBranches.at(relevantBranch) = nextAvailableSpace--;
			// TODO somehow incorporate the branch name into the commit-tree
			//addLine(RenderTree::TREE_LINE_IGNORE_BRANCH, RenderLine::BRANCH_HEAD,
			//		treeElements, commElements, commit, highlight, usedBranches, branchColorMap);
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
	std::reverse(columnToBranchMap.begin(), columnToBranchMap.end());

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

	std::string relevantBranch = commit.branch;
	// create an empty branch tree line
	Elements tree(usedBranches.size(), text(COMMIT_NONE));
	
	// populate tree with all displayed branches
	for (const auto& branch : usedBranches) {
		if (branch.second == -1) {
			continue;
		}

		if (treeLineType == RenderTree::TREE_LINE_IGNORE_BRANCH && branch.first != relevantBranch) {
			tree.at(branch.second) = (text(COMMIT_TREE) | color(branchColorMap.at(branch.first)));
		} else if (treeLineType == RenderTree::TREE_LINE_NODE) {
			if (branch.first == relevantBranch) {
				tree.at(branch.second) = (text(COMMIT_NODE) | color(branchColorMap.at(branch.first)));
			} else {
				tree.at(branch.second) = (text(COMMIT_TREE) | color(branchColorMap.at(branch.first)));
			}
		} else if (treeLineType == RenderTree::TREE_LINE_TREE) {
			tree.at(branch.second) = (text(COMMIT_TREE) | color(branchColorMap.at(branch.first)));
		}
    }

    return hbox(std::move(tree));
}

}  // namespace CommitRender
