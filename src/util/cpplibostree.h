/*
 * Wrapper for libostree for C++
 * - C++ style lifetimes
 * - C++ style usage
 */

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
        OstreeRepo* osr;
    public:
        OSTreeRepo(std::string repo_path);
        ~OSTreeRepo();
    };

} // namespace cpplibostree
