#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace bendiff::core {

// Recursively lists all regular files under `root`.
//
// - Returns relative paths using '/' separators (via generic_string()).
// - Includes all files (does not ignore .git or any other directories).
// - Uses error_code-based filesystem APIs to avoid throwing on permission errors.
std::vector<std::string> ListFilesRecursive(std::filesystem::path root);

} // namespace bendiff::core
