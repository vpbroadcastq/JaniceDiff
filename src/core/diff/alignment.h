#pragma once

#include <diff/diff.h>

#include <optional>
#include <vector>

namespace bendiff::core::diff {

struct AlignedRow {
    std::optional<std::size_t> left;
    std::optional<std::size_t> right;
    LineOp op = LineOp::Equal;
};

// Builds a unified row stream suitable for side-by-side rendering.
//
// For Equal rows: both indices are present.
// For Delete rows: left is present; right is nullopt.
// For Insert rows: right is present; left is nullopt.
std::vector<AlignedRow> BuildAlignedRows(const DiffResult& r);

// Convenience: per-line classification vectors.
std::vector<LineOp> LeftLineClassification(const DiffResult& r);
std::vector<LineOp> RightLineClassification(const DiffResult& r);

} // namespace bendiff::core::diff
