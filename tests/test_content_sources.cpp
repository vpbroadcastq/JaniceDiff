#include <content_sources.h>
#include <process.h>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace {

bool git_available()
{
    const auto wd = fs::temp_directory_path();
    const auto r = bendiff::core::RunProcess({"git", "--version"}, wd);
    return r.exitCode == 0;
}

fs::path make_unique_temp_dir(const std::string& prefix)
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    const auto stamp = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());

    fs::path dir = fs::temp_directory_path() / (prefix + "_" + stamp);
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir;
}

void write_text(const fs::path& path, const std::string& text)
{
    std::ofstream out(path, std::ios::binary);
    ASSERT_TRUE(out.good());
    out << text;
    ASSERT_TRUE(out.good());
}

void git_config_identity(const fs::path& repo)
{
    auto r = bendiff::core::RunProcess({"git", "config", "user.email", "bendiff@test"}, repo);
    ASSERT_EQ(r.exitCode, 0) << r.stderrText;
    r = bendiff::core::RunProcess({"git", "config", "user.name", "bendiff"}, repo);
    ASSERT_EQ(r.exitCode, 0) << r.stderrText;
}

void git_commit_all(const fs::path& repo, const std::string& msg)
{
    auto r = bendiff::core::RunProcess({"git", "add", "-A"}, repo);
    ASSERT_EQ(r.exitCode, 0) << r.stderrText;

    r = bendiff::core::RunProcess({"git", "commit", "-q", "-m", msg}, repo);
    ASSERT_EQ(r.exitCode, 0) << r.stderrText;
}

} // namespace

TEST(ContentSources, FolderModeResolvesAbsolutePaths)
{
    const fs::path left = make_unique_temp_dir("bendiff_folder_left");
    const fs::path right = make_unique_temp_dir("bendiff_folder_right");

    const auto sides = bendiff::core::ResolveFolderContent(left, right, "a/b.txt");
    EXPECT_EQ(sides.left.kind, bendiff::core::ContentSource::Kind::FileOnDisk);
    EXPECT_EQ(sides.right.kind, bendiff::core::ContentSource::Kind::FileOnDisk);

    EXPECT_TRUE(sides.left.absolutePath.is_absolute());
    EXPECT_TRUE(sides.right.absolutePath.is_absolute());
    EXPECT_TRUE(sides.left.absolutePath.string().find("a") != std::string::npos);
    EXPECT_TRUE(sides.right.absolutePath.string().find("b.txt") != std::string::npos);

    fs::remove_all(left);
    fs::remove_all(right);
}

TEST(ContentSources, RepoModeModifiedFileHasGitBytesAndWorkingPath)
{
    if (!git_available()) {
        GTEST_SKIP() << "git not available on PATH";
    }

    const fs::path repo = make_unique_temp_dir("bendiff_repo_content_modified");
    auto r = bendiff::core::RunProcess({"git", "init", "-q"}, repo);
    ASSERT_EQ(r.exitCode, 0) << r.stderrText;
    git_config_identity(repo);

    write_text(repo / "a.txt", "old\n");
    git_commit_all(repo, "initial");

    // Modify working tree.
    write_text(repo / "a.txt", "new\n");

    bendiff::core::ChangedFile cf;
    cf.repoRelativePath = "a.txt";
    cf.kind = bendiff::core::ChangeKind::Modified;

    const auto sides = bendiff::core::ResolveRepoContent(repo, cf);

    EXPECT_EQ(sides.left.kind, bendiff::core::ContentSource::Kind::Bytes);
    EXPECT_EQ(sides.left.process.exitCode, 0) << sides.left.process.stderrText;
    EXPECT_EQ(sides.left.bytes, "old\n");

    EXPECT_EQ(sides.right.kind, bendiff::core::ContentSource::Kind::FileOnDisk);
    EXPECT_EQ(sides.right.absolutePath, repo / "a.txt");

    fs::remove_all(repo);
}

TEST(ContentSources, RepoModeAddedFileHasMissingLeft)
{
    if (!git_available()) {
        GTEST_SKIP() << "git not available on PATH";
    }

    const fs::path repo = make_unique_temp_dir("bendiff_repo_content_added");
    auto r = bendiff::core::RunProcess({"git", "init", "-q"}, repo);
    ASSERT_EQ(r.exitCode, 0) << r.stderrText;
    git_config_identity(repo);

    write_text(repo / "u.txt", "untracked\n");

    bendiff::core::ChangedFile cf;
    cf.repoRelativePath = "u.txt";
    cf.kind = bendiff::core::ChangeKind::Added;

    const auto sides = bendiff::core::ResolveRepoContent(repo, cf);
    EXPECT_EQ(sides.left.kind, bendiff::core::ContentSource::Kind::Missing);
    EXPECT_EQ(sides.right.kind, bendiff::core::ContentSource::Kind::FileOnDisk);

    fs::remove_all(repo);
}

TEST(ContentSources, RepoModeDeletedFileHasMissingRight)
{
    if (!git_available()) {
        GTEST_SKIP() << "git not available on PATH";
    }

    const fs::path repo = make_unique_temp_dir("bendiff_repo_content_deleted");
    auto r = bendiff::core::RunProcess({"git", "init", "-q"}, repo);
    ASSERT_EQ(r.exitCode, 0) << r.stderrText;
    git_config_identity(repo);

    write_text(repo / "d.txt", "gone\n");
    git_commit_all(repo, "initial");

    fs::remove(repo / "d.txt");

    bendiff::core::ChangedFile cf;
    cf.repoRelativePath = "d.txt";
    cf.kind = bendiff::core::ChangeKind::Deleted;

    const auto sides = bendiff::core::ResolveRepoContent(repo, cf);

    EXPECT_EQ(sides.left.kind, bendiff::core::ContentSource::Kind::Bytes);
    EXPECT_EQ(sides.left.process.exitCode, 0) << sides.left.process.stderrText;
    EXPECT_EQ(sides.left.bytes, "gone\n");

    EXPECT_EQ(sides.right.kind, bendiff::core::ContentSource::Kind::Missing);

    fs::remove_all(repo);
}

TEST(ContentSources, RepoModeRenamedUsesRenameFromForCommittedSide)
{
    if (!git_available()) {
        GTEST_SKIP() << "git not available on PATH";
    }

    const fs::path repo = make_unique_temp_dir("bendiff_repo_content_renamed");
    auto r = bendiff::core::RunProcess({"git", "init", "-q"}, repo);
    ASSERT_EQ(r.exitCode, 0) << r.stderrText;
    git_config_identity(repo);

    write_text(repo / "old.txt", "before\n");
    git_commit_all(repo, "initial");

    // Simulate rename in working tree (no commit).
    fs::rename(repo / "old.txt", repo / "new.txt");

    bendiff::core::ChangedFile cf;
    cf.repoRelativePath = "new.txt";
    cf.kind = bendiff::core::ChangeKind::Renamed;
    cf.renameFrom = std::string("old.txt");

    const auto sides = bendiff::core::ResolveRepoContent(repo, cf);

    EXPECT_EQ(sides.left.kind, bendiff::core::ContentSource::Kind::Bytes);
    EXPECT_EQ(sides.left.process.exitCode, 0) << sides.left.process.stderrText;
    EXPECT_EQ(sides.left.bytes, "before\n");

    EXPECT_EQ(sides.right.kind, bendiff::core::ContentSource::Kind::FileOnDisk);
    EXPECT_EQ(sides.right.absolutePath, repo / "new.txt");

    fs::remove_all(repo);
}
