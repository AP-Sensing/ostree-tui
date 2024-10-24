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
#include "commitComponent.hpp"

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
        		return showHelp("ostree-tui","no such branch in repository " + repo + ": " + branch);
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
	visibleCommitViewMap = parseVisibleCommitMap(ostreeRepo, visibleBranches); // TODO This update shouldn't be made here...
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

	// FOOTER
	Footer footer;
  	Component footerRenderer = Renderer([&] {
		return footer.footerRender();
	});

	// COMMIT TREE
/* TODO - The commit-tree is currentrly under a heavy rebuild, see implementation To-Dos below.
 * 	      For a general list of To-Dos refer to https://github.com/AP-Sensing/ostree-tui/pull/21
 * 
 * > Component commitTree should be a Stacked(...) to allow for snappy windows to be arranged with
 *   a drag & drop funcitonality.
 * > Snappy Windows should be an abstracted element, similar to windows, but snapping back to their
 *   place after being dropped.
 * > While dragging a window, the branches should be greyed out, only coloring the branch that the
 *   mouse dragging a commit hovers over
 *   * switch to drag-mode when commit is taken
 *   * let mouse event through to branches
 *   * if commit is dropped on branch, start promotion dialogue
 * > There should obviously still be a commit tree on the left side. How the exact implementation
 *   would go is still to be figured out. It this would be one, or many different elements mainly
 *   depends on how the overlap detection with branches would work, when dragging commits.
 */
	// commit promotion state
	// shared with all relevant components to monitor and react to
	// TODO extend with keyboard functionality
	bool inPromotionSelection{false};
	std::string promotionHash{""};
	std::string promotionBranch{""};
	// parse all commits
	Components commitComponents;
	int scroll_offset{0};
	int i{0};
	auto refresh_commitComponents = [&] {
		for (auto& hash : visibleCommitViewMap) {
			cpplibostree::Commit& commit = ostreeRepo.getCommitList().at(hash);
			commitComponents.push_back(
				// TODO make the commits scrollable (maybe common y offset variable)
				CommitComponent(scroll_offset, inPromotionSelection, promotionHash, promotionBranch, commit, {
					.inner = Renderer([commit] {
    							return vbox({
    						    	text(commit.subject),
							 		text(std::format("{:%Y-%m-%d %T %Ez}", std::chrono::time_point_cast<std::chrono::seconds>(commit.timestamp))),
    							});
    						}),
    				.title = hash.substr(0, 8),
    				.left = 1,
    				.top = i * 4,
					.width = 30,
    	  			.height = 4,
					.resize_left = false,
					.resize_right = false,
					.resize_top = false,
					.resize_down = false,
				})
			);
			i++;
		}
	};
	refresh_commitComponents();

	Component commitTree = Container::Horizontal({
		// commit tree
		// TODO check for mouse overlap, while commit is dragged
		// maybe could also be checked by commit component, if a list of branches with x coordinates is passed
		// commit could then handle everything, including the commit-promotion call back to the window
		Renderer([&] {
			visibleCommitViewMap = parseVisibleCommitMap(ostreeRepo, visibleBranches);
			refresh_commitComponents();
			selectedCommit = std::min(selectedCommit, visibleCommitViewMap.size() - 1);
			// TODO check for promotion & pass information if needed
			if (inPromotionSelection && promotionBranch.size() != 0) {
				std::unordered_map<std::string, Color> promotionBranchColorMap{};
				for (auto& [str,col] : branchColorMap) {
					if (str == promotionBranch) {
						promotionBranchColorMap.insert({str,col});
					} else {
						promotionBranchColorMap.insert({str,Color::Black});
					}
				}
				return CommitRender::commitRender(ostreeRepo, visibleCommitViewMap, visibleBranches, promotionBranchColorMap, selectedCommit);	
			}
			return CommitRender::commitRender(ostreeRepo, visibleCommitViewMap, visibleBranches, branchColorMap, selectedCommit);
		}),
		// commit list
		commitComponents.size() == 0 ? Renderer([&] { return text(" no commits to be shown ") | color(Color::Red); }) : Container::Stacked(commitComponents)
	});

	// window specific shortcuts
	commitTree = CatchEvent(commitTree, [&](Event event) {
		// switch through commits
    	if (event == Event::ArrowUp || event == Event::Character('k') || (event.is_mouse() && event.mouse().button == Mouse::WheelUp)) {
    	  	return prev_commit();
    	}
    	if (event == Event::ArrowDown || event == Event::Character('j') || (event.is_mouse() && event.mouse().button == Mouse::WheelDown)) {
    	  	return next_commit();
    	}
		return false;
	});

/*
 * END of commit-tree TODO
 * Probably shouldn't have to change anything outside of this. 
 */

  	int logSize{45};
  	int footerSize{1};
  	Component container{managerRenderer};
  	container = ResizableSplitLeft(commitTree, container, &logSize);
  	container = ResizableSplitBottom(footerRenderer, container, &footerSize);
	
	commitTree->TakeFocus();

	// add application shortcuts
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
		{"-n, --no-tooltips", "", "Hide Tooltips in promotion view."}
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
