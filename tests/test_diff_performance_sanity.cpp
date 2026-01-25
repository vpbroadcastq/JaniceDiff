#include <diff/alignment.h>
#include <diff/diff.h>

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace bendiff::core::diff {

TEST(PerformanceSanity, ModerateInputCompletesAndShapesAreConsistent)
{
    // Goal: ensure the algorithm doesn't blow up (OOM/hang) on moderately sized inputs.
    // No strict timing assertions (to keep the test stable across machines/configs).

    constexpr std::size_t kLineCount = 10'000;

    std::vector<std::string> left;
    left.reserve(kLineCount);

    for (std::size_t i = 0; i < kLineCount; ++i) {
        // Introduce some repeated lines while still keeping most lines unique.
        if ((i % 100) == 0) {
            left.push_back("COMMON");
        } else {
            left.push_back("line " + std::to_string(i));
        }
    }

    // Create a right side with small edits:
    // - one insertion near the beginning
    // - one deletion in the middle
    // - a few replacements
    std::vector<std::string> right;
    right.reserve(kLineCount + 8);

    right.push_back("header insert");

    for (std::size_t i = 0; i < kLineCount; ++i) {
        if (i == 5'000) {
            // Delete this line from the right side.
            continue;
        }

        if (i == 1234 || i == 4321 || i == 8765) {
            right.push_back("replaced " + std::to_string(i));
            continue;
        }

        right.push_back(left[i]);

        if (i == 2500) {
            right.push_back("mid insert");
        }
    }

    const auto start = std::chrono::steady_clock::now();
    const auto r = DiffLines(left, right, WhitespaceMode::Exact);
    const auto end = std::chrono::steady_clock::now();

    // Basic shape invariants.
    EXPECT_EQ(r.leftLineCount, left.size());
    EXPECT_EQ(r.rightLineCount, right.size());
    EXPECT_EQ(r.mode, WhitespaceMode::Exact);

    const auto leftClass = LeftLineClassification(r);
    const auto rightClass = RightLineClassification(r);
    EXPECT_EQ(leftClass.size(), left.size());
    EXPECT_EQ(rightClass.size(), right.size());

    // There should be at least one change region.
    EXPECT_FALSE(r.hunks.empty());

    // Aligned rows should cover all lines.
    const auto rows = BuildAlignedRows(r);
    EXPECT_GE(rows.size(), std::max(left.size(), right.size()));

    // Debug-only breadcrumb (not a hard limit).
    (void)start;
    (void)end;
}

} // namespace bendiff::core::diff
