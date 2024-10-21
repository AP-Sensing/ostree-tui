#include "OSTreeTUI.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <fcntl.h>

#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for Element, operator|, text, center, border

#include "commit.hpp"
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
		if (visibleBranches[commitPair.second.branch]) {
			visibleCommitViewMap.push_back(commitPair.first);
		}
	}
	// sort by date
	std::sort(visibleCommitViewMap.begin(), visibleCommitViewMap.end(), [&](const std::string& a, const std::string& b) {
		return repo.getCommitList().at(a).timestamp
			 > repo.getCommitList().at(b).timestamp;
	});

	return visibleCommitViewMap;
}

int OSTreeTUI::main(const std::string& repo, const std::vector<std::string>& startupBranches) {
	using namespace ftxui;

	// - STATES ---------- ----------
	
	// Model
	cpplibostree::OSTreeRepo ostreeRepo(repo);
	// View
	size_t selectedCommit{0};									// view-index
	std::unordered_map<std::string, bool>  visibleBranches{};	// map branch visibility to branch
	std::vector<std::string> columnToBranchMap{};				// map column in commit-tree to branch (may be merged into one data-structure with visibleBranches)
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
        		return showHelp("ostree-tui","no such branch in repository " + repo + ": " + branch);
			}
			visibleBranches[branch] = true;
		}
	}

	// - UI ELEMENTS ---------- ----------
	auto screen = ScreenInteractive::Fullscreen();
	
	std::vector<std::string> allBranches = ostreeRepo.getBranches();
 
	visibleCommitViewMap = parseVisibleCommitMap(ostreeRepo, visibleBranches); // TODO This update shouldn't be made here...
	
	// COMMIT TREE
