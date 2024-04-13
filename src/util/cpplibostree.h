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

#pragma once
// C++
#include <string>
#include <sys/types.h>
#include <vector>
#include <set>
// C
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
// external
#include <glib-2.0/glib.h>
#include <ostree-1/ostree.h>

struct Commit {
    // GVariant
    std::string subject;
    std::string body;
    u_int64_t timestamp;
    std::string parent;
    // 
    std::string contentChecksum;
    std::string hash;
    std::string date;
	std::string branch;
    std::vector<std::string> signatures;// replace with signature
};

namespace cpplibostree {

    class OSTreeRepo {
    private:
        //OstreeRepo* repo;
        //GError* error;

        std::string repo_path;
        std::vector<Commit> commit_list = {};
        std::vector<std::string> branches = {};

    public:
        // Class
        explicit OSTreeRepo(std::string repo_path);

        // custom methods
        OstreeRepo* _c(); // ?

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
            void setBranches(std::vector<std::string> branches); // TODO replace -> update (don't modify from outside)

    };

} // namespace cpplibostree
