#include <invocation.h>

#include <gtest/gtest.h>

#include <filesystem>

namespace fs = std::filesystem;

TEST(InvocationParsing, ZeroArgsIsRepoModeUsingCwd)
{
    const auto inv = bendiff::parse_invocation({});
    EXPECT_EQ(inv.mode, bendiff::AppMode::RepoMode);
    EXPECT_FALSE(inv.repoPath.empty());
}

TEST(InvocationParsing, OneArgExistingDirIsRepoMode)
{
    const auto temp = fs::temp_directory_path();
    const auto inv = bendiff::parse_invocation({temp.string()});
    EXPECT_EQ(inv.mode, bendiff::AppMode::RepoMode);
    EXPECT_EQ(inv.repoPath, temp);
}

TEST(InvocationParsing, OneArgMissingDirIsInvalid)
{
    const auto missing = fs::temp_directory_path() / "bendiff_missing_dir_should_not_exist_9f0f4a9d";
    if (fs::exists(missing)) {
        fs::remove_all(missing);
    }

    const auto inv = bendiff::parse_invocation({missing.string()});
    EXPECT_EQ(inv.mode, bendiff::AppMode::Invalid);
    EXPECT_FALSE(inv.error.empty());
}

TEST(InvocationParsing, TwoArgsExistingDirsIsFolderDiffMode)
{
    const auto base = fs::temp_directory_path() / "bendiff_invocation_test_dirs";
    const auto left = base / "left";
    const auto right = base / "right";

    fs::remove_all(base);
    fs::create_directories(left);
    fs::create_directories(right);

    const auto inv = bendiff::parse_invocation({left.string(), right.string()});
    EXPECT_EQ(inv.mode, bendiff::AppMode::FolderDiffMode);
    EXPECT_EQ(inv.leftPath, left);
    EXPECT_EQ(inv.rightPath, right);

    fs::remove_all(base);
}

TEST(InvocationParsing, ThreeArgsIsInvalid)
{
    const auto inv = bendiff::parse_invocation({"a", "b", "c"});
    EXPECT_EQ(inv.mode, bendiff::AppMode::Invalid);
}
