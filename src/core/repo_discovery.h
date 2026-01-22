#pragma once

#include <filesystem>
#include <optional>

namespace bendiff::core {

// Walks up from startDir (inclusive) looking for a .git directory.
// v1 constraint: worktrees not supported; ".git" must be a directory.
std::optional<std::filesystem::path> FindGitRepoRoot(std::filesystem::path startDir);

} // namespace bendiff::core
