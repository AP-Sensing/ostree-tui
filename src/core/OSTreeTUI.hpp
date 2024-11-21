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

enum ViewMode : uint8_t { DEFAULT, COMMIT_PROMOTION, COMMIT_DROP };

class OSTreeTUI {
   public:
    /**
     * @brief Constructs, builds and assembles all components of the OSTreeTUI.
     *
     * @param repo Path to the OSTree repository directory.
     * @param startupBranches Optional list of branches to pre-select at startup (providing nothing
     * will display all branches).
     */
    explicit OSTreeTUI(const std::string& repo,
                       const std::vector<std::string>& startupBranches = {});

    /**
     * @brief Runs the OSTreeTUI (starts the ftxui screen loop).
     *
     * @return Exit Code
     */
    int Run();

    /// @brief OSTreeTUI Refresh Level 3: Refreshes the commit components.
    void RefreshCommitComponents();

    /// @brief OSTreeTUI Refresh Level 2: Refreshes the commit list component & upper levels.
    void RefreshCommitListComponent();

    /// @brief OSTreeTUI Refresh Level 1: Refreshes complete repository & upper levels.
    bool RefreshOSTreeRepository();

    /**
     * @brief Sets the view mode: Defines if the ostree-tui currently displays a commit
     * promotion window, deletion window, ...
     *
     * @param viewMode View mode to set the OSTreeTUI to.
     * @param targetCommitHash Commit, that is influenced by the view mode (e.g. commit to promote,
     * or delete).
     * @param setTargetBranch True = Reset branch, that might be influenced by the view mode (e.g.
     * selected promotion branch).
     * @return true, if view mode changed.
     */
    bool SetViewMode(ViewMode viewMode,
                     const std::string& targetCommitHash = "",
                     bool targetBranch = true);

    /**
     * @brief Promotes a commit, by passing it to the cpplibostree and refreshing the UI.
     *
     * @param hash Hash of commit to be promoted.
     * @param targetBranch Branch to promote the commit to.
     * @param metadataStrings Optional additional metadata-strings to be set.
     * @param newSubject New commit subject.
     * @param keepMetadata Keep metadata of old commit.
     * @return promotion success
     */
    bool PromoteCommit(const std::string& hash,
                       const std::string& targetBranch,
                       const std::vector<std::string>& metadataStrings = {},
                       const std::string& newSubject = "",
                       bool keepMetadata = true);

    /**
     * @brief Deleted a commit, if it is the last commit on a branch (= reset branch head) and
     * refresh the UI.
     *
     * @param commit Commit to drop.
     * @return True on success.
     */
    bool DropLastCommit(const cpplibostree::Commit& commit);

   private:
    /// @brief Calculates all visible commits from an OSTreeRepo and a list of branches.
    void parseVisibleCommitMap();

    /// @brief Adjust scroll offset to fit the selected commit.
    void adjustScrollToSelectedCommit();

   public:
    // SETTER
    void SetModeBranch(const std::string& modeBranch);
    void SetSelectedCommit(size_t selectedCommit);

    // non-const GETTER
    [[nodiscard]] std::vector<std::string>& GetColumnToBranchMap();
    [[nodiscard]] ftxui::ScreenInteractive& GetScreen();

    // GETTER
    [[nodiscard]] const cpplibostree::OSTreeRepo& GetOstreeRepo() const;
    [[nodiscard]] const size_t& GetSelectedCommit() const;
    [[nodiscard]] const std::string& GetModeBranch() const;
    [[nodiscard]] const std::unordered_map<std::string, bool>& GetVisibleBranches() const;
    [[nodiscard]] const std::vector<std::string>& GetColumnToBranchMap() const;
    [[nodiscard]] const std::vector<std::string>& GetVisibleCommitViewMap() const;
    [[nodiscard]] const std::unordered_map<std::string, ftxui::Color>& GetBranchColorMap() const;
    [[nodiscard]] int GetScrollOffset() const;
    [[nodiscard]] ViewMode GetViewMode() const;
    [[nodiscard]] const std::string& GetModeHash() const;

   public:
    // model
    cpplibostree::OSTreeRepo ostreeRepo;

    // backend states
    size_t selectedCommit;
    std::unordered_map<std::string, bool> visibleBranches;  // map branch -> visibe
    std::vector<std::string> columnToBranchMap;             // map branch -> column in commit-tree
    std::vector<std::string> visibleCommitViewMap;          // map view-index -> commit-hash
    std::unordered_map<std::string, ftxui::Color> branchColorMap;  // map branch -> color
    std::string notificationText;                                  // footer notification

    // view states
    int scrollOffset{0};
    ViewMode viewMode = ViewMode::DEFAULT;
    std::string modeHash;
    std::string modeBranch;

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
    ftxui::Component FooterRenderer;
    ftxui::Component container;

   public:
    /**
     * @brief Print a help page including usage, options, etc.
     *
     * @param caller argv[0]
     * @param errorMessage Optional error message to print on top.
     * @return Exit Code
     */
    static int showHelp(const std::string& caller, const std::string& errorMessage = "");

    /**
     * @brief Print the application version
     *
     * @return Exit Code
     */
    static int showVersion();
};
