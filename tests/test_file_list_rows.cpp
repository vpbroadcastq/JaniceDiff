#include <file_list_rows.h>

#include <gtest/gtest.h>

#include <vector>

TEST(FileListRows, GroupsAndSortsIntoFlatHeaderList)
{
    using bendiff::core::ChangedFile;
    using bendiff::core::ChangeKind;

    std::vector<ChangedFile> files;

    files.push_back(ChangedFile{.repoRelativePath = "b/z.txt", .kind = ChangeKind::Modified, .renameFrom = std::nullopt});
    files.push_back(ChangedFile{.repoRelativePath = "README.md", .kind = ChangeKind::Modified, .renameFrom = std::nullopt});
    files.push_back(ChangedFile{.repoRelativePath = "a/y.txt", .kind = ChangeKind::Added, .renameFrom = std::nullopt});
    files.push_back(ChangedFile{.repoRelativePath = "a/x.txt", .kind = ChangeKind::Deleted, .renameFrom = std::nullopt});
    files.push_back(ChangedFile{.repoRelativePath = "b/a.txt", .kind = ChangeKind::Modified, .renameFrom = std::nullopt});

    const auto rows = bendiff::core::BuildGroupedFileListRows(files);

    // Expected order:
    // (root)
    //   README.md
    // a
    //   a/x.txt
    //   a/y.txt
    // b
    //   b/a.txt
    //   b/z.txt
    ASSERT_EQ(rows.size(), 8u);

    EXPECT_FALSE(rows[0].selectable);
    EXPECT_EQ(rows[0].kind, bendiff::core::FileListRowKind::Header);
    EXPECT_EQ(rows[0].displayText, "(root)");

    EXPECT_TRUE(rows[1].selectable);
    EXPECT_EQ(rows[1].displayText, "README.md");
    ASSERT_TRUE(rows[1].file.has_value());

    EXPECT_FALSE(rows[2].selectable);
    EXPECT_EQ(rows[2].kind, bendiff::core::FileListRowKind::Header);
    EXPECT_EQ(rows[2].displayText, "a");

    EXPECT_TRUE(rows[3].selectable);
    EXPECT_EQ(rows[3].displayText, "a/x.txt");

    EXPECT_TRUE(rows[4].selectable);
    EXPECT_EQ(rows[4].displayText, "a/y.txt");

    EXPECT_FALSE(rows[5].selectable);
    EXPECT_EQ(rows[5].kind, bendiff::core::FileListRowKind::Header);
    EXPECT_EQ(rows[5].displayText, "b");

    EXPECT_TRUE(rows[6].selectable);
    EXPECT_EQ(rows[6].displayText, "b/a.txt");

    EXPECT_TRUE(rows[7].selectable);
    EXPECT_EQ(rows[7].displayText, "b/z.txt");
}
