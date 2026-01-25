#include <diff/whitespace.h>

#include <gtest/gtest.h>

namespace bendiff::core::diff {

TEST(WhitespaceKeys, ExactKeepsWhitespaceDifferences)
{
    EXPECT_NE(MakeComparisonKey("foo", WhitespaceMode::Exact),
              MakeComparisonKey("foo ", WhitespaceMode::Exact));
    EXPECT_NE(MakeComparisonKey("f o o", WhitespaceMode::Exact),
              MakeComparisonKey("foo", WhitespaceMode::Exact));
}

TEST(WhitespaceKeys, IgnoreTrailingStripsTrailingSpaceTabAndCarriageReturn)
{
    EXPECT_EQ(MakeComparisonKey("foo", WhitespaceMode::IgnoreTrailing),
              MakeComparisonKey("foo ", WhitespaceMode::IgnoreTrailing));

    EXPECT_EQ(MakeComparisonKey("foo", WhitespaceMode::IgnoreTrailing),
              MakeComparisonKey("foo\t", WhitespaceMode::IgnoreTrailing));

    EXPECT_EQ(MakeComparisonKey("foo", WhitespaceMode::IgnoreTrailing),
              MakeComparisonKey("foo\r", WhitespaceMode::IgnoreTrailing));

    // Internal whitespace is not removed.
    EXPECT_NE(MakeComparisonKey("f o o", WhitespaceMode::IgnoreTrailing),
              MakeComparisonKey("foo", WhitespaceMode::IgnoreTrailing));
}

TEST(WhitespaceKeys, IgnoreAllRemovesAllDefinedWhitespace)
{
    EXPECT_EQ(MakeComparisonKey("f o o", WhitespaceMode::IgnoreAll),
              MakeComparisonKey("foo", WhitespaceMode::IgnoreAll));

    EXPECT_EQ(MakeComparisonKey("f\to\to\r", WhitespaceMode::IgnoreAll),
              MakeComparisonKey("foo", WhitespaceMode::IgnoreAll));

    EXPECT_EQ(MakeComparisonKey("", WhitespaceMode::IgnoreAll), "");
}

} // namespace bendiff::core::diff
