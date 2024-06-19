#include "OSTreeTUI.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>

#include <fcntl.h>

#include "commit.hpp"
#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "scroller.hpp"

#include "footer.hpp"
#include "manager.hpp"

#include "clip.h"

#include "../util/cpplibostree.hpp"

std::vector<std::string> OSTreeTUI::parseVisibleCommitMap(cpplibostree::OSTreeRepo& repo,
                            std::unordered_map<std::string, bool>& visibleBranches) {
    
	std::vector<std::string> visibleCommitViewMap{};
	// get filtered commits
	visibleCommitViewMap = {};
	for (const auto& commitPair : repo.getCommitList()) {
		// filter branches
		for (const auto& branch : commitPair.second.branches) {
			if (visibleBranches[branch]) {
				visibleCommitViewMap.push_back(commitPair.first);
			}
		}
	}
	// sort by date
	std::sort(visibleCommitViewMap.begin(), visibleCommitViewMap.end(), [&](const std::string& a, const std::string& b) {
		return repo.getCommitList().at(a).timestamp
			 > repo.getCommitList().at(b).timestamp;
	});

	return visibleCommitViewMap;
}

int OSTreeTUI::main(const std::string& repo, const std::vector<std::string>& startupBranches, bool showTooltips) {
	using namespace ftxui;

	// - STATES ---------- ----------
	
	// Model
	cpplibostree::OSTreeRepo ostreeRepo(repo);
	// View
	size_t selectedCommit{0};									// view-index
	std::unordered_map<std::string, bool>  visibleBranches{};	// map branch visibility to branch
	std::vector<std::string> visibleCommitViewMap{};			// map from view-index to commit-hash
	std::unordered_map<std::string, Color> branchColorMap{};	// map branch to color
	std::string notificationText = "";							// footer notification
	
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
			if (visibleBranches.find(branch) == visibleBranches.end()) {
        		return showHelp("ostree","no such branch: " + branch);
			}
			visibleBranches[branch] = true;
		}
	}

	// - UPDATES ---------- ----------

	auto refresh_repository = [&] {
		ostreeRepo.updateData();
		visibleCommitViewMap = parseVisibleCommitMap(ostreeRepo, visibleBranches);
		return true;
	};
	auto next_commit = [&] {
		if (selectedCommit + 1 >= visibleCommitViewMap.size()) {
			selectedCommit = visibleCommitViewMap.size() - 1;
			return false;
		}
		++selectedCommit;
		return true;
	};
	auto prev_commit = [&] {
		if (selectedCommit <= 0) {
			selectedCommit = 0;
			return false;
		}
		--selectedCommit;
		return true;
	};

	// - UI ELEMENTS ---------- ----------
	auto screen = ScreenInteractive::Fullscreen();
	
	std::vector<std::string> allBranches = ostreeRepo.getBranches();
 
	// INTERCHANGEABLE VIEW
	// info
	Component infoView = Renderer([&] {
		if (visibleCommitViewMap.size() <= 0) {
			return text(" no commit info available ") | color(Color::RedLight) | bold | center;
		}
		return CommitInfoManager::renderInfoView(ostreeRepo.getCommitList().at(visibleCommitViewMap.at(selectedCommit)));
    });

	// filter
	BranchBoxManager filterManager(ostreeRepo, visibleBranches);
	Component filterView = Renderer(filterManager.branchBoxes, [&] {
		return filterManager.branchBoxRender();
	});

	// promotion
	ContentPromotionManager promotionManager(showTooltips);
	promotionManager.setBranchRadiobox(Radiobox(&allBranches, &promotionManager.selectedBranch));
	promotionManager.setApplyButton(Button(" Apply ", [&] {
		ostreeRepo.promoteCommit(visibleCommitViewMap.at(selectedCommit),
								  ostreeRepo.getBranches().at(static_cast<size_t>(promotionManager.selectedBranch)),
								  {}, promotionManager.newSubject,
								  true);
		refresh_repository();
		notificationText = " Applied content promotion. ";
	}, ButtonOption::Simple()));
	Component promotionView = Renderer(promotionManager.composePromotionComponent(), [&] {
		if (visibleCommitViewMap.size() <= 0) {
			return text(" please select a commit to continue commit-promotion... ") | color(Color::RedLight) | bold | center;
		}
		return promotionManager.renderPromotionView(ostreeRepo, screen.dimy(),
			ostreeRepo.getCommitList().at(visibleCommitViewMap.at(selectedCommit)));
    });

	// interchangeable view (composed)
	Manager manager(infoView, filterView, promotionView);
  	Component managerRenderer = manager.managerRenderer;
	
	// COMMIT TREE
	Component logRenderer = Scroller(&selectedCommit, CommitRender::COMMIT_DETAIL_LEVEL, Renderer([&] {
		visibleCommitViewMap = parseVisibleCommitMap(ostreeRepo, visibleBranches);
		selectedCommit = std::min(selectedCommit, visibleCommitViewMap.size() - 1);
		return CommitRender::commitRender(ostreeRepo, visibleCommitViewMap, visibleBranches, branchColorMap, selectedCommit);
	}));

	// FOOTER
	Footer footer;
  	Component footerRenderer = Renderer([&] {
		return footer.footerRender();
	});

	// window specific shortcuts
	logRenderer = CatchEvent(logRenderer, [&](Event event) {
		// switch through commits
    	if (event == Event::ArrowUp || event == Event::Character('k') || (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
    	  	return prev_commit();
    	}
    	if (event == Event::ArrowDown || event == Event::Character('j') || (event.is_mouse() && event.mouse().button == Mouse::WheelDown)) {
    	  	return next_commit();
    	}
		return false;
	});

  	int logSize{45};
  	int footerSize{1};
  	Component container{managerRenderer};
  	container = ResizableSplitLeft(logRenderer, container, &logSize);
  	container = ResizableSplitBottom(footerRenderer, container, &footerSize);
	
	logRenderer->TakeFocus();

	// add shortcuts
	// TODO change to shift+ctrl+, or change any other way, as it currently
	// blocks the chosen letters from any TUI-internal text input
	Component mainContainer = CatchEvent(container | border, [&](const Event& event) {
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
    	if (event == Event::AltQ || event == Event::Escape) {
			screen.ExitLoopClosure()();
    	  	return true;
    	}
    	return false;
  	});

	bool runSubThreads{true};
	std::thread footerNotificationUpdater([&] {
		while (runSubThreads) {
			using namespace std::chrono_literals;
			// notification is set
			if (notificationText != "") {
				footer.content = notificationText;
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


int OSTreeTUI::showHelp(const std::string& caller, const std::string& errorMessage) {
	using namespace ftxui;

	// define command line options
	std::vector<std::vector<std::string>> command_options = {
		// option, arguments, meaning
		{"-h, --help", "", "Show help options. The REPOSITORY_PATH can be omitted"},
		{"-r, --refs", "REF [REF...]", "Specify a list of visible refs at startup if not specified, show all refs"},
		{"-n, --no-tooltips", "", "Hide Tooltips in promotion view."}
	};

	Elements options   = {text("Options:")};
	Elements arguments = {text("Arguments:")};
	Elements meanings  = {text("Meaning:")};
	for (const auto& command : command_options) {
		options.push_back(text(command.at(0) + "  ") | color(Color::GrayLight));
		arguments.push_back(text(command.at(1) + "  ") | color(Color::GrayLight));
		meanings.push_back(text(command.at(2) + "  "));
	}

	auto helpPage = vbox({
			errorMessage.size() == 0 ? filler() : (text(errorMessage) | bold | color(Color::Red) | flex),
			hbox({
	            text("Usage: "),
				text(caller) | color(Color::GrayLight),
	            text(" REPOSITORY_PATH") | color(Color::Yellow),
				text(" [OPTION...]") | color(Color::Yellow)
	        }),
			text(""),
	        hbox({
				vbox(options),
				vbox(arguments),
				vbox(meanings),
	        }),
			text(""),
			hbox({
				text("Report bugs at "),
				text("https://github.com/AP-Sensing/ostree-tui") | hyperlink("https://github.com/AP-Sensing/ostree-tui")
			}),
			text("")
	    });

	auto screen = Screen::Create(Dimension::Fit(helpPage));
	Render(screen, helpPage);
	screen.Print();
	std::cout << "\n";

	return errorMessage.size() == 0;
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
