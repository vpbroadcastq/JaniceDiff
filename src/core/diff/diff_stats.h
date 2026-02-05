#pragma once

#include <cstddef>

#include <diff/diff.h>

namespace bendiff::core::diff {

struct DiffStats {
    std::size_t hunkCount = 0;
    std::size_t addedLineCount = 0;
    std::size_t deletedLineCount = 0;
};

// Computes lightweight diff statistics from an already-computed DiffResult.
// This is intended for UI display (status bar) and should not trigger any
// expensive recomputation.
DiffStats ComputeDiffStats(const DiffResult& d);

} // namespace bendiff::core::diff
