#pragma once

#include "model.h"

#include <string_view>
#include <vector>

namespace bendiff::core {

// Parses `git status --porcelain=v1` output.
//
// If nulSeparated is true, expects `-z` format (NUL-delimited records) which is
// recommended for robust parsing.
//
// v1 constraints:
// - Ignored entries are excluded ("!!" lines).
// - Submodules are ignored (heuristic: treat status code 'S' as submodule marker).
// - Renames populate `renameFrom` (and `repoRelativePath` is the new path).
std::vector<ChangedFile> ParsePorcelainV1(std::string_view text, bool nulSeparated);

} // namespace bendiff::core