/* The commit-tree is currentrly under a heavy rebuild, see implementation To-Dos below.
 * For a general list of To-Dos refer to https://github.com/AP-Sensing/ostree-tui/pull/21
 *
 * TODO extend with keyboard functionality:
 *      normal scrolling through commits (should also highlight selected commit)
 *      if 'p' is pressed: start promotion
 *      if 'd' is pressed: open deletion window
 * TODO add commit deletion
 *      add deletion button & ask for confirmation (also add keyboard functionality)
 */
	// commit promotion state
	// TODO especially needed for keyboard shortcuts
	//      store shared information about which commit is in which state
	//      each commit can then display itself the way it should
	//      * is promotion action active?
	//      *   keyboard or mouse?
	//      *   which commit?
	//      * is deletion action active?
	//      *   keyboard or mouse?
	//      *   which commit?
	bool inPromotionSelection{false};
	bool refresh{false};
	std::string promotionHash{""};
	std::string promotionBranch{""};
	// parse all commits
	Components commitComponents;
	Component commitList;
	Component tree;
	
	int scrollOffset{0};
	
	auto refresh_commitComponents = [&] {
		commitComponents.clear();
		int i{0};
		visibleCommitViewMap = parseVisibleCommitMap(ostreeRepo, visibleBranches);
		for (auto& hash : visibleCommitViewMap) {
			commitComponents.push_back(
				CommitRender::CommitComponent(i, scrollOffset, inPromotionSelection, promotionHash, promotionBranch, visibleBranches, columnToBranchMap, hash, ostreeRepo, refresh)
			);
			i++;
		}
		//
		commitList = commitComponents.size() == 0
			? Renderer([&] { return text(" no commits to be shown ") | color(Color::Red); })
			: Container::Stacked(commitComponents);
		//
	};
	refresh_commitComponents();

	tree = Renderer([&] {
			refresh_commitComponents();
			selectedCommit = std::min(selectedCommit, visibleCommitViewMap.size() - 1);
			// TODO check for promotion & pass information if needed
			if (inPromotionSelection && promotionBranch.size() != 0) {
				std::unordered_map<std::string, Color> promotionBranchColorMap{};
				for (auto& [str,col] : branchColorMap) {
					if (str == promotionBranch) {
						promotionBranchColorMap.insert({str,col});
					} else {
						promotionBranchColorMap.insert({str,Color::GrayDark});
					}
				}
				return CommitRender::commitRender(ostreeRepo, visibleCommitViewMap, visibleBranches, columnToBranchMap, promotionBranchColorMap, scrollOffset, selectedCommit);	
			}
			return CommitRender::commitRender(ostreeRepo, visibleCommitViewMap, visibleBranches, columnToBranchMap, branchColorMap, scrollOffset, selectedCommit);
		});

	Component commitListComponent = Container::Horizontal({
		tree,
		commitList
	});

	/// refresh all graphical components in the commit-tree
	auto refresh_commitListComoponent = [&] {
		commitListComponent->DetachAllChildren();
		refresh_commitComponents();
		Component tmp = Container::Horizontal({
			tree,
			commitList
		});
		commitListComponent->Add(tmp);
	};
	/// refresh ostree-repository and graphical components
	auto refresh_repository = [&] {
		ostreeRepo.updateData();
		refresh_commitListComoponent();
		return true;
	};

	// window specific shortcuts
	commitListComponent = CatchEvent(commitListComponent, [&](Event event) {
		// scroll
    	if (event.is_mouse() && event.mouse().button == Mouse::WheelUp) {
    	  	if (scrollOffset < 0) {
				++scrollOffset;
			}
			selectedCommit = -scrollOffset / 4;
			return true;
    	}
    	if (event.is_mouse() && event.mouse().button == Mouse::WheelDown) {
    	  	--scrollOffset;
			selectedCommit = -scrollOffset / 4;
			return true;
    	}
		// switch through commits
    	if (event == Event::ArrowUp || event == Event::Character('k')) {
			scrollOffset = std::min(0, scrollOffset + 4);
			selectedCommit = -scrollOffset / 4;
			return true;
    	}
    	if (event == Event::ArrowDown || event == Event::Character('j')) {
    	  	scrollOffset -= 4;
			selectedCommit = -scrollOffset / 4;
			return true;
    	}
		return false;
	});

	// INTERCHANGEABLE VIEW
	// info
	Component infoView = Renderer([&] {
		visibleCommitViewMap = parseVisibleCommitMap(ostreeRepo, visibleBranches); // TODO This update shouldn't be made here...
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
	filterView = CatchEvent(filterView, [&](Event event) {
    	if (event.is_mouse() && event.mouse().button == Mouse::Button::Left) {
    	  	refresh_commitListComoponent();
    	}
		return false;
	});

	// interchangeable view (composed)
	Manager manager(infoView, filterView);
  	Component managerRenderer = manager.managerRenderer;

	// FOOTER
	Footer footer;
  	Component footerRenderer = Renderer([&] {
		return footer.footerRender();
	});

	// build together all components
  	int logSize{45};
  	int footerSize{1};
  	Component container{managerRenderer};
  	container = ResizableSplitLeft(commitListComponent, container, &logSize);
  	container = ResizableSplitBottom(footerRenderer, container, &footerSize);
	
	commitListComponent->TakeFocus();

	// add application shortcuts
	Component mainContainer = CatchEvent(container | border, [&](const Event& event) {
		//if (event == Event::Character('p')) {
		//	inPromotionSelection = true;
		//	promotionHash = visibleCommitViewMap.at(selectedCommit);
		//	promotionBranch = columnToBranchMap.at(0);
		//}
		// copy commit id
    	if (event == Event::AltC) {
			std::string hash = visibleCommitViewMap.at(selectedCommit);
			clip::set_text(hash);
			notificationText = " Copied Hash " + hash + " ";
    	  	return true;
    	}
		// refresh repository
		if (event == Event::AltR || refresh) {
			refresh_repository();
			notificationText = " Refreshed Repository Data ";
			refresh = false;
			return true;
		}
		// exit
    	if (event == Event::AltQ || event == Event::Escape) {
			screen.ExitLoopClosure()();
    	  	return true;
    	}
    	return false;
  	});

	// footer notification update loader
	// Probably not the best solution, having an active wait and should maybe
	// only be started, once a notification is set...
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
	std::vector<std::vector<std::string>> command_options{
		// option, arguments, meaning
		{"-h, --help", "", "Show help options. The REPOSITORY_PATH can be omitted"},
		{"-r, --refs", "REF [REF...]", "Specify a list of visible refs at startup if not specified, show all refs"},
	};

	Elements options   {text("Options:")};
	Elements arguments {text("Arguments:")};
	Elements meanings  {text("Meaning:")};
	for (const auto& command : command_options) {
		options.push_back(text(command.at(0) + "  ") | color(Color::GrayLight));
		arguments.push_back(text(command.at(1) + "  ") | color(Color::GrayLight));
		meanings.push_back(text(command.at(2) + "  "));
	}

	auto helpPage = vbox({
			errorMessage.empty() ? filler() : (text(errorMessage) | bold | color(Color::Red) | flex),
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
				text("Github.com/AP-Sensing/ostree-tui") | hyperlink("https://github.com/AP-Sensing/ostree-tui")
			}),
			text("")
	    });

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
