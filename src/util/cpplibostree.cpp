#include "cpplibostree.h"

// C++
#include <string>
#include <iostream>
// C
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
// external
#include <ostree.h>
#include <glib.h>

using namespace cpplibostree;

OSTreeRepo::OSTreeRepo(std::string repo_path) {
    int dfd;
    if ((dfd = open(repo_path.c_str(), O_DIRECTORY | O_RDONLY)) == -1) {
    	std::cout << "couldn't find repository '" + repo_path + "'\n";
    }
    osr = ostree_repo_open_at (dfd,
             repo_path.c_str(),
             nullptr,
             nullptr);
}

OSTreeRepo::~OSTreeRepo() {
    g_free(osr);
}
