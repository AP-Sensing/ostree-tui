#include <iostream>
#include <string>
#include <vector>

#include "core/OSTreeTUI.hpp"

/**
 * @brief Parse all options listed behind an argument
 * 
 * @param args argument list
 * @param find argument (as vector for multiple arg version support, like -h & --help)
 * @return vector of options
 */
std::vector<std::string> getArgOptions(const std::vector<std::string>& args, const std::vector<std::string>& find) {
	std::vector<std::string> out;
	for (const auto& arg : find) {
		auto argIndex = std::find(args.begin(), args.end(), arg);
		if (argIndex == args.end()) {
			continue;
		}
		while (++argIndex != args.end()) {
			if ((*argIndex).at(0) == '-') {
				// arrived at next argument -> stop
				break;
			}
			out.push_back(*argIndex);
		}
	}
	return out;
}

/// Check if argument exists in argument list
bool argExists(const std::vector<std::string>& args, std::string arg) {
	return std::find(args.begin(), args.end(), arg) != args.end();
}

/// main for argument parsing and OSTree TUI call
int main(int argc, const char** argv) {
	// too few amount of arguments
	if (argc <= 1) {
		return OSTreeTUI::help(argc <= 0 ? "ostree" : argv[0], "no repository provided");
	}

	std::vector<std::string> args(argv + 1, argv + argc);
	// -h, --help
	if (argExists(args,"-h") || argExists(args,"--help")) {
		return OSTreeTUI::help(argv[0]);
	}
	// assume ostree repository path as first argument
	std::string repo = args.at(0);
	// -r, --refs
	std::vector<std::string> startupBranches = getArgOptions(args, {"-r", "--refs"});
	
	// OSTree TUI
	return OSTreeTUI::main(repo, startupBranches);
}
