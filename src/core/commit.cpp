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

#include "ostreetui.hpp"

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
    explicit CommitComponentImpl(size_t position, std::string commit, OSTreeTUI& ostreetui)
        : defaultX(1),
          defaultY(static_cast<int>(position) * COMMIT_WINDOW_HEIGHT),
          commitPosition(position),
          hash(std::move(commit)),
          ostreetui(ostreetui),
          commit(ostreetui.GetOstreeRepo().GetCommitList().at(hash)),
          newVersion(this->commit.version) {
        inner = Renderer([&] {
            return vbox({
                text(ostreetui.GetOstreeRepo().GetCommitList().at(hash).subject),
                text(
                    std::format("{:%Y-%m-%d %T %Ez}",
                                std::chrono::time_point_cast<std::chrono::seconds>(
                                    ostreetui.GetOstreeRepo().GetCommitList().at(hash).timestamp))),
            });
        });
        simpleCommit = inner;
        Add(inner);

        title = hash.substr(0, 8);
        top = defaultY;
        left = defaultX;
        width = COMMIT_WINDOW_WIDTH;
        height = COMMIT_WINDOW_HEIGHT;
    }

   private:
    void resetWindow(bool positionReset = true) {
        if (positionReset) {
            left() = defaultX;
            top() = defaultY;
        }
        width() = defaultWidth;
        height() = defaultHeight;
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

    void startDeletionWindow(bool isMostRecentCommit) {
        left() = std::max(left(), -2);
        width() = DELETION_WINDOW_WIDTH;
        height() = DELETION_WINDOW_HEIGHT;
        // change inner to deletion layout
        DetachAllChildren();
        if (isMostRecentCommit) {
            Add(deletionViewHead);
        } else {
            Add(deletionViewBody);
        }
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
        ostreetui.RemoveCommit(ostreetui.GetOstreeRepo().GetCommitList().at(hash));
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
            startDeletionWindow(ostreetui.GetOstreeRepo().IsMostRecentCommitOnBranch(hash));
        }

        ftxui::Element element = ComponentBase::Render();

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
        element |= reflect(boxWindow_);
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

        mouseHover_ = boxWindow_.Contain(event.mouse().x, event.mouse().y);

        if (mouseHover_ && event.mouse().button == Mouse::Left) {
            ostreetui.SetSelectedCommit(commitPosition);
        }

        if (capturedMouse_) {
            if (event.mouse().motion == Mouse::Released) {
                // reset mouse
                capturedMouse_ = nullptr;
                // drop commit
                if (event.mouse().y > ostreetui.GetScreen().dimy() - 8) {
                    ostreetui.SetViewMode(ViewMode::COMMIT_DROP, hash);
                    ostreetui.SetModeBranch(
                        ostreetui.GetOstreeRepo().GetCommitList().at(hash).branch);
                    top() = defaultY;
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
                left() = event.mouse().x - dragStartX - box_.x_min;
                top() = event.mouse().y - dragStartY - box_.y_min;
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

        if (!mouseHover_) {
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

        capturedMouse_ = CaptureMouse(event);
        if (!capturedMouse_) {
            return true;
        }

        dragStartX = event.mouse().x - left() - box_.x_min;
        dragStartY = event.mouse().y - top() - box_.y_min;

        const bool dragOld = drag_;
        drag_ = true;
        if (!dragOld && drag_) {  // if we start dragging
            defaultX = left();
            defaultY = top();
        }
        return true;
    }

    // window specific members
    Box box_;
    Box boxWindow_;

    bool mouseHover_ = false;
    bool drag_ = false;
    CapturedMouse capturedMouse_;
    int dragStartX = 0;
    int dragStartY = 0;

    int defaultX;
    int defaultY;
    int defaultWidth = COMMIT_WINDOW_WIDTH;
    int defaultHeight = COMMIT_WINDOW_HEIGHT;

    // ostree-tui specific members
    size_t commitPosition;
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
    // deletion view, commit is at head of branch
    Component deletionViewHead = Container::Vertical(
        {Renderer([&] {
             return vbox({text(" Remove Commit...") | bold, text(""),
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
    // deletion view, if commit is not the most recent on its branch
    Component deletionViewBody = Container::Vertical(
        {Renderer([&] {
             std::string parent = ostreetui.GetOstreeRepo().GetCommitList().at(hash).parent;
             return vbox({text(" Remove Commit (and preceding)...") | bold, text(""),
                          text(" ☐ " + ostreetui.GetModeBranch()) | dim, text(" │") | dim,
                          hbox({
                              text(" ✖ ") | color(Color::Red),
                              text(hash.substr(0, 8)) | bold | color(Color::Red),
                          }),
                          parent == "(no parent)"
                              ? text("")
                              : vbox({
                                    text(" ✖ " + parent.substr(0, 8)) | color(Color::Red),
                                    text(" ✖ ...") | color(Color::Red),
                                })});
         }),
         Container::Horizontal({
             Button(" Cancel ", [&] { cancelSpecialWindow(); }) | color(Color::Red) | flex,
             Button(" Remove ", [&] { executeDeletion(); }) | color(Color::Green) | flex,
         })});
};

}  // namespace

ftxui::Component CommitComponent(size_t position, const std::string& commit, OSTreeTUI& ostreetui) {
    return ftxui::Make<CommitComponentImpl>(position, commit, ostreetui);
}

ftxui::Element CommitRender(OSTreeTUI& ostreetui,
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
            ostreetui.GetOstreeRepo().GetCommitList().at(visibleCommitIndex);
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
