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

    /**
     * @brief OSTreeRepo functions as a C++ wrapper around libostree's OstreeRepo. 
     * The complete OSTree repository gets parsed into a complete commit list in
     * commit_list and a list of refs in branches.
     * 
     */
    class OSTreeRepo {
    private:
        std::string repo_path;
        std::unordered_map<std::string,Commit> commit_list = {}; // map commit hash to commit
        std::vector<std::string> branches = {};

    public:
        /**
         * @brief Construct a new OSTreeRepo.
         * 
         * @param repo_path Path to the OSTree Repository
         */
        explicit OSTreeRepo(std::string repo_path);

        /**
         * @brief Return a C-style pointer to a libostree OstreeRepo. This exists, to be
         * able to access functions, that have not yet been adapted in this C++ wrapper.
         * 
         * @return OstreeRepo* 
         */
        OstreeRepo* _c();

        // Getters

        /// Getter
        std::string* getRepo();
        /// Getter
        std::unordered_map<std::string,Commit> getCommitList();
        /// Getter
        std::vector<std::string> getBranches();

        // Methods

        /**
         * @brief Reload the OSTree repository data.
         * 
         * @return true if data was changed during the reload
         * @return false if nothing changed
         */
        bool updateData();

        /**
         * @brief Check if a certain commit is signed. This simply accesses the
         * size() of commit.signatures.
         * 
         * @param commit 
         * @return true if the commit is signed
         * @return false if the commit is not signed
         */
        bool isCommitSigned(const Commit& commit);

        /**
         * @brief Parse commits from a ostree log output to a commit_list, mapping
         * the hashes to commits.
         * 
         * @param branch 
         * @return std::unordered_map<std::string,Commit> 
         */
        std::unordered_map<std::string,Commit> parseCommitsOfBranch(std::string branch);
        
        /**
         * @brief Performs parseCommitsOfBranch() on all available branches and
         * merges all commit lists into one.
         * 
         * @return std::unordered_map<std::string,Commit> 
         */
        std::unordered_map<std::string,Commit> parseCommitsAllBranches();

    private:
        /**
         * @brief Get all branches as a single string, separated by spaces.
         * 
         * @return std::string All branch names, separated by spaces
         */
        std::string getBranchesAsString();

        /**
         * @brief Parse a libostree GVariant commit to a C++ commit struct.
         * 
         * @param variant pointer to GVariant commit
         * @param branch branch of the commit
         * @param hash commit hash
         * @return Commit struct
         */
        Commit parseCommit(GVariant *variant, std::string branch, std::string hash);
        
        /**
         * @brief Parse all commits in a OstreeRepo into a commit vector.
         * 
         * @param repo pointer to libostree Ostree repository
         * @param checksum checksum of first commit
         * @param is_recurse !Do not use!, or set to false. Used only for recursion.
         * @param error gets set, if an error occurred during parsing
         * @param commit_list commit list to parse the commits into
         * @param branch branch to read the commit from
         * @return true if parsing was successful
         * @return false if an error occurred during parsing
         */
        gboolean parseCommitsRecursive (OstreeRepo *repo, const gchar *checksum, gboolean is_recurse, GError **error, std::unordered_map<std::string,Commit> *commit_list, std::string branch);
    };

} // namespace cpplibostree
