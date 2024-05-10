#include <iostream>
#include <string>

#include "core/OSTreeTUI.h"


int main(int argc, const char** argv) {
	// argument parsing
	argc--;
	argv++;
	if (argc <= 0) {
		std::cout << "no repository provided\n";
		std::cout << "usage: " << argv[-1] << " repository\n";
		return 0;
	}

	std::string repo = argv[0];
	
	// OSTree TUI
	return OSTreeTUI::main(repo);
}
