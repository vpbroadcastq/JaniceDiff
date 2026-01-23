#include <dir_diff_model.h>

#include <gtest/gtest.h>

TEST(DirDiffModel, CanConstructDirDiffResult)
{
    bendiff::core::DirDiffResult result;
    result.leftRoot = "/tmp/left";
    result.rightRoot = "/tmp/right";

    bendiff::core::DirEntry same;
    same.relativePath = "src/main.cpp";
    same.status = bendiff::core::DirEntryStatus::Same;
    same.isBinaryHint = false;

    bendiff::core::DirEntry different;
    different.relativePath = "README.md";
    different.status = bendiff::core::DirEntryStatus::Different;

    bendiff::core::DirEntry leftOnly;
    leftOnly.relativePath = "only_left.txt";
    leftOnly.status = bendiff::core::DirEntryStatus::LeftOnly;

    result.entries.push_back(same);
    result.entries.push_back(different);
    result.entries.push_back(leftOnly);

    ASSERT_EQ(result.entries.size(), 3u);
    EXPECT_EQ(result.entries[0].status, bendiff::core::DirEntryStatus::Same);
    EXPECT_EQ(result.entries[1].status, bendiff::core::DirEntryStatus::Different);
    EXPECT_EQ(result.entries[2].status, bendiff::core::DirEntryStatus::LeftOnly);
}
