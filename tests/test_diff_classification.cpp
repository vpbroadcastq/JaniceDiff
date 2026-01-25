#include <diff/alignment.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace bendiff::core::diff {

TEST(Classification, SizesMatchInput)
{
    const std::vector<std::string> left = {"a", "b"};
    const std::vector<std::string> right = {"a", "x", "b"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);

    EXPECT_EQ(LeftLineClassification(r).size(), left.size());
    EXPECT_EQ(RightLineClassification(r).size(), right.size());
}

TEST(Classification, InsertDoesNotMarkLeft)
{
    const std::vector<std::string> left = {"a"};
    const std::vector<std::string> right = {"a", "b"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);

    EXPECT_EQ(LeftLineClassification(r), (std::vector<LineOp>{LineOp::Equal}));
    EXPECT_EQ(RightLineClassification(r), (std::vector<LineOp>{LineOp::Equal, LineOp::Insert}));
}

TEST(Classification, DeleteDoesNotMarkRight)
{
    const std::vector<std::string> left = {"a", "b"};
    const std::vector<std::string> right = {"a"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);

    EXPECT_EQ(LeftLineClassification(r), (std::vector<LineOp>{LineOp::Equal, LineOp::Delete}));
    EXPECT_EQ(RightLineClassification(r), (std::vector<LineOp>{LineOp::Equal}));
}

TEST(Classification, ReplaceMarksDeleteAndInsert)
{
    const std::vector<std::string> left = {"a", "b", "c"};
    const std::vector<std::string> right = {"a", "x", "c"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);

    EXPECT_EQ(LeftLineClassification(r), (std::vector<LineOp>{LineOp::Equal, LineOp::Delete, LineOp::Equal}));
    EXPECT_EQ(RightLineClassification(r), (std::vector<LineOp>{LineOp::Equal, LineOp::Insert, LineOp::Equal}));
}

TEST(AlignedRows, BuildsUnifiedRowStream)
{
    const std::vector<std::string> left = {"a", "b", "c"};
    const std::vector<std::string> right = {"a", "x", "c"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);
    const auto rows = BuildAlignedRows(r);

    // Expect: Equal(a), Delete(b), Insert(x), Equal(c)
    ASSERT_EQ(rows.size(), 4u);

    EXPECT_EQ(rows[0].op, LineOp::Equal);
    EXPECT_TRUE(rows[0].left.has_value());
    EXPECT_TRUE(rows[0].right.has_value());

    EXPECT_EQ(rows[1].op, LineOp::Delete);
    EXPECT_TRUE(rows[1].left.has_value());
    EXPECT_FALSE(rows[1].right.has_value());

    EXPECT_EQ(rows[2].op, LineOp::Insert);
    EXPECT_FALSE(rows[2].left.has_value());
    EXPECT_TRUE(rows[2].right.has_value());

    EXPECT_EQ(rows[3].op, LineOp::Equal);
    EXPECT_TRUE(rows[3].left.has_value());
    EXPECT_TRUE(rows[3].right.has_value());
}

} // namespace bendiff::core::diff
