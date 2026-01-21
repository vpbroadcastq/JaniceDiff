#include <model.h>

#include <gtest/gtest.h>

TEST(CoreModel, CanConstructRepoStatusAndChangedFiles)
{
    bendiff::core::RepoStatus status;
    status.repoRoot = "/tmp/example-repo";

    bendiff::core::ChangedFile a;
    a.repoRelativePath = "src/main.cpp";
    a.kind = bendiff::core::ChangeKind::Modified;

    bendiff::core::ChangedFile r;
    r.repoRelativePath = "newname.txt";
    r.kind = bendiff::core::ChangeKind::Renamed;
    r.renameFrom = std::string("oldname.txt");

    status.files.push_back(a);
    status.files.push_back(r);

    ASSERT_EQ(status.files.size(), 2u);
    EXPECT_EQ(status.files[0].kind, bendiff::core::ChangeKind::Modified);
    EXPECT_TRUE(status.files[1].renameFrom.has_value());
}
