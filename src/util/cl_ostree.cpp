#include "cl_ostree.h"

#include <string>
#include <sstream>
#include <algorithm>
#include <utility>

#include "commandline.h"


cl_ostree::OSTreeRepo::OSTreeRepo(std::string repo):
        repo_path(std::move(repo)),
        branches({}) {
    updateData();
}

auto cl_ostree::OSTreeRepo::updateData() -> bool {
    commit_list = parseCommitsAllBranches();
    // TODO update branches
	return true;
}

auto cl_ostree::OSTreeRepo::getRepo() -> std::string* {
    return &repo_path;
}

auto cl_ostree::OSTreeRepo::getCommitList() -> std::vector<Commit>* {
    return &commit_list;
}

auto cl_ostree::OSTreeRepo::getCommitListSorted() -> std::vector<Commit>* {
    std::sort(commit_list.begin(), commit_list.end(), [](const Commit& lhs, const Commit& rhs) {
      return lhs.hash.compare(rhs.hash) > 0;
    });
    std::stable_sort(commit_list.begin(), commit_list.end(), [](const Commit& lhs, const Commit& rhs) {
      return lhs.parent == rhs.hash;
    });
    return &commit_list;
}

auto cl_ostree::OSTreeRepo::getBranches() -> std::vector<std::string>* {
    return &branches;
}

auto cl_ostree::OSTreeRepo::setBranches(std::vector<std::string> branches) -> void {
    this->branches = std::move(branches); 
}

auto cl_ostree::OSTreeRepo::parseCommitsAllBranches() -> std::vector<Commit> {
    
	std::stringstream branches_ss(getBranchesAsString());
	std::string branch;

	std::vector<Commit> commitList;

	while (branches_ss >> branch) {
		auto commits = parseCommits(branch);
		commitList.insert(commitList.end(), commits.begin(), commits.end());
	}
  	
	return commitList;
}

auto cl_ostree::OSTreeRepo::parseCommits(std::string branch) -> std::vector<Commit> {
  	std::vector<Commit> commitList;
  	std::stringstream log(getLogStringOfBranch(branch));
  	std::string word;

	Commit cur = {"error", "couldn't read commit", "", "", "", branch};

	bool ready = false;
  	while (log >> word) {
    	if (word == "commit") {
			if (ready) {
				commitList.push_back(std::move(cur));
			}
			ready = true;
			// create new commit
			cur = {"", "", "", "", "", branch};
			log >> word;
			cur.hash = word;
		} else if (word == "Parent:") {
			log >> word;
			cur.parent = word;
		} else if (word == "ContentChecksum:") {
			log >> word;
			cur.contentChecksum = word;
		} else if (word == "Date:") {
			log >> word;
			cur.date = word;
		}
  	}
	commitList.push_back(std::move(cur));

	return commitList;
}

// _Command_Line_Methods_______________________________________

auto cl_ostree::OSTreeRepo::isCommitSigned(const Commit& commit) -> bool {
    std::string cmd = CMD_HEAD + repo_path + " show " + commit.hash + " | grep Signature";
	std::string out = commandline::exec(cmd.c_str());
	return !out.empty();
}

auto cl_ostree::OSTreeRepo::getBranchesAsString() -> std::string {
    std::string command = CMD_HEAD + repo_path + " refs";
	return commandline::exec(command.c_str());
}

auto cl_ostree::OSTreeRepo::getLogStringOfBranch(const std::string& branch) -> std::string {
    auto command = CMD_HEAD + repo_path + " log " + branch;
  	return commandline::exec(command.c_str());
}
