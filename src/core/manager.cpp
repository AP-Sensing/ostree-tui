#include "manager.hpp"

#include <assert.h>
#include <cstdio>
#include <string>

#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/event.hpp"  // for Event
#include "ftxui/dom/elements.hpp"     // for Element, operator|, text, center, border

#include "../util/cpplibostree.hpp"

#include "ostreetui.hpp"

// Manager

Manager::Manager(OSTreeTUI& ostreetui,
                 const ftxui::Component& infoView,
                 const ftxui::Component& filterView)
    : ostreetui(ostreetui) {
    using namespace ftxui;

    tabSelection = Menu(&tabEntries, &tabIndex, MenuOption::HorizontalAnimated());

    tabContent = Container::Tab({infoView, filterView}, &tabIndex);

    managerRenderer = Container::Vertical(
        {tabSelection, tabContent,
         Renderer([] { return vbox({filler()}) | flex; }),  // push elements apart
         Renderer([&] {
             Elements branches;
             for (size_t i{ostreetui.GetColumnToBranchMap().size()}; i > 0; i--) {
                 std::string branch = ostreetui.GetColumnToBranchMap().at(i - 1);
                 std::string line = "――☐――― " + branch;
                 branches.push_back(text(line) | color(ostreetui.GetBranchColorMap().at(branch)));
             }
             return vbox(branches);
         })});
}

ftxui::Component Manager::GetManagerRenderer() {
    return managerRenderer;
}

int Manager::GetTabIndex() const {
    return tabIndex;
}

// BranchBoxManager

BranchBoxManager::BranchBoxManager(OSTreeTUI& ostreetui,
                                   cpplibostree::OSTreeRepo& repo,
                                   std::unordered_map<std::string, bool>& visibleBranches) {
    using namespace ftxui;

    CheckboxOption cboption = CheckboxOption::Simple();
    cboption.on_change = [&] { ostreetui.RefreshCommitListComponent(); };
    // CheckboxOption cboption = {.on_change = [&] { ostreetui.RefreshCommitListComponent(); }};

    // branch visibility
    for (const auto& branch : repo.GetBranches()) {
        branchBoxes->Add(Checkbox(branch, &(visibleBranches.at(branch)), cboption));
    }
}

ftxui::Element BranchBoxManager::BranchBoxRender() {
    using namespace ftxui;

    // branch filter
    Elements bfbElements = {
        text(L"branches:") | bold,
        filler(),
        branchBoxes->Render() | vscroll_indicator | frame | size(HEIGHT, LESS_THAN, 10),
    };
    return vbox(bfbElements);
}

// CommitInfoManager

ftxui::Element CommitInfoManager::RenderInfoView(const cpplibostree::Commit& displayCommit) {
    using namespace ftxui;

    // selected commit info
    Elements signatures;
    for (const auto& signature : displayCommit.signatures) {
        std::string ts =
            std::format("{:%Y-%m-%d %T %Ez}",
                        std::chrono::time_point_cast<std::chrono::seconds>(signature.timestamp));
        signatures.push_back(
            vbox({hbox({text("‣ "), text(signature.pubkeyAlgorithm) | bold, text(" signature")}),
                  text("  with key ID " + signature.fingerprint), text("  made " + ts)}));
    }
    return vbox(
        {text(" Subject:") | color(Color::Green),
         paragraph(displayCommit.subject) | color(Color::White), filler(),
         text(" Hash: ") | color(Color::Green), text(displayCommit.hash), filler(),
         text(" Date: ") | color(Color::Green),
         text(std::format("{:%Y-%m-%d %T %Ez}", std::chrono::time_point_cast<std::chrono::seconds>(
                                                    displayCommit.timestamp))),
         filler(),
         // TODO insert version, only if exists
         displayCommit.version.empty() ? filler() : text(" Version: ") | color(Color::Green),
         displayCommit.version.empty() ? filler() : text(displayCommit.version),
         text(" Parent: ") | color(Color::Green), text(displayCommit.parent), filler(),
         text(" Checksum: ") | color(Color::Green), text(displayCommit.contentChecksum), filler(),
         displayCommit.signatures.size() > 0 ? text(" Signatures: ") | color(Color::Green)
                                             : text(""),
         vbox(signatures), filler()});
}
