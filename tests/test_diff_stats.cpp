#include <gtest/gtest.h>

#include <diff/diff.h>
#include <diff/diff_stats.h>

namespace {

using bendiff::core::diff::ComputeDiffStats;
using bendiff::core::diff::DiffLines;
using bendiff::core::diff::WhitespaceMode;

TEST(DiffStats, EqualInputsAreZero)
{
    const std::vector<std::string> left = {"a", "b"};
    const std::vector<std::string> right = {"a", "b"};

    const auto d = DiffLines(left, right, WhitespaceMode::Exact);
    const auto s = ComputeDiffStats(d);

    EXPECT_EQ(s.hunkCount, 0u);
    EXPECT_EQ(s.addedLineCount, 0u);
    EXPECT_EQ(s.deletedLineCount, 0u);
}

TEST(DiffStats, ReplaceIsDeletePlusInsert)
{
    const std::vector<std::string> left = {"a"};
    const std::vector<std::string> right = {"b"};

    const auto d = DiffLines(left, right, WhitespaceMode::Exact);
    const auto s = ComputeDiffStats(d);

    EXPECT_EQ(s.hunkCount, 1u);
    EXPECT_EQ(s.addedLineCount, 1u);
    EXPECT_EQ(s.deletedLineCount, 1u);
}

TEST(DiffStats, InsertAndDeleteCountedAcrossHunks)
{
    // Two separated change regions: insertion near start and deletion near end.
    const std::vector<std::string> left = {"a", "b", "c", "d", "e", "f"};
    const std::vector<std::string> right = {"a", "X", "b", "c", "d", "e"};

    const auto d = DiffLines(left, right, WhitespaceMode::Exact);
    const auto s = ComputeDiffStats(d);

    EXPECT_EQ(s.hunkCount, 2u);
    EXPECT_EQ(s.addedLineCount, 1u);
    EXPECT_EQ(s.deletedLineCount, 1u);
}

} // namespace
