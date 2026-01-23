#pragma once

#include <filesystem>

namespace bendiff::core {

enum class FileCompareResult {
    Same,
    Different,
    Unreadable,
};

// Compares two files for bytewise equality.
//
// - If either file cannot be opened/read => Unreadable
// - If sizes differ => Different
// - Otherwise compares contents in chunks => Same/Different
FileCompareResult CompareFilesBytewise(const std::filesystem::path& left,
                                      const std::filesystem::path& right);

} // namespace bendiff::core
