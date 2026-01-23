#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace bendiff::core {

enum class DirEntryStatus {
    Same,
    Different,
    LeftOnly,
    RightOnly,
    Unreadable,
};

struct DirEntry {
    std::string relativePath;
    DirEntryStatus status = DirEntryStatus::Same;
    bool isBinaryHint = false;
};

struct DirDiffResult {
    std::filesystem::path leftRoot;
    std::filesystem::path rightRoot;
    std::vector<DirEntry> entries;
};

} // namespace bendiff::core
