/*_____________________________________________________________
 | Interface for command line ostree
 |   Offers an interface to execute common functions on ostree
 |   repositories. Depending on the scope of this project, this
 |   file will be changed to an interface to libostree, instead
 |   of the commandline access (see cpplibostree.h).
 |___________________________________________________________*/

#include "commandline.h"

#include <string>
#include <vector>

#pragma once

struct Commit {
    std::string hash;
    std::string parent;
    std::string contentChecksum;
    std::string date;
    std::string subject;
	std::string branch;
};

namespace cl_ostree {

    class OSTreeRepo {
        private:
            std::string repo_path;
            std::vector<Commit> commit_list = {};
            std::vector<std::string> branches = {};
        
        public:
            explicit OSTreeRepo(std::string repo_path);

            bool updateData();

            std::string* getRepo();

            std::vector<Commit>* getCommitList();
            void setCommitList(std::vector<Commit> commit_list);
            std::vector<Commit> getCommitListSorted();
            bool isCommitSigned(Commit commit);

            std::vector<std::string>* getBranches();
            void setBranches(std::vector<std::string> branches); // TODO replace with update, do not modify from outside
            std::string getBranchesAsString();
    };

    bool isCommitSigned(std::string repo_path, Commit commit);
    std::string getAllBranches(std::string repo_path);

} // namespace cl_ostree
