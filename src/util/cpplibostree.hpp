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
#include <chrono>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
// C
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
// external
#include <glib.h>
#include <ostree.h>


namespace cpplibostree {

    using Clock = std::chrono::utc_clock;
    using Timepoint = std::chrono::time_point<Clock>;

    struct Signature {
        bool valid      {false};
        bool sig_expired{true};
        bool key_expired{true};
        bool key_revoked{false};
        bool key_missing{true};
        std::string fingerprint;
        std::string fingerprint_primary;
        Timepoint timestamp;
        Timepoint expire_timestamp;
        std::string pubkey_algorithm;
        std::string username;
        std::string usermail;
        Timepoint key_expire_timestamp;
        Timepoint key_expire_timestamp_primary;
    } __attribute__((aligned(128)));

    struct Commit {
        std::string hash;
        std::string contentChecksum;
        std::string subject{"Error - invalid commit state"};
        std::string body;
        Timepoint timestamp;
        std::string parent;
        // a commit already stores, to which branches it belongs to
	    std::vector<std::string> branches;
        std::vector<Signature> signatures;
    } __attribute__((aligned(128)));

    // map commit hash to commit
    using CommitList = std::unordered_map<std::string,Commit>;

    /**
     * @brief OSTreeRepo functions as a C++ wrapper around libostree's OstreeRepo. 
     * The complete OSTree repository gets parsed into a complete commit list in
     * commit_list and a list of refs in branches.
     * 
     */
    class OSTreeRepo {
    private:
        std::string repo_path;
        CommitList commit_list;
        std::vector<std::string> branches;

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

        /// Getter
        std::string getRepoPath();
        /// Getter
        CommitList getCommitList();
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
        static bool isCommitSigned(const Commit& commit);

        /**
         * @brief Parse commits from a ostree log output to a commit_list, mapping
         * the hashes to commits.
         * 
         * @param branch 
         * @return std::unordered_map<std::string,Commit> 
         */
        CommitList parseCommitsOfBranch(const std::string& branch);
        
        /**
         * @brief Performs parseCommitsOfBranch() on all available branches and
         * merges all commit lists into one.
         * 
         * @return std::unordered_map<std::string,Commit> 
         */
        CommitList parseCommitsAllBranches();

        // read & write access to OSTree repo:

        /**
         * @brief Promotes a commit to another branch. Similar to: 
         * `ostree commit --repo=repo -b newRef -s newSubject --tree=ref=hash`
         * 
         * @param hash hash of the commit to promote
         * @param newRef branch to promote to
         * @param newSubject new commit subject, it needed
         * @param keepMetadata should new commit keep metadata of old commit
         * @return true on success
         * @return false on failed promotion
         */
        bool promoteCommit(const std::string& hash, const std::string& newRef, const std::vector<std::string> addMetadataStrings, const std::string& newSubject = "", bool keepMetadata = true);

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
        Commit parseCommit(GVariant *variant, const std::string& branch, const std::string& hash);
        
        /**
         * @brief Parse all commits in a OstreeRepo into a commit vector.
         * 
         * @param repo pointer to libostree Ostree repository
         * @param checksum checksum of first commit
         * @param error gets set, if an error occurred during parsing
         * @param commit_list commit list to parse the commits into
         * @param branch branch to read the commit from
         * @param is_recurse !Do not use!, or set to false. Used only for recursion.
         * @return true if parsing was successful
         * @return false if an error occurred during parsing
         */
        gboolean parseCommitsRecursive (OstreeRepo *repo, const gchar *checksum, GError **error,
                                        CommitList *commit_list, const std::string& branch,
                                        gboolean is_recurse = false);
    };

} // namespace cpplibostree
