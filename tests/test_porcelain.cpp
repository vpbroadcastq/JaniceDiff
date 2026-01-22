#include <porcelain.h>

#include <gtest/gtest.h>

#include <string>

TEST(PorcelainV1, ParsesModifiedTrackedFile)
{
    const std::string text(" M src/main.cpp\0", sizeof(" M src/main.cpp\0") - 1);
    const auto files = bendiff::core::ParsePorcelainV1(text, /*nulSeparated=*/true);

    ASSERT_EQ(files.size(), 1u);
    EXPECT_EQ(files[0].repoRelativePath, "src/main.cpp");
    EXPECT_EQ(files[0].kind, bendiff::core::ChangeKind::Modified);
    EXPECT_FALSE(files[0].renameFrom.has_value());
}

TEST(PorcelainV1, ParsesUntrackedAsAdded)
{
    const std::string text("?? new_file.txt\0", sizeof("?? new_file.txt\0") - 1);
    const auto files = bendiff::core::ParsePorcelainV1(text, /*nulSeparated=*/true);

    ASSERT_EQ(files.size(), 1u);
    EXPECT_EQ(files[0].repoRelativePath, "new_file.txt");
    EXPECT_EQ(files[0].kind, bendiff::core::ChangeKind::Added);
}

TEST(PorcelainV1, ParsesDeletedFile)
{
    const std::string text(" D deleted.txt\0", sizeof(" D deleted.txt\0") - 1);
    const auto files = bendiff::core::ParsePorcelainV1(text, /*nulSeparated=*/true);

    ASSERT_EQ(files.size(), 1u);
    EXPECT_EQ(files[0].repoRelativePath, "deleted.txt");
    EXPECT_EQ(files[0].kind, bendiff::core::ChangeKind::Deleted);
}

TEST(PorcelainV1, ParsesRenamePair)
{
    // -z format rename: status + oldPath NUL newPath NUL
    const std::string text("R  oldname.txt\0newname.txt\0", sizeof("R  oldname.txt\0newname.txt\0") - 1);
    const auto files = bendiff::core::ParsePorcelainV1(text, /*nulSeparated=*/true);

    ASSERT_EQ(files.size(), 1u);
    EXPECT_EQ(files[0].repoRelativePath, "newname.txt");
    EXPECT_EQ(files[0].kind, bendiff::core::ChangeKind::Renamed);
    ASSERT_TRUE(files[0].renameFrom.has_value());
    EXPECT_EQ(*files[0].renameFrom, "oldname.txt");
}

TEST(PorcelainV1, ExcludesSubmoduleMarker)
{
    const std::string text("S  submodule\0 M keep.txt\0", sizeof("S  submodule\0 M keep.txt\0") - 1);
    const auto files = bendiff::core::ParsePorcelainV1(text, /*nulSeparated=*/true);

    ASSERT_EQ(files.size(), 1u);
    EXPECT_EQ(files[0].repoRelativePath, "keep.txt");
}
