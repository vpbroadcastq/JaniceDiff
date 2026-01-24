#include <gtest/gtest.h>

#include <loaded_text_file.h>

namespace bendiff::core {

TEST(LoadedTextFile, CanConstruct)
{
    LoadedTextFile f;
    f.absolutePath = "/tmp/example.txt";
    f.status = LoadStatus::Ok;
    f.lines = {"a", "b"};
    f.hadFinalNewline = true;

    EXPECT_EQ(f.status, LoadStatus::Ok);
    ASSERT_EQ(f.lines.size(), 2u);
    EXPECT_EQ(f.lines[0], "a");
    EXPECT_EQ(f.lines[1], "b");
    EXPECT_TRUE(f.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, EmptyIsNoLines)
{
    const auto r = SplitLinesNormalizeNewlines("");
    EXPECT_TRUE(r.lines.empty());
    EXPECT_FALSE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, NoNewlineSingleLine)
{
    const auto r = SplitLinesNormalizeNewlines("abc");
    ASSERT_EQ(r.lines.size(), 1u);
    EXPECT_EQ(r.lines[0], "abc");
    EXPECT_FALSE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, LfSplitting)
{
    const auto r = SplitLinesNormalizeNewlines("a\nb\nc");
    ASSERT_EQ(r.lines.size(), 3u);
    EXPECT_EQ(r.lines[0], "a");
    EXPECT_EQ(r.lines[1], "b");
    EXPECT_EQ(r.lines[2], "c");
    EXPECT_FALSE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, FinalLfPreservedFlag)
{
    const auto r = SplitLinesNormalizeNewlines("a\nb\nc\n");
    ASSERT_EQ(r.lines.size(), 3u);
    EXPECT_EQ(r.lines[2], "c");
    EXPECT_TRUE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, LoneLfIsOneEmptyLine)
{
    const auto r = SplitLinesNormalizeNewlines("\n");
    ASSERT_EQ(r.lines.size(), 1u);
    EXPECT_EQ(r.lines[0], "");
    EXPECT_TRUE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, TwoLfsAreTwoEmptyLines)
{
    const auto r = SplitLinesNormalizeNewlines("\n\n");
    ASSERT_EQ(r.lines.size(), 2u);
    EXPECT_EQ(r.lines[0], "");
    EXPECT_EQ(r.lines[1], "");
    EXPECT_TRUE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, CrLfSplitting)
{
    const auto r = SplitLinesNormalizeNewlines("a\r\nb\r\n");
    ASSERT_EQ(r.lines.size(), 2u);
    EXPECT_EQ(r.lines[0], "a");
    EXPECT_EQ(r.lines[1], "b");
    EXPECT_TRUE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, LoneCrSplitting)
{
    const auto r = SplitLinesNormalizeNewlines("a\rb\r");
    ASSERT_EQ(r.lines.size(), 2u);
    EXPECT_EQ(r.lines[0], "a");
    EXPECT_EQ(r.lines[1], "b");
    EXPECT_TRUE(r.hadFinalNewline);
}

TEST(SplitLinesNormalizeNewlines, MixedEndings)
{
    const auto r = SplitLinesNormalizeNewlines("a\r\nb\nc\rd");
    ASSERT_EQ(r.lines.size(), 4u);
    EXPECT_EQ(r.lines[0], "a");
    EXPECT_EQ(r.lines[1], "b");
    EXPECT_EQ(r.lines[2], "c");
    EXPECT_EQ(r.lines[3], "d");
    EXPECT_FALSE(r.hadFinalNewline);
}

} // namespace bendiff::core
