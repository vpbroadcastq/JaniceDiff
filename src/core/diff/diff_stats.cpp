#include <diff/diff_stats.h>

namespace bendiff::core::diff {

DiffStats ComputeDiffStats(const DiffResult& d)
{
    DiffStats s;
    s.hunkCount = d.hunks.size();

    for (const auto& h : d.hunks) {
        for (const auto& dl : h.lines) {
            if (dl.op == LineOp::Insert) {
                ++s.addedLineCount;
            } else if (dl.op == LineOp::Delete) {
                ++s.deletedLineCount;
            }
        }
    }

    return s;
}

} // namespace bendiff::core::diff
