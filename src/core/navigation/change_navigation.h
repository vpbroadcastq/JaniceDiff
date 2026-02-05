#pragma once

#include <diff/diff.h>

#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace bendiff::core::navigation {

// Navigation unit (v1): diff hunks.
// Each ChangeLocation refers to a single DiffHunk in the DiffResult.
struct ChangeLocation {
    std::size_t hunkIndex = 0;

    // 0-based starts/counts (match DiffHunk fields).
    std::size_t leftStart = 0;
    std::size_t leftCount = 0;
    std::size_t rightStart = 0;
    std::size_t rightCount = 0;
};

std::vector<ChangeLocation> EnumerateChangeHunks(const diff::DiffResult& d);

// Returns the index into the `changes` vector.
std::optional<std::size_t> NextChangeIndex(std::optional<std::size_t> currentIndex,
                                          std::span<const ChangeLocation> changes);
std::optional<std::size_t> PrevChangeIndex(std::optional<std::size_t> currentIndex,
                                          std::span<const ChangeLocation> changes);

} // namespace bendiff::core::navigation
