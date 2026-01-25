#include <diff/alignment.h>

#include <cassert>

namespace bendiff::core::diff {

namespace {

inline void apply_op_positions(LineOp op, std::size_t& leftPos, std::size_t& rightPos)
{
    switch (op) {
    case LineOp::Equal:
        ++leftPos;
        ++rightPos;
        return;
    case LineOp::Delete:
        ++leftPos;
        return;
    case LineOp::Insert:
        ++rightPos;
        return;
    }
}

} // namespace

std::vector<AlignedRow> BuildAlignedRows(const DiffResult& r)
{
    std::vector<AlignedRow> rows;
    rows.reserve(r.leftLineCount + r.rightLineCount);

    std::size_t leftPos = 0;
    std::size_t rightPos = 0;

    auto emit_equals = [&](std::size_t count) {
        for (std::size_t i = 0; i < count; ++i) {
            rows.push_back(AlignedRow{
                .left = leftPos,
                .right = rightPos,
                .op = LineOp::Equal,
            });
            ++leftPos;
            ++rightPos;
        }
    };

    for (const auto& h : r.hunks) {
        // Emit equal run before the hunk (if any).
        assert(h.leftStart >= leftPos);
        assert(h.rightStart >= rightPos);
        const std::size_t gapLeft = h.leftStart - leftPos;
        const std::size_t gapRight = h.rightStart - rightPos;
        assert(gapLeft == gapRight);
        emit_equals(gapLeft);

        // Emit edit rows.
        for (const auto& dl : h.lines) {
            if (dl.op == LineOp::Delete) {
                rows.push_back(AlignedRow{
                    .left = dl.leftIndex,
                    .right = std::nullopt,
                    .op = LineOp::Delete,
                });
            } else if (dl.op == LineOp::Insert) {
                rows.push_back(AlignedRow{
                    .left = std::nullopt,
                    .right = dl.rightIndex,
                    .op = LineOp::Insert,
                });
            } else {
                // Defensive: equality lines can exist if hunks include context.
                rows.push_back(AlignedRow{
                    .left = dl.leftIndex,
                    .right = dl.rightIndex,
                    .op = LineOp::Equal,
                });
            }

            apply_op_positions(dl.op, leftPos, rightPos);
        }

        // Sanity-check computed end positions.
        assert(leftPos == h.leftStart + h.leftCount);
        assert(rightPos == h.rightStart + h.rightCount);
    }

    // Emit trailing equal run.
    assert(r.leftLineCount >= leftPos);
    assert(r.rightLineCount >= rightPos);
    const std::size_t tailLeft = r.leftLineCount - leftPos;
    const std::size_t tailRight = r.rightLineCount - rightPos;
    assert(tailLeft == tailRight);
    emit_equals(tailLeft);

    return rows;
}

std::vector<LineOp> LeftLineClassification(const DiffResult& r)
{
    std::vector<LineOp> out(r.leftLineCount, LineOp::Equal);
    for (const auto& h : r.hunks) {
        for (const auto& dl : h.lines) {
            if (dl.op == LineOp::Delete && dl.leftIndex != DiffLine::npos) {
                out[dl.leftIndex] = LineOp::Delete;
            }
        }
    }
    return out;
}

std::vector<LineOp> RightLineClassification(const DiffResult& r)
{
    std::vector<LineOp> out(r.rightLineCount, LineOp::Equal);
    for (const auto& h : r.hunks) {
        for (const auto& dl : h.lines) {
            if (dl.op == LineOp::Insert && dl.rightIndex != DiffLine::npos) {
                out[dl.rightIndex] = LineOp::Insert;
            }
        }
    }
    return out;
}

} // namespace bendiff::core::diff
