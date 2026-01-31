#include <diff/diff.h>
#include <render/diff_render_model.h>

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

namespace bendiff::core::render {

namespace {

char SideChar(RenderBlockSide side)
{
    switch (side) {
        case RenderBlockSide::Left:
            return 'L';
        case RenderBlockSide::Right:
            return 'R';
        case RenderBlockSide::Both:
            return 'B';
    }
    return '?';
}

char OpChar(bendiff::core::diff::LineOp op)
{
    switch (op) {
        case bendiff::core::diff::LineOp::Equal:
            return '=';
        case bendiff::core::diff::LineOp::Insert:
            return '+';
        case bendiff::core::diff::LineOp::Delete:
            return '-';
    }
    return '?';
}

std::string NumOrDash(const std::optional<std::size_t>& n)
{
    if (!n) {
        return "-";
    }
    return std::to_string(*n);
}

std::vector<std::string> Simplify(const RenderDocument& doc)
{
    // One entry per visual row, including the block side.
    // Format: "<Side> <Op> <leftLine> <rightLine>"
    std::vector<std::string> out;
    for (const auto& block : doc.blocks) {
        for (const auto& line : block.lines) {
            std::string s;
            s.reserve(32);
            s.push_back(SideChar(block.side));
            s.push_back(' ');
            s.push_back(OpChar(line.op));
            s.push_back(' ');
            s += NumOrDash(line.leftLine);
            s.push_back(' ');
            s += NumOrDash(line.rightLine);
            out.push_back(std::move(s));
        }
    }
    return out;
}

std::vector<RenderBlockSide> BlockSides(const RenderDocument& doc)
{
    std::vector<RenderBlockSide> out;
    out.reserve(doc.blocks.size());
    for (const auto& b : doc.blocks) {
        out.push_back(b.side);
    }
    return out;
}

} // namespace

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

TEST(DiffRenderModel, InlineDeletedFileIsOneBigLeftBlock)
{
    bendiff::core::LoadedTextFile left;
    left.status = bendiff::core::LoadStatus::Ok;
    left.lines = {"a", "b"};

    bendiff::core::LoadedTextFile right;
    right.status = bendiff::core::LoadStatus::NotFound;

    // DiffResult isn't used for single-sided inline rendering; pass a default.
    bendiff::core::diff::DiffResult d;

    const auto doc = BuildInlineRender(left, right, d);
    ASSERT_EQ(doc.blocks.size(), 1u);
    EXPECT_EQ(doc.blocks[0].side, RenderBlockSide::Left);
    ASSERT_EQ(doc.blocks[0].lines.size(), 2u);

    EXPECT_EQ(doc.blocks[0].lines[0].op, bendiff::core::diff::LineOp::Delete);
    EXPECT_EQ(doc.blocks[0].lines[0].leftLine, 1u);
    EXPECT_FALSE(doc.blocks[0].lines[0].rightLine.has_value());
    EXPECT_EQ(doc.blocks[0].lines[0].leftText, "a");

    EXPECT_EQ(doc.blocks[0].lines[1].op, bendiff::core::diff::LineOp::Delete);
    EXPECT_EQ(doc.blocks[0].lines[1].leftLine, 2u);
    EXPECT_FALSE(doc.blocks[0].lines[1].rightLine.has_value());
    EXPECT_EQ(doc.blocks[0].lines[1].leftText, "b");
}

TEST(DiffRenderModel, InlineAddedFileIsOneBigRightBlock)
{
    bendiff::core::LoadedTextFile left;
    left.status = bendiff::core::LoadStatus::NotFound;

    bendiff::core::LoadedTextFile right;
    right.status = bendiff::core::LoadStatus::Ok;
    right.lines = {"x"};

    bendiff::core::diff::DiffResult d;

    const auto doc = BuildInlineRender(left, right, d);
    ASSERT_EQ(doc.blocks.size(), 1u);
    EXPECT_EQ(doc.blocks[0].side, RenderBlockSide::Right);
    ASSERT_EQ(doc.blocks[0].lines.size(), 1u);

    EXPECT_EQ(doc.blocks[0].lines[0].op, bendiff::core::diff::LineOp::Insert);
    EXPECT_FALSE(doc.blocks[0].lines[0].leftLine.has_value());
    EXPECT_EQ(doc.blocks[0].lines[0].rightLine, 1u);
    EXPECT_EQ(doc.blocks[0].lines[0].rightText, "x");
}

TEST(DiffRenderModel, InlineInsertInMiddleProducesBothThenRightThenBoth)
{
    bendiff::core::LoadedTextFile left;
    left.status = bendiff::core::LoadStatus::Ok;
    left.lines = {"a", "b", "c", "d"};

    bendiff::core::LoadedTextFile right;
    right.status = bendiff::core::LoadStatus::Ok;
    right.lines = {"a", "b", "X", "c", "d"};

    const auto d = bendiff::core::diff::DiffLines(left.lines, right.lines, bendiff::core::diff::WhitespaceMode::Exact);
    const auto doc = BuildInlineRender(left, right, d);

    EXPECT_EQ(BlockSides(doc), (std::vector<RenderBlockSide>{RenderBlockSide::Both, RenderBlockSide::Right, RenderBlockSide::Both}));
    EXPECT_EQ(Simplify(doc), (std::vector<std::string>{
                                  "B = 1 1",
                                  "B = 2 2",
                                  "R + - 3",
                                  "B = 3 4",
                                  "B = 4 5",
                              }));
}

TEST(DiffRenderModel, InlineConsecutiveDeletesAreGroupedIntoOneLeftBlock)
{
    bendiff::core::LoadedTextFile left;
    left.status = bendiff::core::LoadStatus::Ok;
    left.lines = {"a", "x", "y", "b"};

    bendiff::core::LoadedTextFile right;
    right.status = bendiff::core::LoadStatus::Ok;
    right.lines = {"a", "b"};

    const auto d = bendiff::core::diff::DiffLines(left.lines, right.lines, bendiff::core::diff::WhitespaceMode::Exact);
    const auto doc = BuildInlineRender(left, right, d);

    EXPECT_EQ(BlockSides(doc), (std::vector<RenderBlockSide>{RenderBlockSide::Both, RenderBlockSide::Left, RenderBlockSide::Both}));
    EXPECT_EQ(Simplify(doc), (std::vector<std::string>{
                                  "B = 1 1",
                                  "L - 2 -",
                                  "L - 3 -",
                                  "B = 4 2",
                              }));
}

TEST(DiffRenderModel, InlineConsecutiveInsertsAreGroupedIntoOneRightBlock)
{
    bendiff::core::LoadedTextFile left;
    left.status = bendiff::core::LoadStatus::Ok;
    left.lines = {"a", "b"};

    bendiff::core::LoadedTextFile right;
    right.status = bendiff::core::LoadStatus::Ok;
    right.lines = {"a", "x", "y", "b"};

    const auto d = bendiff::core::diff::DiffLines(left.lines, right.lines, bendiff::core::diff::WhitespaceMode::Exact);
    const auto doc = BuildInlineRender(left, right, d);

    EXPECT_EQ(BlockSides(doc), (std::vector<RenderBlockSide>{RenderBlockSide::Both, RenderBlockSide::Right, RenderBlockSide::Both}));
    EXPECT_EQ(Simplify(doc), (std::vector<std::string>{
                                  "B = 1 1",
                                  "R + - 2",
                                  "R + - 3",
                                  "B = 2 4",
                              }));
}

TEST(DiffRenderModel, SideBySideIgnoreTrailingWhitespaceCanMakeLinesEqual)
{
    bendiff::core::LoadedTextFile left;
    left.status = bendiff::core::LoadStatus::Ok;
    left.lines = {"a ", "b\t"};

    bendiff::core::LoadedTextFile right;
    right.status = bendiff::core::LoadStatus::Ok;
    right.lines = {"a", "b"};

    const auto d = bendiff::core::diff::DiffLines(left.lines, right.lines, bendiff::core::diff::WhitespaceMode::IgnoreTrailing);
    const auto doc = BuildSideBySideRender(left, right, d);

    ASSERT_EQ(doc.blocks.size(), 1u);
    EXPECT_EQ(doc.blocks[0].side, RenderBlockSide::Both);
    EXPECT_EQ(Simplify(doc), (std::vector<std::string>{
                                  "B = 1 1",
                                  "B = 2 2",
                              }));
}

TEST(DiffRenderModel, InlineIgnoreTrailingWhitespaceCanMakeLinesEqual)
{
    bendiff::core::LoadedTextFile left;
    left.status = bendiff::core::LoadStatus::Ok;
    left.lines = {"a ", "b\t"};

    bendiff::core::LoadedTextFile right;
    right.status = bendiff::core::LoadStatus::Ok;
    right.lines = {"a", "b"};

    const auto d = bendiff::core::diff::DiffLines(left.lines, right.lines, bendiff::core::diff::WhitespaceMode::IgnoreTrailing);
    const auto doc = BuildInlineRender(left, right, d);

    EXPECT_EQ(BlockSides(doc), (std::vector<RenderBlockSide>{RenderBlockSide::Both}));
    EXPECT_EQ(Simplify(doc), (std::vector<std::string>{
                                  "B = 1 1",
                                  "B = 2 2",
                              }));
}

} // namespace bendiff::core::render
