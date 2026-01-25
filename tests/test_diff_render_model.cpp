#include <diff/diff.h>
#include <render/diff_render_model.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace bendiff::core::render {

TEST(DiffRenderModel, SideBySideProducesSingleBothBlockWithAlignedRows)
{
    bendiff::core::LoadedTextFile left;
    left.status = bendiff::core::LoadStatus::Ok;
    left.lines = {"a", "b"};

    bendiff::core::LoadedTextFile right;
    right.status = bendiff::core::LoadStatus::Ok;
    right.lines = {"a", "X", "b"};

    const auto d = bendiff::core::diff::DiffLines(left.lines, right.lines, bendiff::core::diff::WhitespaceMode::Exact);
    const auto doc = BuildSideBySideRender(left, right, d);

    ASSERT_EQ(doc.blocks.size(), 1u);
    EXPECT_EQ(doc.blocks[0].side, RenderBlockSide::Both);

    const auto& lines = doc.blocks[0].lines;
    ASSERT_EQ(lines.size(), 3u);

    // a == a
    EXPECT_EQ(lines[0].op, bendiff::core::diff::LineOp::Equal);
    EXPECT_EQ(lines[0].leftLine, 1u);
    EXPECT_EQ(lines[0].rightLine, 1u);
    EXPECT_EQ(lines[0].leftText, "a");
    EXPECT_EQ(lines[0].rightText, "a");

    // insert X
    EXPECT_EQ(lines[1].op, bendiff::core::diff::LineOp::Insert);
    EXPECT_FALSE(lines[1].leftLine.has_value());
    EXPECT_EQ(lines[1].rightLine, 2u);
    EXPECT_EQ(lines[1].leftText, "");
    EXPECT_EQ(lines[1].rightText, "X");

    // b == b
    EXPECT_EQ(lines[2].op, bendiff::core::diff::LineOp::Equal);
    EXPECT_EQ(lines[2].leftLine, 2u);
    EXPECT_EQ(lines[2].rightLine, 3u);
    EXPECT_EQ(lines[2].leftText, "b");
    EXPECT_EQ(lines[2].rightText, "b");
}

TEST(DiffRenderModel, SideBySideDeleteShowsBlankRight)
{
    bendiff::core::LoadedTextFile left;
    left.status = bendiff::core::LoadStatus::Ok;
    left.lines = {"a", "X", "b"};

    bendiff::core::LoadedTextFile right;
    right.status = bendiff::core::LoadStatus::Ok;
    right.lines = {"a", "b"};

    const auto d = bendiff::core::diff::DiffLines(left.lines, right.lines, bendiff::core::diff::WhitespaceMode::Exact);
    const auto doc = BuildSideBySideRender(left, right, d);

    ASSERT_EQ(doc.blocks.size(), 1u);
    EXPECT_EQ(doc.blocks[0].side, RenderBlockSide::Both);

    const auto& lines = doc.blocks[0].lines;
    ASSERT_EQ(lines.size(), 3u);

    // a == a
    EXPECT_EQ(lines[0].op, bendiff::core::diff::LineOp::Equal);
    EXPECT_EQ(lines[0].leftLine, 1u);
    EXPECT_EQ(lines[0].rightLine, 1u);
    EXPECT_EQ(lines[0].leftText, "a");
    EXPECT_EQ(lines[0].rightText, "a");

    // delete X
    EXPECT_EQ(lines[1].op, bendiff::core::diff::LineOp::Delete);
    EXPECT_EQ(lines[1].leftLine, 2u);
    EXPECT_FALSE(lines[1].rightLine.has_value());
    EXPECT_EQ(lines[1].leftText, "X");
    EXPECT_EQ(lines[1].rightText, "");

    // b == b
    EXPECT_EQ(lines[2].op, bendiff::core::diff::LineOp::Equal);
    EXPECT_EQ(lines[2].leftLine, 3u);
    EXPECT_EQ(lines[2].rightLine, 2u);
    EXPECT_EQ(lines[2].leftText, "b");
    EXPECT_EQ(lines[2].rightText, "b");
}

TEST(DiffRenderModel, InlineReplaceProducesLeftBlockThenRightBlock)
{
    bendiff::core::LoadedTextFile left;
    left.status = bendiff::core::LoadStatus::Ok;
    left.lines = {"a", "b", "c"};

    bendiff::core::LoadedTextFile right;
    right.status = bendiff::core::LoadStatus::Ok;
    right.lines = {"a", "B", "c"};

    const auto d = bendiff::core::diff::DiffLines(left.lines, right.lines, bendiff::core::diff::WhitespaceMode::Exact);
    const auto doc = BuildInlineRender(left, right, d);

    ASSERT_EQ(doc.blocks.size(), 4u);
    EXPECT_EQ(doc.blocks[0].side, RenderBlockSide::Both);
    EXPECT_EQ(doc.blocks[1].side, RenderBlockSide::Left);
    EXPECT_EQ(doc.blocks[2].side, RenderBlockSide::Right);
    EXPECT_EQ(doc.blocks[3].side, RenderBlockSide::Both);

    ASSERT_EQ(doc.blocks[1].lines.size(), 1u);
    EXPECT_EQ(doc.blocks[1].lines[0].op, bendiff::core::diff::LineOp::Delete);
    EXPECT_EQ(doc.blocks[1].lines[0].leftLine, 2u);
    EXPECT_FALSE(doc.blocks[1].lines[0].rightLine.has_value());
    EXPECT_EQ(doc.blocks[1].lines[0].leftText, "b");

    ASSERT_EQ(doc.blocks[2].lines.size(), 1u);
    EXPECT_EQ(doc.blocks[2].lines[0].op, bendiff::core::diff::LineOp::Insert);
    EXPECT_FALSE(doc.blocks[2].lines[0].leftLine.has_value());
    EXPECT_EQ(doc.blocks[2].lines[0].rightLine, 2u);
    EXPECT_EQ(doc.blocks[2].lines[0].rightText, "B");
}

} // namespace bendiff::core::render
