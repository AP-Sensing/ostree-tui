/*_____________________________________________________________
 | OSTree TUI
 |   A terminal user interface for OSTree.
 |___________________________________________________________*/

#include <string>
#include <vector>

#include "../util/cpplibostree.hpp"

namespace OSTreeTUI {

    /**
     * @brief OSTree TUI main
     * 
     * @param repo ostree repository path
     */
    int main(const std::string& repo, const std::vector<std::string>& startupBranches = {});

    /**
     * @brief Print help page
     * 
     * @param caller argv[0]
     * @param errorMessage optional error message to print on top
     * @return 0, if no error message provided
     * @return 1, if error message is provided, assuming bad program stop
     */
    int showHelp(const std::string& caller, const std::string& errorMessage = "");

    /**
     * @brief Print the application version
     * 
     * @return int
     */
    int showVersion();

    std::vector<std::string> parseVisibleCommitMap(cpplibostree::OSTreeRepo& repo,
                            std::unordered_map<std::string, bool>& visibleBranches);

} // namespace OSTreeTUI
