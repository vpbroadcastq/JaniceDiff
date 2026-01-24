#include <render_model.h>

#include <gtest/gtest.h>

namespace bendiff::core {

TEST(RenderModel, DeletedFileInlineMarksAllLinesDeleted)
{
    LoadedTextFile committed;
    committed.status = LoadStatus::Ok;
    committed.lines = {"a", "b", "c"};

    const auto rm = BuildDeletedFileRenderModel(committed, DiffViewMode::Inline);
    EXPECT_TRUE(rm.isDeletedFile);
    EXPECT_EQ(rm.mode, DiffViewMode::Inline);

    ASSERT_EQ(rm.inlinePane.lines.size(), 3u);
    EXPECT_EQ(rm.inlinePane.lines[0].kind, RenderLineKind::Deleted);
    EXPECT_EQ(rm.inlinePane.lines[0].text, "a");
    EXPECT_EQ(rm.inlinePane.lines[2].kind, RenderLineKind::Deleted);
    EXPECT_EQ(rm.inlinePane.lines[2].text, "c");
}

TEST(RenderModel, DeletedFileSideBySideRightIsEmpty)
{
    LoadedTextFile committed;
    committed.status = LoadStatus::Ok;
    committed.lines = {"x"};

    const auto rm = BuildDeletedFileRenderModel(committed, DiffViewMode::SideBySide);
    EXPECT_TRUE(rm.isDeletedFile);
    EXPECT_EQ(rm.mode, DiffViewMode::SideBySide);

    ASSERT_EQ(rm.leftPane.lines.size(), 1u);
    EXPECT_EQ(rm.leftPane.lines[0].kind, RenderLineKind::Deleted);
    EXPECT_EQ(rm.leftPane.lines[0].text, "x");

    EXPECT_TRUE(rm.rightPane.lines.empty());
}

} // namespace bendiff::core
