#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace bendiff::core::diff {

enum class WhitespaceMode {
    Exact,
    IgnoreTrailing,
    IgnoreAll,
};

enum class LineOp {
    Equal,
    Insert,
    Delete,
};

struct DiffLine {
    LineOp op = LineOp::Equal;

    // Indices refer to the original input vectors.
    // Convention:
    // - Insert: leftIndex == npos
    // - Delete: rightIndex == npos
    std::size_t leftIndex = 0;
    std::size_t rightIndex = 0;

    static constexpr std::size_t npos = static_cast<std::size_t>(-1);
};

struct DiffHunk {
    std::size_t leftStart = 0;
    std::size_t leftCount = 0;
    std::size_t rightStart = 0;
    std::size_t rightCount = 0;

    std::vector<DiffLine> lines;
};

struct DiffResult {
    WhitespaceMode mode = WhitespaceMode::Exact;
    std::vector<DiffHunk> hunks;
    std::size_t leftLineCount = 0;
    std::size_t rightLineCount = 0;
};

// v1 line-diff entry point (algorithm implemented in Milestone 5).
DiffResult DiffLines(std::span<const std::string> left,
                     std::span<const std::string> right,
                     WhitespaceMode mode);

} // namespace bendiff::core::diff
