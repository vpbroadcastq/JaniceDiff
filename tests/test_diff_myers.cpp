#include <diff/diff.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace bendiff::core::diff {

namespace {

std::vector<LineOp> OpsOnly(const DiffResult& r)
{
    std::vector<LineOp> out;
    for (const auto& h : r.hunks) {
        for (const auto& l : h.lines) {
            out.push_back(l.op);
        }
    }
    return out;
}

} // namespace

TEST(MyersDiff, EmptyLeftIsAllInserts)
{
    const std::vector<std::string> left;
    const std::vector<std::string> right = {"a", "b"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);
    EXPECT_EQ(OpsOnly(r), (std::vector<LineOp>{LineOp::Insert, LineOp::Insert}));
}

TEST(MyersDiff, EmptyRightIsAllDeletes)
{
    const std::vector<std::string> left = {"a", "b"};
    const std::vector<std::string> right;

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);
    EXPECT_EQ(OpsOnly(r), (std::vector<LineOp>{LineOp::Delete, LineOp::Delete}));
}

TEST(MyersDiff, EqualInputsAreAllEquals)
{
    const std::vector<std::string> left = {"a", "b"};
    const std::vector<std::string> right = {"a", "b"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);
    EXPECT_TRUE(r.hunks.empty());
}

TEST(MyersDiff, SingleReplaceBecomesDeleteInsert)
{
    const std::vector<std::string> left = {"a", "b", "c"};
    const std::vector<std::string> right = {"a", "x", "c"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);
    EXPECT_EQ(OpsOnly(r), (std::vector<LineOp>{LineOp::Delete, LineOp::Insert}));
}

TEST(MyersDiff, RepeatedLinesDeterministicAlignment)
{
    const std::vector<std::string> left = {"a", "b", "a"};
    const std::vector<std::string> right = {"a", "a"};

    const auto r = DiffLines(left, right, WhitespaceMode::Exact);
    EXPECT_EQ(OpsOnly(r), (std::vector<LineOp>{LineOp::Delete}));
}

TEST(MyersDiff, WhitespaceModeAffectsEquality)
{
    const std::vector<std::string> left = {"foo"};
    const std::vector<std::string> right = {"foo \t"};

    const auto exact = DiffLines(left, right, WhitespaceMode::Exact);
    EXPECT_EQ(OpsOnly(exact), (std::vector<LineOp>{LineOp::Delete, LineOp::Insert}));

    const auto ignoreTrailing = DiffLines(left, right, WhitespaceMode::IgnoreTrailing);
    EXPECT_TRUE(ignoreTrailing.hunks.empty());
}

} // namespace bendiff::core::diff
