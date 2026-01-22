#pragma once

#include "model.h"
#include "process.h"

#include <filesystem>

namespace bendiff::core {

struct RepoStatusResult {
	RepoStatus status;
	ProcessResult process;
};

// Runs:
//   git status --porcelain=v1 -z
// in the given repoRoot (as working directory).
//
// Notes:
// - This does not parse porcelain output (that's M2-T5).
// - On failure, exitCode will be non-zero and stderrText may contain diagnostics.
ProcessResult RunGitStatusPorcelainV1Z(std::filesystem::path repoRoot);

// Runs git status and (if successful) parses porcelain output into RepoStatus.
// Always returns the underlying process result for diagnostics.
RepoStatusResult GetRepoStatusWithDiagnostics(std::filesystem::path repoRoot);

// Retrieves the repo status model (RepoStatus + list of ChangedFile).
//
// M2-T4 implements only invocation of git status; parsing is intentionally deferred.
// Therefore, `files` will be empty until M2-T5 is implemented.
RepoStatus GetRepoStatus(std::filesystem::path repoRoot);

} // namespace bendiff::core
