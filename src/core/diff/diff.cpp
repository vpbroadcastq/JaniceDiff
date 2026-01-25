#include <diff/diff.h>
#include <diff/whitespace.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace bendiff::core::diff {

namespace {

std::vector<std::string> BuildKeys(std::span<const std::string> lines, WhitespaceMode mode)
{
    std::vector<std::string> keys;
    keys.reserve(lines.size());
    for (const auto& line : lines) {
        keys.push_back(MakeComparisonKey(line, mode));
    }
    return keys;
}

std::vector<DiffLine> MyersDiffOps(const std::vector<std::string>& leftKeys, const std::vector<std::string>& rightKeys)
{
    using coord_t = std::ptrdiff_t;
    const coord_t n = static_cast<coord_t>(leftKeys.size());
    const coord_t m = static_cast<coord_t>(rightKeys.size());

    const coord_t maxD = n + m;
    const coord_t offset = maxD;

    std::vector<coord_t> v(static_cast<std::size_t>(2 * maxD + 1), -1);
    v[static_cast<std::size_t>(offset + 1)] = 0;
    std::vector<std::vector<coord_t>> trace;
    trace.reserve(static_cast<std::size_t>(maxD + 1));

    coord_t endX = 0;
    coord_t endY = 0;

    for (coord_t d = 0; d <= maxD; ++d) {
        for (coord_t k = -d; k <= d; k += 2) {
            coord_t x;
            const auto idx = static_cast<std::size_t>(offset + k);
            const auto idxKm1 = static_cast<std::size_t>(offset + (k - 1));
            const auto idxKp1 = static_cast<std::size_t>(offset + (k + 1));

            if (k == -d || (k != d && v[idxKm1] < v[idxKp1])) {
                // Down (insertion in left->right transform).
                x = v[idxKp1];
            } else {
                // Right (deletion in left->right transform).
                x = v[idxKm1] + 1;
            }

            coord_t y = x - k;
            while (x < n && y < m && leftKeys[static_cast<std::size_t>(x)] == rightKeys[static_cast<std::size_t>(y)]) {
                ++x;
                ++y;
            }
            v[idx] = x;

            if (x >= n && y >= m) {
                endX = x;
                endY = y;
                trace.push_back(v);
                goto done;
            }
        }

        trace.push_back(v);
    }

done:
    // Backtrack.
    coord_t x = endX;
    coord_t y = endY;
    std::vector<DiffLine> reversed;
    reversed.reserve(static_cast<std::size_t>(n + m));

    for (coord_t d = static_cast<coord_t>(trace.size()) - 1; d > 0; --d) {
        const auto& vPrev = trace[static_cast<std::size_t>(d - 1)];
        const coord_t k = x - y;

        coord_t prevK;
        const auto idxKm1 = static_cast<std::size_t>(offset + (k - 1));
        const auto idxKp1 = static_cast<std::size_t>(offset + (k + 1));

        if (k == -d || (k != d && vPrev[idxKm1] < vPrev[idxKp1])) {
            prevK = k + 1;
        } else {
            prevK = k - 1;
        }

        const coord_t prevX = vPrev[static_cast<std::size_t>(offset + prevK)];
        const coord_t prevY = prevX - prevK;

        while (x > prevX && y > prevY) {
            reversed.push_back(DiffLine{
                .op = LineOp::Equal,
                .leftIndex = static_cast<std::size_t>(x - 1),
                .rightIndex = static_cast<std::size_t>(y - 1),
            });
            --x;
            --y;
        }

        if (x == prevX) {
            // Insertion (right element was added).
            reversed.push_back(DiffLine{
                .op = LineOp::Insert,
                .leftIndex = DiffLine::npos,
                .rightIndex = static_cast<std::size_t>(y - 1),
            });
            --y;
        } else {
            // Deletion (left element was removed).
            reversed.push_back(DiffLine{
                .op = LineOp::Delete,
                .leftIndex = static_cast<std::size_t>(x - 1),
                .rightIndex = DiffLine::npos,
            });
            --x;
        }
    }

    while (x > 0 && y > 0) {
        reversed.push_back(DiffLine{
            .op = LineOp::Equal,
            .leftIndex = static_cast<std::size_t>(x - 1),
            .rightIndex = static_cast<std::size_t>(y - 1),
        });
        --x;
        --y;
    }
    while (x > 0) {
        reversed.push_back(DiffLine{
            .op = LineOp::Delete,
            .leftIndex = static_cast<std::size_t>(x - 1),
            .rightIndex = DiffLine::npos,
        });
        --x;
    }
    while (y > 0) {
        reversed.push_back(DiffLine{
            .op = LineOp::Insert,
            .leftIndex = DiffLine::npos,
            .rightIndex = static_cast<std::size_t>(y - 1),
        });
        --y;
    }

    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

std::vector<DiffHunk> BuildEditHunksZeroContext(const std::vector<DiffLine>& ops)
{
    std::vector<DiffHunk> hunks;

    std::size_t leftPos = 0;
    std::size_t rightPos = 0;

    auto advance_positions = [&](const DiffLine& dl) {
        switch (dl.op) {
        case LineOp::Equal:
            ++leftPos;
            ++rightPos;
            break;
        case LineOp::Delete:
            ++leftPos;
            break;
        case LineOp::Insert:
            ++rightPos;
            break;
        }
    };

    DiffHunk current;
    bool inHunk = false;

    auto flush = [&]() {
        if (!inHunk) {
            return;
        }
        hunks.push_back(std::move(current));
        current = DiffHunk{};
        inHunk = false;
    };

    for (const auto& dl : ops) {
        if (dl.op == LineOp::Equal) {
            flush();
            advance_positions(dl);
            continue;
        }

        if (!inHunk) {
            inHunk = true;
            current.leftStart = leftPos;
            current.rightStart = rightPos;
            current.leftCount = 0;
            current.rightCount = 0;
        }

        current.lines.push_back(dl);
        if (dl.op == LineOp::Delete) {
            ++current.leftCount;
        } else if (dl.op == LineOp::Insert) {
            ++current.rightCount;
        }
        advance_positions(dl);
    }

    flush();
    return hunks;
}

} // namespace

DiffResult DiffLines(std::span<const std::string> left,
                     std::span<const std::string> right,
                     WhitespaceMode mode)
{
    DiffResult r;
    r.mode = mode;
    r.leftLineCount = left.size();
    r.rightLineCount = right.size();

    const auto leftKeys = BuildKeys(left, mode);
    const auto rightKeys = BuildKeys(right, mode);
    auto ops = MyersDiffOps(leftKeys, rightKeys);

    // M5-T3: produce a deterministic edit script.
    // M5-T4: split into contiguous edit hunks (no context in v1).
    r.hunks = BuildEditHunksZeroContext(ops);
    return r;
}

} // namespace bendiff::core::diff
