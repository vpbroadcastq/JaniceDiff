#pragma once

#include "dir_diff_model.h"

#include <filesystem>

namespace bendiff::core {

// Diffs two directory trees.
//
// v1 contract:
// - Walk both roots recursively (files only)
// - Build union set of relative file paths
// - Classify LeftOnly/RightOnly/Same/Different/Unreadable
// - Do not ignore special directories like .git
// - Return stable ordering (lexicographic by relativePath)
DirDiffResult DiffDirectories(const std::filesystem::path& leftRoot,
                             const std::filesystem::path& rightRoot);

} // namespace bendiff::core
