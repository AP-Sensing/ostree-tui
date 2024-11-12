/*_____________________________________________________________
 | OSTree TUI
 |   A terminal user interface for OSTree.
 |___________________________________________________________*/

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ftxui/component/component.hpp"  // for Renderer, ResizableSplitBottom, ResizableSplitLeft, ResizableSplitRight, ResizableSplitTop
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"                  // for Element, operator|, text, center, border

#include "commit.hpp"
#include "footer.hpp"
#include "manager.hpp"

#include "../util/cpplibostree.hpp"

class OSTreeTUI {
   public:
    /**
     * @brief Constructs, builds and assembles all components of the OSTreeTUI.
     *
     * @param repo ostree repository (OSTreeRepo)
     * @param startupBranches optional list of branches to pre-select at startup (providing nothing
     * will display all branches)
     */
    explicit OSTreeTUI(const std::string& repo,
                       const std::vector<std::string> startupBranches = {});

    /**
     * @brief Runs the OSTreeTUI (starts the ftxui screen loop).
     *
     * @return exit code
     */
    int run();

    /// @brief Refresh Level 3: Refreshes the commit components
    void refresh_commitComponents();

    /// @brief Refresh Level 2: Refreshes the commit list component & upper levels
    void refresh_commitListComoponent();

    /// @brief Refresh Level 1: Refreshes complete repository & upper levels
    bool refresh_repository();

    /**
     * @brief sets the promotion mode
     * @param active activate (true), or deactivate (false) promotion mode
     * @param hash provide if setting mode to true
     * @return false, if other promotion gets overwritten
     */
    bool setPromotionMode(bool active, std::string hash = "", bool setPromotionBranch = true);

    /** @brief promote a commit
     * @param hash
     * @param branch
     * @return promotion success
     */
    bool promoteCommit(std::string hash,
                       std::string branch,
                       std::vector<std::string> metadataStrings = {},
                       std::string newSubject = "",
                       bool keepMetadata = true);

   private:
    /**
     * @brief Calculates all visible commits from an OSTreeRepo and a list of branches.
     *
     * @param repo OSTreeRepo
     * @param visibleBranches Map: branch name -> visible
     * @return Complete list of commit hashes in repo, that are part of the given branches
     */
    std::vector<std::string> parseVisibleCommitMap(
        cpplibostree::OSTreeRepo& repo,
        std::unordered_map<std::string, bool>& visibleBranches);

    /**
     * @brief Adjust scroll offset to fit the selected commit
     */
    void adjustScrollToSelectedCommit();

   public:
    // SETTER
    void setPromotionBranch(std::string promotionBranch);
    void setSelectedCommit(size_t selectedCommit);

    // non-const GETTER
    std::vector<std::string>& getColumnToBranchMap();
    ftxui::ScreenInteractive& getScreen();

    // GETTER
    const cpplibostree::OSTreeRepo& getOstreeRepo() const;
    const size_t& getSelectedCommit() const;
    const std::string& getPromotionBranch() const;
    const std::unordered_map<std::string, bool>& getVisibleBranches() const;
    const std::vector<std::string>& getColumnToBranchMap() const;
    const std::vector<std::string>& getVisibleCommitViewMap() const;
    const std::unordered_map<std::string, ftxui::Color>& getBranchColorMap() const;
    const int& getScrollOffset() const;
    const bool& getInPromotionSelection() const;
    const std::string& getPromotionHash() const;

   private:
    // model
    cpplibostree::OSTreeRepo ostreeRepo;

    // backend states
    size_t selectedCommit;
    std::unordered_map<std::string, bool> visibleBranches;  // map branch -> visibe
    std::vector<std::string>
        columnToBranchMap;  // map branch -> column in commit-tree (may be merged into one
                            // data-structure with visibleBranches)
    std::vector<std::string> visibleCommitViewMap;                 // map view-index -> commit-hash
    std::unordered_map<std::string, ftxui::Color> branchColorMap;  // map branch -> color
    std::string notificationText;                                  // footer notification

    // view states
    int scrollOffset{0};
    bool inPromotionSelection{false};
    std::string promotionHash;
    std::string promotionBranch;

    // view constants
    int logSize{45};
    int footerSize{1};

    // components
    Footer footer;
    std::unique_ptr<BranchBoxManager> filterManager{nullptr};
    std::unique_ptr<Manager> manager{nullptr};
    ftxui::ScreenInteractive screen;
    ftxui::Component mainContainer;
    ftxui::Components commitComponents;
    ftxui::Component commitList;
    ftxui::Component tree;
    ftxui::Component commitListComponent;
    ftxui::Component infoView;
    ftxui::Component filterView;
    ftxui::Component managerRenderer;
    ftxui::Component footerRenderer;
    ftxui::Component container;

   public:
    /**
     * @brief Print help page
     *
     * @param caller argv[0]
     * @param errorMessage optional error message to print on top
     * @return 0, if no error message provided
     * @return 1, if error message is provided, assuming bad program stop
     */
    static int showHelp(const std::string& caller, const std::string& errorMessage = "");

    /**
     * @brief Print the application version
     *
     * @return int
     */
    static int showVersion();
};
