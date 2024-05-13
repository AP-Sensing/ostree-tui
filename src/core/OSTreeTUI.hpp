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
    int main(const std::string& repo);

    std::vector<std::string> parseVisibleCommitMap(cpplibostree::OSTreeRepo& repo,
                            std::unordered_map<std::string, bool>& visible_branches);

} // namespace OSTreeTUI
