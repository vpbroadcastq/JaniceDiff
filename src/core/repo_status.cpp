#include "repo_status.h"

#include "porcelain.h"

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

    return GetRepoStatusWithDiagnostics(repoRoot).status;
}

RepoStatusResult GetRepoStatusWithDiagnostics(fs::path repoRoot)
{
    std::error_code ec;
    const fs::path abs = fs::absolute(repoRoot, ec);
    if (!ec && !abs.empty()) {
        repoRoot = abs;
    }

    RepoStatusResult out;
    out.status.repoRoot = repoRoot;
    out.process = RunGitStatusPorcelainV1Z(repoRoot);

    if (out.process.exitCode == 0) {
        out.status.files = ParsePorcelainV1(out.process.stdoutText, /*nulSeparated=*/true);
    }

    return out;
}

} // namespace bendiff::core
