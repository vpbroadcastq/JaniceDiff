#include "repo_status.h"

#include <system_error>

namespace fs = std::filesystem;

namespace bendiff::core {

ProcessResult RunGitStatusPorcelainV1Z(fs::path repoRoot)
{
    std::error_code ec;
    const fs::path abs = fs::absolute(repoRoot, ec);
    if (!ec && !abs.empty()) {
        repoRoot = abs;
    }

    // v1: capture stdout/stderr for later parsing and diagnostics.
    return RunProcess({"git", "status", "--porcelain=v1", "-z"}, repoRoot);
}

RepoStatus GetRepoStatus(fs::path repoRoot)
{
    std::error_code ec;
    const fs::path abs = fs::absolute(repoRoot, ec);
    if (!ec && !abs.empty()) {
        repoRoot = abs;
    }

    (void)RunGitStatusPorcelainV1Z(repoRoot);

    RepoStatus status;
    status.repoRoot = repoRoot;

    // M2-T5 will populate status.files by parsing porcelain output.
    return status;
}

} // namespace bendiff::core
