#include "cpplibostree.h"

// C++
#include <cstdlib>
#include <string>
#include <iostream>
#include <vector>
// C
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
// external
#include <ostree.h>
#include <glib-2.0/glib.h>
#include <vector>

using namespace cpplibostree;

OSTreeRepo::OSTreeRepo(std::string repo_path) {

    // open ostree repository
    GError *error = NULL;
    OstreeRepo *repo = ostree_repo_open_at(AT_FDCWD, repo_path.c_str(), NULL, &error);

    if (repo == NULL) {
        g_printerr("Error opening repository: %s\n", error->message);
        g_error_free(error);
        // TODO exit with error
    }

    // (TEST) get a list of refs
        GHashTable *refs_hash = NULL;
        gboolean result = ostree_repo_list_refs_ext(repo, NULL, &refs_hash, OSTREE_REPO_LIST_REFS_EXT_NONE, NULL, &error);
        if (!result) {
            g_printerr("Error listing refs: %s\n", error->message);
            g_error_free(error);
            g_object_unref(repo);
            // TODO exit with error
        }
        // Iterate through the refs and print them
        GHashTableIter iter;
        gpointer key, value;
        g_hash_table_iter_init(&iter, refs_hash);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            const gchar *ref_name = (const gchar *)key;
            g_print("\nref found: %s\n", ref_name);
        }
        // Cleanup: Free resources
        g_hash_table_unref(refs_hash);
    // (TEST)

    osr = repo;
}

OSTreeRepo::~OSTreeRepo() {
    g_object_unref(osr);
}

// METHODS

auto OSTreeRepo::_c() -> OstreeRepo* {
    return osr;
}

auto OSTreeRepo::getRepo() -> std::string* {
    // TODO
    return nullptr;
}

auto OSTreeRepo::getCommitList() -> std::vector<Commit>* {
    // TODO
    std::vector<Commit> ret;
    return &ret;
}

auto OSTreeRepo::getCommitListSorted() -> std::vector<Commit>* {
    // TODO
    std::vector<Commit> ret;
    return &ret;
}

auto OSTreeRepo::getBranches() -> std::vector<std::string>* {
    // TODO
    std::vector<std::string> ret;
    return &ret;
}

auto OSTreeRepo::updateData() -> bool {
    // TODO
    return false;
}

auto OSTreeRepo::isCommitSigned(const Commit& commit) -> bool {
    // TODO
    return false;
}

auto OSTreeRepo::parseCommits(std::string branch) -> std::vector<Commit> {
    // TODO
    return std::vector<Commit>();
}

auto OSTreeRepo::parseCommitsAllBranches() -> std::vector<Commit> {
    // TODO
    return std::vector<Commit>();
}

auto OSTreeRepo::getBranchesAsString() -> std::string {
    // TODO
    return "";
}

auto OSTreeRepo::getLogStringOfBranch(const std::string& branch) -> std::string {
    // TODO
    return "";
}

void OSTreeRepo::setBranches(std::vector<std::string> branches) { // TODO replace -> update (don't modify from outside)
    // TODO
}
