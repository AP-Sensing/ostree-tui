/*_____________________________________________________________
 | partial C++ wrapper for libostree
 | - C++ style lifetimes
 | - C++ style usage
 |
 | all methods of classes are converted in the following way:
 | - if viable, output is returned, not passed as pointer
 | - *self is not needed, rather integrated in class method
 | - cancellable is automatically generated & doesn't need to
 |   be passed
 | - errors are thrown, not passed as pointer
 |___________________________________________________________*/

// C++
#include <string>
#include <vector>
#include <set>
// C
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
// external
#include <glib-2.0/glib.h>
#include <ostree-1/ostree.h>


namespace cpplibostree {

    /// TODO remove & replace by GVariant Commit of ostreelib
    struct Commit {
        std::string hash;
        std::string parent;
        std::string contentChecksum;
        std::string date;
        std::string subject;
    	std::string branch;
        std::set<std::string> signatures;// replace with signature
    };

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

        // Getters

            std::string* getRepo();
            std::vector<Commit>* getCommitList();
            std::vector<Commit>* getCommitListSorted();
            std::vector<std::string>* getBranches();

        // Methods

            /// update all data
            bool updateData();
            /// check if a certain commit is signed
            bool isCommitSigned(const Commit& commit);
            /// parse commits from a ostree log output
            std::vector<Commit> parseCommits(std::string branch);
            /// same as parseCommits(), but on all available branches
            std::vector<Commit> parseCommitsAllBranches();
            /// get ostree refs
            std::string getBranchesAsString();
            std::string getLogStringOfBranch(const std::string& branch);
            void setBranches(std::vector<std::string> branches); // TODO replace -> update (don't modify from outside)

    };

} // namespace cpplibostree
