#include "OSTreeTUI.hpp"

#include <fcntl.h>
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include <ftxui/component/event.hpp>  // for Event, Event::ArrowDown, Event::ArrowUp, Event::End, Event::Home, Event::PageDown, Event::PageUp
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"                  // for Element, operator|, text, center, border

#include "clip.h"

#include "../util/cpplibostree.hpp"

OSTreeTUI::OSTreeTUI(const std::string& repo, const std::vector<std::string> startupBranches)
    : ostreeRepo(repo), selectedCommit(0), screen(ftxui::ScreenInteractive::Fullscreen()) {
    using namespace ftxui;

    // set all branches as visible and define a branch color
    for (const auto& branch : ostreeRepo.getBranches()) {
        // if startupBranches are defined, set all as non-visible
        visibleBranches[branch] = startupBranches.size() == 0 ? true : false;
        std::hash<std::string> nameHash{};
        branchColorMap[branch] = Color::Palette256((nameHash(branch) + 10) % 256);
    }
    // if startupBranches are defined, only set those visible
    if (startupBranches.size() != 0) {
        for (const auto& branch : startupBranches) {
            visibleBranches[branch] = true;
        }
    }

    // - UI ELEMENTS ---------- ----------

    // COMMIT TREE
    refresh_commitComponents();

    tree = Renderer([&] {
        refresh_commitComponents();
        selectedCommit = std::min(selectedCommit, visibleCommitViewMap.size() - 1);
        // TODO check for promotion & pass information if needed
        if (inPromotionSelection && promotionBranch.size() != 0) {
            std::unordered_map<std::string, Color> promotionBranchColorMap{};
            for (auto& [str, col] : branchColorMap) {
                if (str == promotionBranch) {
                    promotionBranchColorMap.insert({str, col});
                } else {
                    promotionBranchColorMap.insert({str, Color::GrayDark});
                }
            }
            return CommitRender::commitRender(*this, promotionBranchColorMap);
        }
        return CommitRender::commitRender(*this, branchColorMap);
    });

    commitListComponent = Container::Horizontal({tree, commitList});

    // window specific shortcuts
    commitListComponent = CatchEvent(commitListComponent, [&](Event event) {
        // switch through commits
        if ((!inPromotionSelection && event == Event::ArrowUp) ||
            (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
            selectedCommit = std::max(0, static_cast<int>(selectedCommit) - 1);
            adjustScrollToSelectedCommit();
            return true;
        }
        if ((!inPromotionSelection && event == Event::ArrowDown) ||
            (event.is_mouse() && event.mouse().button == Mouse::WheelDown)) {
            selectedCommit = std::min(selectedCommit + 1, getVisibleCommitViewMap().size() - 1);
            adjustScrollToSelectedCommit();
            return true;
        }
        return false;
    });

    // INTERCHANGEABLE VIEW
    // info
    infoView = Renderer([&] {
        visibleCommitViewMap = parseVisibleCommitMap(ostreeRepo, visibleBranches);
        if (visibleCommitViewMap.size() <= 0) {
            return text(" no commit info available ") | color(Color::RedLight) | bold | center;
        }
        return CommitInfoManager::renderInfoView(
            ostreeRepo.getCommitList().at(visibleCommitViewMap.at(selectedCommit)));
    });

    // filter
    filterManager =
        std::unique_ptr<BranchBoxManager>(new BranchBoxManager(ostreeRepo, visibleBranches));
    filterView =
        Renderer(filterManager->branchBoxes, [&] { return filterManager->branchBoxRender(); });
    filterView = CatchEvent(filterView, [&](Event event) {
        if (event.is_mouse() && event.mouse().button == Mouse::Button::Left) {
            refresh_commitListComoponent();
        }
        return false;
    });

    // interchangeable view (composed)
    manager = std::unique_ptr<Manager>(new Manager(*this, infoView, filterView));
    managerRenderer = manager->getManagerRenderer();

    // FOOTER
    footerRenderer = Renderer([&] { return footer.footerRender(); });

    // BUILD MAIN CONTAINER
    container = Component(managerRenderer);
    container = ResizableSplitLeft(commitListComponent, container, &logSize);
    container = ResizableSplitBottom(footerRenderer, container, &footerSize);

    commitListComponent->TakeFocus();

    // add application shortcuts
    mainContainer = CatchEvent(container | border, [&](const Event& event) {
        if (event == Event::AltP) {
            setPromotionMode(true, visibleCommitViewMap.at(selectedCommit));
        }
        // copy commit id
        if (event == Event::AltC) {
            std::string hash = visibleCommitViewMap.at(selectedCommit);
            clip::set_text(hash);
            notificationText = " Copied Hash " + hash + " ";
            return true;
        }
        // refresh repository
        if (event == Event::AltR) {
            refresh_repository();
            notificationText = " Refreshed Repository Data ";
            return true;
        }
        // exit
        if (event == Event::AltQ) {
            screen.ExitLoopClosure()();
            return true;
        }
        // make commit list focussable
        if (event == Event::ArrowLeft && managerRenderer->Focused() &&
            manager->getTabIndex() == 0) {
            commitListComponent->TakeFocus();
            return true;
        }
        return false;
    });
}

int OSTreeTUI::run() {
    using namespace ftxui;
    // footer notification update loader
    // Probably not the best solution, having an active wait and should maybe
    // only be started, once a notification is set...
    bool runSubThreads{true};
    std::thread footerNotificationUpdater([&] {
        while (runSubThreads) {
            using namespace std::chrono_literals;
            // notification is set
            if (notificationText != "") {
                footer.setContent(notificationText);
                screen.Post(Event::Custom);
                std::this_thread::sleep_for(2s);
                // clear notification
                notificationText = "";
                footer.resetContent();
                screen.Post(Event::Custom);
            }
            std::this_thread::sleep_for(0.2s);
        }
    });

    screen.Loop(mainContainer);
    runSubThreads = false;
    footerNotificationUpdater.join();

    return EXIT_SUCCESS;
}

void OSTreeTUI::refresh_commitComponents() {
    using namespace ftxui;

    commitComponents.clear();
    int i{0};
    visibleCommitViewMap = parseVisibleCommitMap(ostreeRepo, visibleBranches);
    for (auto& hash : visibleCommitViewMap) {
        commitComponents.push_back(CommitRender::CommitComponent(i, hash, *this));
        i++;
    }

    commitList =
        commitComponents.size() == 0
            ? Renderer([&] { return text(" no commits to be shown ") | color(Color::Red); })
            : Container::Stacked(commitComponents);
}

void OSTreeTUI::refresh_commitListComoponent() {
    using namespace ftxui;

    commitListComponent->DetachAllChildren();
    refresh_commitComponents();
    Component tmp = Container::Horizontal({tree, commitList});
    commitListComponent->Add(tmp);
}

bool OSTreeTUI::refresh_repository() {
    ostreeRepo.updateData();
    refresh_commitListComoponent();
    return true;
}

bool OSTreeTUI::setPromotionMode(bool active, std::string hash, bool setPromotionBranch) {
    // deactivate promotion mode
    if (!active) {
        inPromotionSelection = false;
        promotionBranch = "";
        promotionHash = "";
        return true;
    }
    // set promotion mode
    if (!inPromotionSelection || hash != promotionHash) {
        inPromotionSelection = true;
        if (setPromotionBranch) {
            promotionBranch = promotionBranch.empty() ? columnToBranchMap.at(0) : promotionBranch;
        }
        promotionHash = hash;
        return true;
    }
    // nothing to update
    return false;
}

bool OSTreeTUI::promoteCommit(std::string hash,
                              std::string branch,
                              std::vector<std::string> metadataStrings,
                              std::string newSubject,
                              bool keepMetadata) {
    bool success =
        ostreeRepo.promoteCommit(hash, branch, metadataStrings, newSubject, keepMetadata);
    setPromotionMode(false);
    // reload repository
    if (success) {
        scrollOffset = 0;
        selectedCommit = 0;
        screen.PostEvent(ftxui::Event::AltR);
    }
    return success;
}

std::vector<std::string> OSTreeTUI::parseVisibleCommitMap(
    cpplibostree::OSTreeRepo& repo,
    std::unordered_map<std::string, bool>& visibleBranches) {
    std::vector<std::string> visibleCommitViewMap{};
    // get filtered commits
    visibleCommitViewMap = {};
    for (const auto& commitPair : repo.getCommitList()) {
        if (visibleBranches[commitPair.second.branch]) {
            visibleCommitViewMap.push_back(commitPair.first);
        }
    }
    // sort by date
    std::sort(visibleCommitViewMap.begin(), visibleCommitViewMap.end(),
              [&](const std::string& a, const std::string& b) {
                  return repo.getCommitList().at(a).timestamp >
                         repo.getCommitList().at(b).timestamp;
              });

    return visibleCommitViewMap;
}

void OSTreeTUI::adjustScrollToSelectedCommit() {
    // try to scroll it to the middle
    int windowHeight = screen.dimy() - 4;
    int scollOffsetToFitCommitToTop = -selectedCommit * CommitRender::COMMIT_WINDOW_HEIGHT;
    int newScroll =
        scollOffsetToFitCommitToTop + windowHeight / 2 - CommitRender::COMMIT_WINDOW_HEIGHT;
    // adjust if on edges
    int min = 0;
    int max = -windowHeight - static_cast<int>(visibleCommitViewMap.size() - 1) *
                                  CommitRender::COMMIT_WINDOW_HEIGHT;
    scrollOffset = std::max(newScroll, max);
    scrollOffset = std::min(min, newScroll);
}

// SETTER & non-const GETTER
void OSTreeTUI::setPromotionBranch(std::string promotionBranch) {
    this->promotionBranch = promotionBranch;
}

void OSTreeTUI::setSelectedCommit(size_t selectedCommit) {
    this->selectedCommit = selectedCommit;
    adjustScrollToSelectedCommit();
}

std::vector<std::string>& OSTreeTUI::getColumnToBranchMap() {
    return columnToBranchMap;
}

ftxui::ScreenInteractive& OSTreeTUI::getScreen() {
    return screen;
}

// GETTER
const cpplibostree::OSTreeRepo& OSTreeTUI::getOstreeRepo() const {
    return ostreeRepo;
}

const size_t& OSTreeTUI::getSelectedCommit() const {
    return selectedCommit;
}

const std::string& OSTreeTUI::getPromotionBranch() const {
    return promotionBranch;
}

const std::unordered_map<std::string, bool>& OSTreeTUI::getVisibleBranches() const {
    return visibleBranches;
}

const std::vector<std::string>& OSTreeTUI::getColumnToBranchMap() const {
    return columnToBranchMap;
}

const std::vector<std::string>& OSTreeTUI::getVisibleCommitViewMap() const {
    return visibleCommitViewMap;
}

const std::unordered_map<std::string, ftxui::Color>& OSTreeTUI::getBranchColorMap() const {
    return branchColorMap;
}

const int& OSTreeTUI::getScrollOffset() const {
    return scrollOffset;
}

const bool& OSTreeTUI::getInPromotionSelection() const {
    return inPromotionSelection;
}

const std::string& OSTreeTUI::getPromotionHash() const {
    return promotionHash;
}

// STATIC
int OSTreeTUI::showHelp(const std::string& caller, const std::string& errorMessage) {
    using namespace ftxui;

    // define command line options
    std::vector<std::vector<std::string>> command_options{
        // option, arguments, meaning
        {"-h, --help", "", "Show help options. The REPOSITORY_PATH can be omitted"},
        {"-r, --refs", "REF [REF...]",
         "Specify a list of visible refs at startup if not specified, show all refs"},
    };

    Elements options{text("Options:")};
    Elements arguments{text("Arguments:")};
    Elements meanings{text("Meaning:")};
    for (const auto& command : command_options) {
        options.push_back(text(command.at(0) + "  ") | color(Color::GrayLight));
        arguments.push_back(text(command.at(1) + "  ") | color(Color::GrayLight));
        meanings.push_back(text(command.at(2) + "  "));
    }

    auto helpPage = vbox(
        {errorMessage.empty() ? filler() : (text(errorMessage) | bold | color(Color::Red) | flex),
         hbox({text("Usage: "), text(caller) | color(Color::GrayLight),
               text(" REPOSITORY_PATH") | color(Color::Yellow),
               text(" [OPTION...]") | color(Color::Yellow)}),
         text(""),
         hbox({
             vbox(options),
             vbox(arguments),
             vbox(meanings),
         }),
         text(""),
         hbox({text("Report bugs at "), text("Github.com/AP-Sensing/ostree-tui") |
                                            hyperlink("https://github.com/AP-Sensing/ostree-tui")}),
         text("")});

    auto screen = Screen::Create(Dimension::Fit(helpPage));
    Render(screen, helpPage);
    screen.Print();
    std::cout << "\n";

    return errorMessage.empty();
}

int OSTreeTUI::showVersion() {
    using namespace ftxui;

    auto versionText = text("ostree-tui 0.2.1");

    auto screen = Screen::Create(Dimension::Fit(versionText));
    Render(screen, versionText);
    screen.Print();
    std::cout << "\n";

    return 0;
}
