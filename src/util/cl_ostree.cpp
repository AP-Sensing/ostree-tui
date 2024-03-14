#include "cl_ostree.h"

#include <string>
#include <algorithm>

// class OSTreeRepo
cl_ostree::OSTreeRepo::OSTreeRepo(std::string repo):
        repo_path(repo),
        commit_list({}),
        branches({})
    {}

auto cl_ostree::OSTreeRepo::getCommitList() -> std::vector<Commit> {
    return commit_list;
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

auto cl_ostree::OSTreeRepo::isCommitSigned(Commit commit) -> bool {
    std::string cmd = "ostree --repo=" + repo_path + " show " + commit.hash + " | grep Signature";
	std::string out = commandline::exec(cmd.c_str());
	return !out.empty();
}

auto cl_ostree::OSTreeRepo::getAllBranchesAsString() -> std::string {
    std::string command = "ostree refs --repo=" + repo_path;
	return commandline::exec(command.c_str());
}

// deprecated
bool cl_ostree::isCommitSigned(std::string repo_path, Commit commit) {
	std::string cmd = "ostree --repo=" + repo_path + " show " + commit.hash + " | grep Signature";
	std::string out = commandline::exec(cmd.c_str());
	return !out.empty();
}

// deprecated
std::string cl_ostree::getAllBranches(std::string repo_path) {
    std::string command = "ostree refs --repo=" + repo_path;
	return commandline::exec(command.c_str());
}
