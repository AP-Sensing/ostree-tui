#include "cpplibostree.hpp"

// C++
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
// C
#include <cstdio>
#include <fcntl.h>
#include <glib-2.0/glib.h>
#include <ostree.h>
#include <assert.h>

namespace cpplibostree {

    OSTreeRepo::OSTreeRepo(std::string path):
            repo_path(std::move(path)), 
            commit_list({}),
            branches({}) {
        updateData();
    }

    bool OSTreeRepo::updateData() {
        // parse branches
        std::string branchString = getBranchesAsString();
        std::stringstream bss(branchString);
        std::string word;
        while (bss >> word) {
            branches.push_back(word);
        }

        // parse commits
        commit_list = parseCommitsAllBranches();

        return true;
    }

    // METHODS

    OstreeRepo* OSTreeRepo::_c() {
        // open repo
        GError *error{nullptr};
        OstreeRepo *repo = ostree_repo_open_at(AT_FDCWD, repo_path.c_str(), nullptr, &error);
        if (repo == nullptr) {
            g_printerr("Error opening repository: %s\n", error->message);
            g_error_free(error);
            return nullptr;
        }
        return repo;
    }

    std::string OSTreeRepo::getRepoPath() {
        return repo_path;
    }

    CommitList OSTreeRepo::getCommitList() {
        return commit_list;
    }

    std::vector<std::string> OSTreeRepo::getBranches() {
        return branches;
    }

    bool OSTreeRepo::isCommitSigned(const Commit& commit) {
        return commit.signatures.size() > 0;
    }

