#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace bendiff::core {

enum class ChangeKind {
    Modified,
    Added,
    Deleted,
    Renamed,
    Unmerged,
    Unknown,
};

struct ChangedFile {
    std::string repoRelativePath;
    ChangeKind kind = ChangeKind::Unknown;
    std::optional<std::string> renameFrom;
};

struct RepoStatus {
    std::filesystem::path repoRoot;
    std::vector<ChangedFile> files;
};

} // namespace bendiff::core
