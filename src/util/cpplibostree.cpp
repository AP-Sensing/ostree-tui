#include "cpplibostree.h"

// C++
#include <cstdlib>
#include <string>
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

OSTreeRepo::OSTreeRepo(std::string path): repo_path(path) {

    // open ostree repository
    GError *error = NULL;
    OstreeRepo *repo = ostree_repo_open_at(AT_FDCWD, path.c_str(), NULL, &error);

    if (repo == NULL) {
        g_printerr("Error opening repository: %s\n", error->message);
        g_error_free(error);
        // TODO exit with error
    }

    // (TEST) get a list of refs
            g_print("\nrefs: %s\n", getBranchesAsString().c_str());
            parseCommits("foo");
    // (TEST)

    this->repo = repo;
}

OSTreeRepo::~OSTreeRepo() {
    g_object_unref(repo);
}

// METHODS

auto OSTreeRepo::_c() -> OstreeRepo* {
    return repo;
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

auto OSTreeRepo::updateData() -> bool {
    // TODO
    return false;
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
        //g_set_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
        //             "Invalid timestamp: %" G_GUINT64_FORMAT, timestamp);
        return NULL;
    }

    if (local_tz) {
        /* Convert to local time and display in the locale's preferred
         * representation.
         */
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
void dump_commit (GVariant *variant) {
    const gchar *subject;
    const gchar *body;
    guint64 timestamp;
    g_autofree char *parent = NULL;
    g_autofree char *str = NULL;
    g_autofree char *version = NULL;
    g_autoptr (GError) local_error = NULL;

    /* See OSTREE_COMMIT_GVARIANT_FORMAT */
    g_variant_get (variant, "(a{sv}aya(say)&s&stayay)", NULL, NULL, NULL, &subject, &body, &timestamp,
                 NULL, NULL);

    timestamp = GUINT64_FROM_BE (timestamp);
    str = format_timestamp (timestamp, FALSE, &local_error);
    if (!str) {
        g_assert (local_error); /* Pacify static analysis */
        //errx (1, "Failed to read commit: %s", local_error->message);
    }

    if ((parent = ostree_commit_get_parent (variant))) {
        g_print ("Parent:  %s\n", parent);
    }

    g_autofree char *contents = ostree_commit_get_content_checksum (variant);
    g_print ("ContentChecksum:  %s\n", contents ?: "<invalid commit>");
    g_print ("Date:  %s\n", str);

    //if ((version = ot_admin_checksum_version (variant))) {
    //    g_print ("Version: %s\n", version);
    //}

    if (subject[0]) {
        g_print ("\n");
        //dump_indented_lines (subject);
        g_print ("%s\n", subject);
    } else {
        g_print ("(no subject)\n");
    }

    if (body[0]) {
        g_print ("\n");
        //dump_indented_lines (body);
        g_print ("%s\n", body);
    }
    g_print ("\n");
}

// modified log_commit() from
// https://github.com/ostreedev/ostree/blob/main/src/ostree/ot-builtin-log.c#L40
gboolean log_commit (OstreeRepo *repo, const gchar *checksum, gboolean is_recurse, GError **error) {
    GError *local_error = NULL;

    g_print("parsing commit, cs: %s\n", std::string(checksum));

    g_autoptr (GVariant) variant = NULL;
    if (!ostree_repo_load_variant (repo, OSTREE_OBJECT_TYPE_COMMIT, checksum, &variant, &local_error)) {
        if (is_recurse && g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
            g_print ("<< History beyond this commit not fetched >>\n");
            g_clear_error (&local_error);
            return true;
        } else {
            g_propagate_error (error, local_error);
            return false;
        }
    }

    // TODO replace with commit parsing
    //ot_dump_object (OSTREE_OBJECT_TYPE_COMMIT, checksum, variant, flags);
    dump_commit(variant);

    /* Get the parent of this commit */
    g_autofree char *parent = ostree_commit_get_parent (variant);
    if (parent && !log_commit (repo, parent, true, error))
        return false;
    
    return true;
}

auto OSTreeRepo::parseCommits(std::string branch) -> std::vector<Commit> {
    // TODO
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
    if (!ostree_repo_resolve_rev (repo, branch.c_str(), FALSE, &checksum, &error)) {
        return ret;
    }

    log_commit (repo, checksum, FALSE, &error);

    return ret;
}

auto OSTreeRepo::parseCommitsAllBranches() -> std::vector<Commit> {
    // TODO
    return std::vector<Commit>();
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

auto OSTreeRepo::getLogStringOfBranch(const std::string& branch) -> std::string {
    // TODO
    return "";
}

auto OSTreeRepo::setBranches(std::vector<std::string> branches) -> void {
    this->branches = std::move(branches); 
}
