#include "cpplibostree.h"

// C++
#include <cstdlib>
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

    g_autoptr (GError) error = NULL;
    OstreeRepo *repo = NULL;

    g_autoptr (GFile) repo_path_ptr = g_file_new_for_path (repo_path.c_str());
    g_autoptr (OstreeRepo) ret_repo = ostree_repo_new (repo_path_ptr);
    if (!ostree_repo_open (ret_repo, NULL, &error))
        std::cout << "ostree error\n";

    osr = OSTREE_REPO (g_steal_pointer (&ret_repo));
}

OSTreeRepo::~OSTreeRepo() {
    g_free(osr);
}

OstreeRepo* OSTreeRepo::_c() {
    return osr;
}
