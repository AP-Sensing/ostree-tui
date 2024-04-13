#include "cpplibostree.h"

// C++
#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
// C
#include <cstdio>
#include <cerrno>
#include <fcntl.h>
// external
#include <ostree.h>
#include <glib-2.0/glib.h>
#include <vector>

using namespace cpplibostree;

OSTreeRepo::OSTreeRepo(std::string path):
        repo_path(std::move(path)), 
        branches({}) {
    updateData();
}

auto OSTreeRepo::updateData() -> bool {

    std::string branchString = getBranchesAsString();
    std::stringstream bss(branchString);
    std::string word;
    while (bss >> word) {
        branches.push_back(word);
    }

    commit_list = parseCommitsAllBranches();

    return true;
}

// METHODS

auto OSTreeRepo::_c() -> OstreeRepo* {
    // TODO
    return nullptr;
}

auto OSTreeRepo::getRepo() -> std::string* {
    return &repo_path;
}

auto OSTreeRepo::getCommitList() -> std::vector<Commit>* {
    return &commit_list;
}

auto OSTreeRepo::getCommitListSorted() -> std::vector<Commit>* {
    std::stable_sort(commit_list.begin(), commit_list.end(), [](const Commit& lhs, const Commit& rhs) {
      return lhs.date.compare(rhs.date) > 0;
    });
    std::stable_sort(commit_list.begin(), commit_list.end(), [](const Commit& lhs, const Commit& rhs) {
      return lhs.parent == rhs.hash;
    });
    return &commit_list;
}

auto OSTreeRepo::getBranches() -> std::vector<std::string>* {
    return &branches;
}

auto OSTreeRepo::setBranches(std::vector<std::string> branches) -> void {
    this->branches = std::move(branches); 
}

auto OSTreeRepo::isCommitSigned(const Commit& commit) -> bool {
    // TODO
    return false;
}

// https://github.com/ostreedev/ostree/blob/main/src/ostree/ot-dump.c#L52
gchar * format_timestamp (guint64 timestamp, gboolean local_tz, GError **error) {
    GDateTime *dt;
    gchar *str;

    dt = g_date_time_new_from_unix_utc (timestamp);
    if (dt == NULL) {
        return NULL;
    }

    if (local_tz) {
        // Convert to local time and display in the locale's preferred representation.
        g_autoptr (GDateTime) dt_local = g_date_time_to_local (dt);
        str = g_date_time_format (dt_local, "%c");
    } else {
        str = g_date_time_format (dt, "%Y-%m-%d %H:%M:%S +0000");
    }
    
    g_date_time_unref (dt);
    return str;
}

// modified dump_commit() from
// https://github.com/ostreedev/ostree/blob/main/src/ostree/ot-dump.c#L120
Commit dump_commit (GVariant *variant, std::string branch) {
    Commit commit = {"error", "", 0, "", "", "", "", "", {}};

    const gchar *subject;
    const gchar *body;
    guint64 timestamp;
    g_autofree char *parent = NULL;
    g_autofree char *date = NULL;
    g_autofree char *version = NULL;
    g_autoptr (GError) local_error = NULL;

    // see OSTREE_COMMIT_GVARIANT_FORMAT
    g_variant_get (variant, "(a{sv}aya(say)&s&stayay)", NULL, NULL, NULL, &subject, &body, &timestamp,
                 NULL, NULL);

    timestamp = GUINT64_FROM_BE (timestamp);
    date = format_timestamp (timestamp, FALSE, &local_error);

    if ((parent = ostree_commit_get_parent (variant))) {
        commit.parent = parent;
    }

    g_autofree char *contents = ostree_commit_get_content_checksum (variant);
    commit.contentChecksum = contents ?: "<invalid commit>";
    commit.date = date;

    if (subject[0]) {
        commit.subject = subject;
    } else {
        commit.subject = "(no subject)";
    }

    if (body[0]) {
        commit.body = body;
    }

    commit.branch = branch;

    return commit;
}

// modified log_commit() from
// https://github.com/ostreedev/ostree/blob/main/src/ostree/ot-builtin-log.c#L40
gboolean log_commit (OstreeRepo *repo, const gchar *checksum, gboolean is_recurse, GError **error, std::vector<Commit> *commit_list, std::string branch) {
    GError *local_error = NULL;

    g_autoptr (GVariant) variant = NULL;
    if (!ostree_repo_load_variant (repo, OSTREE_OBJECT_TYPE_COMMIT, checksum, &variant, &local_error)) {
        if (is_recurse && g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
            g_clear_error (&local_error);
            return true;
        } else {
            g_propagate_error (error, local_error);
            return false;
        }
    }

    // TODO check for errors in dump_commit
    // TODO add commit hash
    commit_list->push_back(dump_commit(variant, branch));

    // Get the parent of this commit
    g_autofree char *parent = ostree_commit_get_parent (variant);
    if (parent && !log_commit (repo, parent, true, error, commit_list, branch)) {
        return false;
    }
    
    return true;
}

auto OSTreeRepo::parseCommits(std::string branch) -> std::vector<Commit> {
    auto ret = std::vector<Commit>();

    // TODO don't reopen new repo
    GError *error = NULL;
    OstreeRepo *repo = ostree_repo_open_at(AT_FDCWD, "testrepo", NULL, &error);
    if (repo == NULL) {
        g_printerr("Error opening repository: %s\n", error->message);
        g_error_free(error);
        return ret;
    }

    // recursive commit log
    g_autofree char *checksum = NULL;
    if (!ostree_repo_resolve_rev (repo, branch.c_str(), false, &checksum, &error)) {
        return ret;
    }

    log_commit(repo, checksum, false, &error, &ret, branch);

    return ret;
}

auto OSTreeRepo::parseCommitsAllBranches() -> std::vector<Commit> {
    
    std::istringstream branches_string(getBranchesAsString());
    std::string branch;

    std::vector<Commit> commits_all_branches;

    while (branches_string >> branch) {
        auto commits = parseCommits(branch);
        commits_all_branches.insert(commits_all_branches.end(), commits.begin(), commits.end());
    }

    return commits_all_branches;
}

auto OSTreeRepo::getBranchesAsString() -> std::string {
    std::string branches_str = "";

    // TODO don't reopen new repo
    // but it somehow fails when trying to use the already opened repo (?)
    GError *error = NULL;
    OstreeRepo *repo = ostree_repo_open_at(AT_FDCWD, repo_path.c_str(), NULL, &error);

    if (repo == NULL) {
        g_printerr("Error opening repository: %s\n", error->message);
        g_error_free(error);
        return "";
    }

    // get a list of refs
    GHashTable *refs_hash = NULL;
    gboolean result = ostree_repo_list_refs_ext(repo, NULL, &refs_hash, OSTREE_REPO_LIST_REFS_EXT_NONE, NULL, &error);
    if (!result) {
        g_printerr("Error listing refs: %s\n", error->message);
        g_error_free(error);
        g_object_unref(repo);
        // TODO exit with error
        return "";
    }

    // iterate through the refs
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, refs_hash);
    while (g_hash_table_iter_next(&iter, &key, &value)) {
        const gchar *ref_name = (const gchar *)key;
        branches_str += " " + std::string(ref_name);
    }

    // free
    g_hash_table_unref(refs_hash);

    return branches_str;
}