    Commit OSTreeRepo::parseCommit(GVariant *variant, const std::string& branch, const std::string& hash) {
        Commit commit;

        const gchar *subject    {nullptr};
        const gchar *body       {nullptr};
        guint64 timestamp       {0};
        g_autofree char *parent {nullptr};

        // see OSTREE_COMMIT_GVARIANT_FORMAT
        g_variant_get(variant, "(a{sv}aya(say)&s&stayay)", nullptr, nullptr, nullptr, &subject, &body, &timestamp, nullptr, nullptr);
        assert(body);
        assert(timestamp);

        timestamp = GUINT64_FROM_BE(timestamp);
        commit.timestamp = Timepoint(std::chrono::seconds(timestamp));

        parent = ostree_commit_get_parent(variant);
        if (parent) {
            commit.parent = parent;
        } else {
            commit.parent = "(no parent)";
        }

        g_autofree char *contents = ostree_commit_get_content_checksum(variant);
        assert(contents);
        commit.contentChecksum = contents;

        if (subject[0]) {
            std::string val = subject;
            commit.subject = val;
        } else {
            commit.subject = "(no subject)";
        }

        if (body[0]) {
            commit.body = body;
        }

        commit.branches.push_back(branch);
        commit.hash = hash;

        // Signatures ___ refactor into own method
        // open repo
        GError *error = nullptr;
        OstreeRepo *repo = ostree_repo_open_at(AT_FDCWD, repo_path.c_str(), nullptr, &error);
        if (repo == nullptr) {
            g_printerr("Error opening repository: %s\n", error->message);
            g_error_free(error);
            assert(repo);
        }
        // see ostree print_object for reference
        g_autoptr(OstreeGpgVerifyResult) result = nullptr;
        g_autoptr(GError) local_error = nullptr;
        result = ostree_repo_verify_commit_ext (repo, commit.parent.c_str(), nullptr, nullptr, nullptr,
                                                      &local_error);
        if (g_error_matches (local_error, OSTREE_GPG_ERROR, OSTREE_GPG_ERROR_NO_SIGNATURE) || local_error != nullptr) {
            /* Ignore */
        } else {
            assert(result);
            guint n_sigs = ostree_gpg_verify_result_count_all (result);
            // parse all found signatures
            for (guint ii = 0; ii < n_sigs; ii++) {
                g_autoptr(GVariant) variant = nullptr;
                variant = ostree_gpg_verify_result_get_all (result, ii);
                // see ostree_gpg_verify_result_describe_variant for reference
                gint64 timestamp                {0};
                gint64 exp_timestamp            {0};
                gint64 key_exp_timestamp        {0};
                gint64 key_exp_timestamp_primary{0};
                const char *fingerprint         {nullptr};
                const char *fingerprint_primary {nullptr};
                const char *pubkey_algo         {nullptr};
                const char *user_name           {nullptr};
                const char *user_email          {nullptr};
                gboolean valid                  {false};
                gboolean sig_expired            {false};
                gboolean key_expired            {false};
                gboolean key_revoked            {false};
                gboolean key_missing            {false};

                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_VALID, "b", &valid);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_SIG_EXPIRED, "b", &sig_expired);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_KEY_EXPIRED, "b", &key_expired);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_KEY_REVOKED, "b", &key_revoked);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_KEY_MISSING, "b", &key_missing);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_FINGERPRINT, "&s", &fingerprint);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_FINGERPRINT_PRIMARY, "&s", &fingerprint_primary);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_TIMESTAMP, "x", &timestamp);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_EXP_TIMESTAMP, "x", &exp_timestamp);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_PUBKEY_ALGO_NAME, "&s", &pubkey_algo);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_USER_NAME, "&s", &user_name);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_USER_EMAIL, "&s", &user_email);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_KEY_EXP_TIMESTAMP, "x", &key_exp_timestamp);
                g_variant_get_child (variant, OSTREE_GPG_SIGNATURE_ATTR_KEY_EXP_TIMESTAMP_PRIMARY, "x", &key_exp_timestamp_primary);
                
                // create signature struct
                Signature sig;

                sig.valid = valid;
                sig.sig_expired = sig_expired;
                sig.key_expired = key_expired;
                sig.key_revoked = key_revoked;
                sig.key_missing = key_missing;
                sig.fingerprint = fingerprint;
                sig.fingerprint_primary = fingerprint_primary;
                sig.timestamp = Timepoint(std::chrono::seconds(timestamp));
                sig.expire_timestamp = Timepoint(std::chrono::seconds(exp_timestamp));
                sig.pubkey_algorithm = pubkey_algo;
                sig.username = user_name;
                sig.usermail = user_email;
                sig.key_expire_timestamp = Timepoint(std::chrono::seconds(key_exp_timestamp));
                sig.key_expire_timestamp_primary = Timepoint(std::chrono::seconds(key_exp_timestamp_primary));

                commit.signatures.push_back(std::move(sig));
            }
        }

        return commit;
    }

    // modified log_commit() from https://github.com/ostreedev/ostree/blob/main/src/ostree/ot-builtin-log.c#L40
    gboolean OSTreeRepo::parseCommitsRecursive (OstreeRepo *repo, const gchar *checksum, GError **error,
                    CommitList *commit_list, const std::string& branch, gboolean is_recurse) {
        GError *local_error{nullptr};

        g_autoptr (GVariant) variant = nullptr;
        if (!ostree_repo_load_variant(repo, OSTREE_OBJECT_TYPE_COMMIT, checksum, &variant, &local_error)) {
            return is_recurse && g_error_matches(local_error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND);
        }

        commit_list->insert({
            static_cast<std::string>(checksum),
            parseCommit(variant, branch, static_cast<std::string>(checksum))
        });

        // parent recursion
        g_autofree char *parent = ostree_commit_get_parent(variant);

        return !(parent && !parseCommitsRecursive(repo, parent, error, commit_list, branch, true));
    }

    CommitList OSTreeRepo::parseCommitsOfBranch(const std::string& branch) {
        auto ret = CommitList();

        // open repo
        GError *error = nullptr;
        OstreeRepo *repo = ostree_repo_open_at(AT_FDCWD, repo_path.c_str(), nullptr, &error);
        if (repo == nullptr) {
            g_printerr("Error opening repository: %s\n", error->message);
            g_error_free(error);
            return ret;
        }

        // recursive commit log
        g_autofree char *checksum = nullptr;
        if (!ostree_repo_resolve_rev(repo, branch.c_str(), false, &checksum, &error)) {
            return ret;
        }

        parseCommitsRecursive(repo, checksum, &error, &ret, branch);

        return ret;
    }

    CommitList OSTreeRepo::parseCommitsAllBranches() {

        std::istringstream branches_string(getBranchesAsString());
        std::string branch;

        CommitList commits_all_branches;

        while (branches_string >> branch) {
            auto commits = parseCommitsOfBranch(branch);
            commits_all_branches.insert(commits.begin(), commits.end());
        }

        return commits_all_branches;
    }

    std::string OSTreeRepo::getBranchesAsString() {
        std::string branches_str;

        // open repo
        GError *error {nullptr};
        OstreeRepo *repo = ostree_repo_open_at(AT_FDCWD, repo_path.c_str(), nullptr, &error);

        if (repo == nullptr) {
            g_printerr("Error opening repository: %s\n", error->message);
            g_error_free(error);
            return "";
        }

        // get a list of refs
        GHashTable *refs_hash {nullptr};
        gboolean result = ostree_repo_list_refs_ext(repo, nullptr, &refs_hash, OSTREE_REPO_LIST_REFS_EXT_NONE, nullptr, &error);
        if (!result) {
            g_printerr("Error listing refs: %s\n", error->message);
            g_error_free(error);
            g_object_unref(repo);
            // TODO exit with error
            return "";
        }

        // iterate through the refs
        GHashTableIter iter;
        gpointer key {nullptr};
        gpointer value {nullptr};
        g_hash_table_iter_init(&iter, refs_hash);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
            const gchar *ref_name = static_cast<const gchar *>(key);
            branches_str += " " + std::string(ref_name);
        }

        // free
        g_hash_table_unref(refs_hash);

        return branches_str;
    }

    /// TODO This implementation should not rely on the ostree CLI -> change to libostree usage.
    bool OSTreeRepo::promoteCommit(const std::string& hash, const std::string& newRef,
                                   const std::vector<std::string> addMetadataStrings,
                                   const std::string& newSubject, bool keepMetadata) {
        if (hash.size() <= 0 || newRef.size() <= 0) {
            return false;
        }
        
        std::string command = "ostree commit";
        command += " --repo=" + repo_path;
        command += " -b " + newRef;
        command += (newSubject.size() <= 0
                            ? ""
                            : " -s \"" + newSubject + "\"");
        command += (keepMetadata
                            ? ""
                            : " --keep-metadata");
        for (auto str : addMetadataStrings) {
            command += "--add-metadata-string=\"" + str + "\"";
        }
        command += " --tree=ref=" + hash;
        
        // run as CLI command
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        
        if (!pipe) {
            return false;
        }
        return true;
    }

} // namespace cpplibostree
