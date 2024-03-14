#include "cl_ostree.h"

#include <string>

bool cl_ostree::isCommitSigned(std::string repo_path, Commit commit) {
	std::string cmd = "ostree --repo=" + repo_path + " show " + commit.hash + " | grep Signature";
	std::string out = commandline::exec(cmd.c_str());
	return !out.empty();
}

std::string cl_ostree::getAllBranches(std::string repo_path) {
    std::string command = "ostree refs --repo=" + repo_path;
	return commandline::exec(command.c_str());
}
