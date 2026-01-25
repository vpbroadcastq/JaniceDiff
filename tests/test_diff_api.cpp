#include <diff/diff.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace bendiff::core::diff {

TEST(DiffApi, CanCallDiffLinesAndGetCounts)
{
    const std::vector<std::string> left = {"a", "b"};
    const std::vector<std::string> right = {"a", "c", "d"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);

    EXPECT_EQ(r.mode, WhitespaceMode::Exact);
    EXPECT_EQ(r.leftLineCount, left.size());
    EXPECT_EQ(r.rightLineCount, right.size());
}

} // namespace bendiff::core::diff
