/*_____________________________________________________________
 | incomplete Wrapper for libostree for C++
 | - C++ style lifetimes
 | - C++ style usage
 |
 | all methods of classes are converted in the following way:
 | - if viable, output is returned, not passed as pointer
 | - *self is not needed, rather integrated in class method
 | - cancellable is automatically generated & doesn't need to
 |   be passed
 | - errors are thrown, not passed as pointer
 |___________________________________________________________/

// C++
#include <string>
// C
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
// external
#include <ostree.h>
#include <glib.h>

namespace cpplibostree {

    class OSTreeRepo {
    public:
        //g_autoptr(OstreeRepo) osr;
        OstreeRepo* osr;

    public:
        // Class
        explicit OSTreeRepo(std::string repo_path);
        ~OSTreeRepo();

        // custom methods
        OstreeRepo* _c();

        // libostree methods
        GHashTable ostree_repo_list_refs(std::string refspec_prefix);

    }; // class OSTreeRepo

} // namespace cpplibostree
*/