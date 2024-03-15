#include "cl_ostree.h"

#include <string>
#include <algorithm>

// class OSTreeRepo
cl_ostree::OSTreeRepo::OSTreeRepo(std::string repo):
        repo_path(repo),
        commit_list({}),
        branches({})
    {}

auto cl_ostree::OSTreeRepo::getRepo() -> std::string* {
    return &repo_path;
}

auto cl_ostree::OSTreeRepo::getCommitList() -> std::vector<Commit>* {
    return &commit_list;
}

auto cl_ostree::OSTreeRepo::setCommitList(std::vector<Commit> commit_list) -> void {
    this->commit_list = commit_list; 
}

auto cl_ostree::OSTreeRepo::getCommitListSorted() -> std::vector<Commit> {
    std::sort(commit_list.begin(), commit_list.end(), [](const Commit& lhs, const Commit& rhs) {
      return lhs.hash.compare(rhs.hash) > 0;
    });
    std::stable_sort(commit_list.begin(), commit_list.end(), [](const Commit& lhs, const Commit& rhs) {
      return lhs.parent == rhs.hash;
    });
    return commit_list;
}

auto cl_ostree::OSTreeRepo::getBranches() -> std::vector<std::string>* {
    return &branches;
}

auto cl_ostree::OSTreeRepo::setBranches(std::vector<std::string> branches) -> void {
    this->branches = branches; 
}

// _Command_Line_Methods_______________________________________

auto cl_ostree::OSTreeRepo::isCommitSigned(Commit commit) -> bool {
    std::string cmd = CMD_HEAD + repo_path + " show " + commit.hash + " | grep Signature";
	std::string out = commandline::exec(cmd.c_str());
	return !out.empty();
}

auto cl_ostree::OSTreeRepo::getBranchesAsString() -> std::string {
    std::string command = CMD_HEAD + repo_path + " refs";
	return commandline::exec(command.c_str());
}

auto cl_ostree::OSTreeRepo::getLogStringOfBranch(std::string branch) -> std::string {
    auto command = CMD_HEAD + repo_path + " log " + branch;
  	return commandline::exec(command.c_str());
}
