#include <navigation/change_navigation.h>

#include <diff/diff.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace bendiff::core::navigation {

TEST(ChangeNavigation, EnumerateChangeHunksMatchesHunkStarts)
{
    // Two separated deletes produce two hunks.
    const std::vector<std::string> left = {"a", "X", "b", "Y", "c"};
    const std::vector<std::string> right = {"a", "b", "c"};

    const auto d = bendiff::core::diff::DiffLines(left, right, bendiff::core::diff::WhitespaceMode::Exact);
    ASSERT_EQ(d.hunks.size(), 2u);

    const auto changes = EnumerateChangeHunks(d);
    ASSERT_EQ(changes.size(), 2u);

    EXPECT_EQ(changes[0].hunkIndex, 0u);
    EXPECT_EQ(changes[0].leftStart, 1u);
    EXPECT_EQ(changes[0].leftCount, 1u);
    EXPECT_EQ(changes[0].rightStart, 1u);
    EXPECT_EQ(changes[0].rightCount, 0u);

    EXPECT_EQ(changes[1].hunkIndex, 1u);
    EXPECT_EQ(changes[1].leftStart, 3u);
    EXPECT_EQ(changes[1].leftCount, 1u);
    EXPECT_EQ(changes[1].rightStart, 2u);
    EXPECT_EQ(changes[1].rightCount, 0u);
}

TEST(ChangeNavigation, NextPrevBehaveOnEdges)
{
    // Construct a tiny change list without relying on diff behavior.
    const std::vector<ChangeLocation> changes = {
        ChangeLocation{.hunkIndex = 0},
        ChangeLocation{.hunkIndex = 1},
        ChangeLocation{.hunkIndex = 2},
    };

    EXPECT_EQ(NextChangeIndex(std::nullopt, changes), 0u);
    EXPECT_EQ(PrevChangeIndex(std::nullopt, changes), 2u);

    EXPECT_EQ(NextChangeIndex(0u, changes), 1u);
    EXPECT_EQ(PrevChangeIndex(0u, changes), std::nullopt);

    EXPECT_EQ(NextChangeIndex(2u, changes), std::nullopt);
    EXPECT_EQ(PrevChangeIndex(2u, changes), 1u);

    // Out-of-range current index is treated like "no current".
    EXPECT_EQ(NextChangeIndex(999u, changes), 0u);
    EXPECT_EQ(PrevChangeIndex(999u, changes), 2u);
}

TEST(ChangeNavigation, EmptyChangesReturnNullopt)
{
    const std::vector<ChangeLocation> empty;
    EXPECT_EQ(NextChangeIndex(std::nullopt, empty), std::nullopt);
    EXPECT_EQ(PrevChangeIndex(std::nullopt, empty), std::nullopt);
}

} // namespace bendiff::core::navigation
