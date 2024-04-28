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
#include <unordered_map>
#include <set>
// C
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
// external
#include <glib-2.0/glib.h>
#include <ostree-1/ostree.h>

struct Signature {
    std::string pubkey_algorithm;
    std::string fingerprint;
    std::string timestamp;
    std::string expire_timestamp;
    std::string username;
    std::string usermail;
};

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
    // a commit already stores, to which branches it belongs to
	std::vector<std::string> branches;
    std::vector<Signature> signatures;
};

namespace cpplibostree {

    class OSTreeRepo {
    private:
        std::string repo_path;
        std::unordered_map<std::string,Commit> commit_list = {}; // map commit hash to commit
        std::vector<std::string> branches = {};

    public:
        // Class
        explicit OSTreeRepo(std::string repo_path);

        // custom methods
        OstreeRepo* _c(); // ?

        // Getters

            std::string* getRepo();
            std::unordered_map<std::string,Commit> getCommitList();
            std::vector<std::string> getBranches();

        // Methods

            /// update all data
            bool updateData();
            /// check if a certain commit is signed
            bool isCommitSigned(const Commit& commit);
            /// parse commits from a ostree log output
            std::unordered_map<std::string,Commit> parseCommitsOfBranch(std::string branch);
            /// same as parseCommitsOfBranch(), but on all available branches
            std::unordered_map<std::string,Commit> parseCommitsAllBranches();
            /// get ostree refs
            std::string getBranchesAsString();
            void setBranches(std::vector<std::string> branches); // TODO separate model and view -> update (don't modify from outside)
    private:
            /// parse a GVarian commit to Commit struct
            Commit parseCommit(GVariant *variant, std::string branch, std::string hash);
            /// parse all commits in a OstreeRepo into a commit vector
            gboolean parseCommitsRecursive (OstreeRepo *repo, const gchar *checksum, gboolean is_recurse, GError **error, std::unordered_map<std::string,Commit> *commit_list, std::string branch);
    };

} // namespace cpplibostree
