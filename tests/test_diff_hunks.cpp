#include <diff/diff.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace bendiff::core::diff {

TEST(Hunks, SingleIsolatedInsertionProducesOneHunk)
{
    const std::vector<std::string> left = {"a", "b", "c"};
    const std::vector<std::string> right = {"a", "b", "X", "c"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);
    ASSERT_EQ(r.hunks.size(), 1u);

    const auto& h = r.hunks[0];
    EXPECT_EQ(h.leftStart, 2u);
    EXPECT_EQ(h.leftCount, 0u);
    EXPECT_EQ(h.rightStart, 2u);
    EXPECT_EQ(h.rightCount, 1u);

    ASSERT_EQ(h.lines.size(), 1u);
    EXPECT_EQ(h.lines[0].op, LineOp::Insert);
    EXPECT_EQ(h.lines[0].rightIndex, 2u);
    EXPECT_EQ(h.lines[0].leftIndex, DiffLine::npos);
}

TEST(Hunks, TwoSeparatedChangeRegionsProduceTwoHunks)
{
    const std::vector<std::string> left = {"a", "b", "c", "d", "e"};
    const std::vector<std::string> right = {"a", "X", "c", "d", "Y"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);
    ASSERT_EQ(r.hunks.size(), 2u);

    EXPECT_EQ(r.hunks[0].leftStart, 1u);
    EXPECT_EQ(r.hunks[0].rightStart, 1u);
    EXPECT_EQ(r.hunks[1].leftStart, 4u);
    EXPECT_EQ(r.hunks[1].rightStart, 4u);

    // Each region is a single replace -> Delete+Insert (with no Equal lines in the hunk).
    ASSERT_EQ(r.hunks[0].lines.size(), 2u);
    EXPECT_EQ(r.hunks[0].lines[0].op, LineOp::Delete);
    EXPECT_EQ(r.hunks[0].lines[1].op, LineOp::Insert);

    ASSERT_EQ(r.hunks[1].lines.size(), 2u);
    EXPECT_EQ(r.hunks[1].lines[0].op, LineOp::Delete);
    EXPECT_EQ(r.hunks[1].lines[1].op, LineOp::Insert);
}

} // namespace bendiff::core::diff
