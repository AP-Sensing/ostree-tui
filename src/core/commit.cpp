#include "commit.hpp"

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

/// Partially inspired from
/// https://github.com/ArthurSonzogni/FTXUI/blob/main/src/ftxui/component/window.cpp
Element DefaultRenderState(const WindowRenderState& state,
                           ftxui::Color selectedColor = Color::White,
                           bool dimmable = true) {
    Element element = state.inner;
    if (!dimmable) {
        selectedColor = Color::White;
    }
    if (selectedColor == Color::White && dimmable) {
        element |= dim;
    } else {
        element |= bold;
    }
    element |= color(selectedColor);

    element = window(text(state.title), element);
    element |= clear_under;

    return element;
}

/// Draggable commit window, including ostree-tui logic for overlap detection, etc.
/// Partially inspired from
/// https://github.com/ArthurSonzogni/FTXUI/blob/main/src/ftxui/component/window.cpp
class CommitComponentImpl : public ComponentBase, public WindowOptions {
   public:
    explicit CommitComponentImpl(int position, std::string commit, OSTreeTUI& ostreetui)
        : commitPosition(position),
          hash(std::move(commit)),
          ostreetui(ostreetui),
          commit(ostreetui.GetOstreeRepo().getCommitList().at(hash)),
          newVersion(this->commit.version),
          drag_initial_y(position * COMMIT_WINDOW_HEIGHT),
          drag_initial_x(1) {
        inner = Renderer([&] {
            return vbox({
                text(ostreetui.GetOstreeRepo().getCommitList().at(hash).subject),
                text(
                    std::format("{:%Y-%m-%d %T %Ez}",
                                std::chrono::time_point_cast<std::chrono::seconds>(
                                    ostreetui.GetOstreeRepo().getCommitList().at(hash).timestamp))),
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
    void resetWindow(bool positionReset = true) {
        if (positionReset) {
            left() = drag_initial_x;
            top() = drag_initial_y;
        }
        width() = width_initial;
        height() = height_initial;
        // reset window contents
        DetachAllChildren();
        Add(simpleCommit);
    }

    void startPromotionWindow() {
        left() = std::max(left(), -2);
        width() = PROMOTION_WINDOW_WIDTH;
        height() = PROMOTION_WINDOW_HEIGHT;
        // change inner to promotion layout
        DetachAllChildren();
        Add(promotionView);
        if (!Focused()) {
            TakeFocus();
            // manually fix focus issues
            // change to proper control layout if possible...
            ostreetui.GetScreen().Post(Event::ArrowDown);
            ostreetui.GetScreen().Post(Event::ArrowRight);
            ostreetui.GetScreen().Post(Event::ArrowDown);
            ostreetui.GetScreen().Post(Event::ArrowRight);
            ostreetui.GetScreen().Post(Event::ArrowUp);
        }
    }

    void startDeletionWindow() {
        left() = std::max(left(), -2);
        width() = DELETION_WINDOW_WIDTH;
        height() = DELETION_WINDOW_HEIGHT;
        // change inner to deletion layout
        DetachAllChildren();
        Add(deletionView);
        TakeFocus();
    }

    void executePromotion() {
        // promote on the ostree repo
        std::vector<std::string> metadataStrings;
        if (!newVersion.empty()) {
            metadataStrings.push_back("version=" + newVersion);
        }
        ostreetui.PromoteCommit(hash, ostreetui.GetModeBranch(), metadataStrings, newSubject, true);
        resetWindow();
    }

    void executeDeletion() {
        // delete on the ostree repo
        ostreetui.DropLastCommit(ostreetui.GetOstreeRepo().getCommitList().at(hash));
        resetWindow();
    }

    void cancelSpecialWindow() {
        ostreetui.SetViewMode(ViewMode::DEFAULT);
        resetWindow();
    }

    Element Render() final {
        // check if promotion was started not from drag & drop, but from ostreetui
        if (ostreetui.GetViewMode() == ViewMode::COMMIT_DRAGGING &&
            ostreetui.GetModeHash() == hash) {
            resetWindow(false);
        } else if (ostreetui.GetViewMode() == ViewMode::COMMIT_PROMOTION &&
                   ostreetui.GetModeHash() == hash) {
            if (!ostreetui.GetModeBranch().empty()) {
                startPromotionWindow();
            } else {
                resetWindow(false);
            }
        } else if (ostreetui.GetViewMode() == ViewMode::COMMIT_DROP &&
                   ostreetui.GetModeHash() == hash) {
            const auto& commitList = ostreetui.GetOstreeRepo().getCommitList();
            auto commit = commitList.at(hash);
            if (ostreetui.GetOstreeRepo().IsMostRecentCommitOnBranch(hash)) {
                startDeletionWindow();
            } else {
                ostreetui.SetNotificationText("Can't drop commit " + commit.hash.substr(0, 8) +
                                             "... not last commit on branch ");
                cancelSpecialWindow();
            }
        }

        auto element = ComponentBase::Render();

        const WindowRenderState state = {element, title(), Active(), drag_};

        if (commitPosition == ostreetui.GetSelectedCommit()) {  // selected & not in promotion
            element =
                render ? render(state)
                       : DefaultRenderState(state, ostreetui.GetBranchColorMap().at(commit.branch),
                                            ostreetui.GetModeHash() != hash);
        } else {
            element =
                render ? render(state)
                       : DefaultRenderState(state, Color::White, ostreetui.GetModeHash() != hash);
        }

        // Position and record the drawn area of the window.
        element |= reflect(box_window_);
        element |= PositionAndSize(left(), top() + ostreetui.GetScrollOffset(), width(), height());
        element |= reflect(box_);

        return element;
    }

    bool OnEvent(Event event) final {
        if (ComponentBase::OnEvent(event)) {
            return true;
        }

        // exit special window
        if (ostreetui.GetViewMode() != ViewMode::DEFAULT && event == Event::Escape) {
            cancelSpecialWindow();
            return true;
        }

        // promotion window specific
        if (ostreetui.GetViewMode() == ViewMode::COMMIT_PROMOTION) {
            // navigate promotion branches
            if (event == Event::ArrowLeft) {
                const long int it =
                    std::find(ostreetui.GetColumnToBranchMap().begin(),
                              ostreetui.GetColumnToBranchMap().end(), ostreetui.GetModeBranch()) -
                    ostreetui.GetColumnToBranchMap().begin();
                ostreetui.SetModeBranch(ostreetui.GetColumnToBranchMap().at(
                    (it - 1) % ostreetui.GetColumnToBranchMap().size()));
                return true;
            }
            if (event == Event::ArrowRight) {
                const long int it =
                    std::find(ostreetui.GetColumnToBranchMap().begin(),
                              ostreetui.GetColumnToBranchMap().end(), ostreetui.GetModeBranch()) -
                    ostreetui.GetColumnToBranchMap().begin();
                ostreetui.SetModeBranch(ostreetui.GetColumnToBranchMap().at(
                    (it + 1) % ostreetui.GetColumnToBranchMap().size()));
                return true;
            }
        }

        if (!event.is_mouse()) {
            return false;
        }

        if ((ostreetui.GetViewMode() == ViewMode::COMMIT_PROMOTION ||
             ostreetui.GetViewMode() == ViewMode::COMMIT_DRAGGING) &&
            !drag_) {
            return true;
        }

        mouse_hover_ = box_window_.Contain(event.mouse().x, event.mouse().y);

        if (mouse_hover_ && event.mouse().button == Mouse::Left) {
            ostreetui.SetSelectedCommit(commitPosition);
        }

        if (captured_mouse_) {
            if (event.mouse().motion == Mouse::Released) {
                // reset mouse
                captured_mouse_ = nullptr;
                // drop commit
                if (event.mouse().y > ostreetui.GetScreen().dimy() - 8) {
                    ostreetui.SetViewMode(ViewMode::COMMIT_DROP, hash);
                    top() = drag_initial_y;
                }
                // check if position matches branch & do something if it does
                else if (ostreetui.GetModeBranch().empty()) {
                    ostreetui.SetViewMode(ViewMode::DEFAULT, hash);
                    resetWindow();
                } else {
                    ostreetui.SetViewMode(ViewMode::COMMIT_PROMOTION, hash);
                }
                return true;
            }

            if (drag_) {
                left() = event.mouse().x - drag_start_x - box_.x_min;
                top() = event.mouse().y - drag_start_y - box_.y_min;
                ostreetui.SetViewMode(ViewMode::COMMIT_DRAGGING, hash);
                // check if potential commit deletion
                if (event.mouse().y > ostreetui.GetScreen().dimy() - 8) {
                    ostreetui.SetModeBranch("");
                } else {
                    // potential promotion
                    ostreetui.SetViewMode(ViewMode::COMMIT_DRAGGING, hash, false);
                    // calculate which branch currently is hovered over
                    ostreetui.SetModeBranch("");
                    const int branch_pos = event.mouse().x / 2;
                    int count{0};
                    for (const auto& [branch, visible] : ostreetui.GetVisibleBranches()) {
                        if (visible) {
                            ++count;
                        }
                        if (count == branch_pos) {
                            ostreetui.SetViewMode(ViewMode::COMMIT_PROMOTION, hash);
                            ostreetui.SetModeBranch(
                                ostreetui.GetColumnToBranchMap().at(event.mouse().x / 2 - 1));
                            break;
                        }
                    }
                }
            } else {
                // not promotion
                ostreetui.SetViewMode(ViewMode::DEFAULT);
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
        if (!drag_old && drag_) {  // if we start dragging
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
    int commitPosition;
    std::string hash;
    OSTreeTUI& ostreetui;

    // promotion view
    cpplibostree::Commit commit;
    std::string newSubject;
    std::string newVersion;
    Component simpleCommit = Renderer([] { return text("error in commit window creation"); });
    Component promotionView = Container::Vertical(
        {Renderer([&] {
             return vbox({
                 text(""),
                 text(" Promote Commit...") | bold,
                 text(""),
                 text(" ☐ " + hash.substr(0, 8)) | bold,
             });
         }),
         Container::Horizontal({Renderer([&] { return text(" ┆ subject: "); }),
                                Input(&newSubject, "enter new subject...") | underlined}),
         // render version, if available
         commit.version.empty()
             ? Renderer([] { return filler(); })
             : Container::Horizontal({Renderer([&] { return text(" ┆ version: "); }),
                                      Input(&newVersion, commit.version) | underlined}),
         Renderer([&] {
             return vbox({text(" ┆"), text(" ┆ to branch:"),
                          text(" ☐ " + ostreetui.GetModeBranch()) | bold, text(" │") | bold});
         }),
         Container::Horizontal({
             Button(" Cancel ", [&] { cancelSpecialWindow(); }) | color(Color::Red) | flex,
             Button(" Promote ", [&] { executePromotion(); }) | color(Color::Green) | flex,
         })});
    Component deletionView = Container::Vertical(
        {Renderer([&] {
             return vbox({text(""), text(" Remove Commit...") | bold, text(""),
                          hbox({
                              text(" ✖ ") | color(Color::Red),
                              text(hash.substr(0, 8)) | bold | color(Color::Red),
                          }),
                          text(" ✖ " + commit.subject) | color(Color::Red),
                          text(" ✖") | color(Color::Red),
                          text(" ☐ " + ostreetui.GetModeBranch()) | dim, text(" │") | dim});
         }),
         Container::Horizontal({
             Button(" Cancel ", [&] { cancelSpecialWindow(); }) | color(Color::Red) | flex,
             Button(" Remove ", [&] { executeDeletion(); }) | color(Color::Green) | flex,
         })});
};

}  // namespace

ftxui::Component CommitComponent(int position, const std::string& commit, OSTreeTUI& ostreetui) {
    return ftxui::Make<CommitComponentImpl>(position, commit, ostreetui);
}

ftxui::Element commitRender(OSTreeTUI& ostreetui,
                            const std::unordered_map<std::string, ftxui::Color>& branchColorMap) {
    using namespace ftxui;

    int scrollOffset = ostreetui.GetScrollOffset();

    // check empty commit list
    if (ostreetui.GetVisibleCommitViewMap().empty() || ostreetui.GetVisibleBranches().empty()) {
        return color(Color::RedLight, text(" no commits to be shown ") | bold | center);
    }

    // stores the dedicated tree-column of each branch, -1 meaning not displayed yet
    std::unordered_map<std::string, int> usedBranches{};
    for (const auto& branchPair : ostreetui.GetVisibleBranches()) {
        if (branchPair.second) {
            usedBranches[branchPair.first] = -1;
        }
    }
    int nextAvailableSpace = static_cast<int>(usedBranches.size() - 1);

    // - RENDER -
    Elements treeElements{};

    ostreetui.GetColumnToBranchMap().clear();
    for (const auto& visibleCommitIndex : ostreetui.GetVisibleCommitViewMap()) {
        const cpplibostree::Commit commit =
            ostreetui.GetOstreeRepo().getCommitList().at(visibleCommitIndex);
        // branch head if it is first branch usage
        const std::string relevantBranch = commit.branch;
        if (usedBranches.at(relevantBranch) == -1) {
            ostreetui.GetColumnToBranchMap().push_back(relevantBranch);
            usedBranches.at(relevantBranch) = nextAvailableSpace--;
        }
        // commit
        if (scrollOffset++ >= 0) {
            treeElements.push_back(
                addTreeLine(RenderTree::TREE_LINE_NODE, commit, usedBranches, branchColorMap));
        }
        for (int i{0}; i < 3; i++) {
            if (scrollOffset++ >= 0) {
                treeElements.push_back(
                    addTreeLine(RenderTree::TREE_LINE_TREE, commit, usedBranches, branchColorMap));
            }
        }
    }
    std::reverse(ostreetui.GetColumnToBranchMap().begin(), ostreetui.GetColumnToBranchMap().end());

    return vbox(std::move(treeElements));
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

        if (treeLineType == RenderTree::TREE_LINE_TREE ||
            (treeLineType == RenderTree::TREE_LINE_IGNORE_BRANCH &&
             branch.first != relevantBranch)) {
            tree.at(branch.second) = (text(COMMIT_TREE) | color(branchColorMap.at(branch.first)));
        } else if (treeLineType == RenderTree::TREE_LINE_NODE) {
            if (branch.first == relevantBranch) {
                tree.at(branch.second) =
                    (text(COMMIT_NODE) | color(branchColorMap.at(branch.first)));
            } else {
                tree.at(branch.second) =
                    (text(COMMIT_TREE) | color(branchColorMap.at(branch.first)));
            }
        }
    }

    return hbox(std::move(tree));
}

}  // namespace CommitRender
