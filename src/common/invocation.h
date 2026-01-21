#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace bendiff {

enum class AppMode {
    RepoMode,
    FolderDiffMode,
    Invalid,
};

struct Invocation {
    AppMode mode = AppMode::Invalid;

    // Repo mode
    std::filesystem::path repoPath;

    // Folder diff mode
    std::filesystem::path leftPath;
    std::filesystem::path rightPath;

    // Invalid mode details (human-readable)
    std::string error;
};

// Parses application arguments (excluding argv[0]) and classifies the invocation.
//
// Contract (v1):
// - 0 args  -> RepoMode, using CWD
// - 1 arg   -> RepoMode, using provided path (basic existence check)
// - 2 args  -> FolderDiffMode, using provided paths (basic existence check)
// - else    -> Invalid
Invocation parse_invocation(const std::vector<std::string>& args);

const char* to_string(AppMode mode);

} // namespace bendiff
