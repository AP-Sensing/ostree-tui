/*_____________________________________________________________
 | Interface for command line ostree
 |   Offers an interface to execute common functions on ostree
 |   repositories. Depending on the scope of this project, this
 |   file will be changed to an interface to libostree, instead
 |   of the commandline access (see cpplibostree.h).
 |___________________________________________________________*/

#include <string>
#include <vector>

#pragma once

/// represents a OSTree commit
struct Commit {
    std::string hash;
    std::string parent;
    std::string contentChecksum;
    std::string date;
    std::string subject;
	std::string branch;
};

constexpr auto CMD_HEAD = "ostree --repo=";

namespace cl_ostree {

    /// OSTree repository representation, must be kept up-to-date by calling update()
    class OSTreeRepo {
        private:
            std::string repo_path;
            std::vector<Commit> commit_list = {};
            std::vector<std::string> branches = {};
        
        public:
            explicit OSTreeRepo(std::string repo_path);

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

} // namespace cl_ostree
