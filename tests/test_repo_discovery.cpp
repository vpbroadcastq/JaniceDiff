#include <repo_discovery.h>

#include <gtest/gtest.h>

#include <filesystem>

namespace fs = std::filesystem;

TEST(RepoDiscovery, ReturnsNulloptWhenNotInRepo)
{
    const auto base = fs::temp_directory_path() / "bendiff_repo_discovery_not_repo";
    fs::remove_all(base);
    fs::create_directories(base / "a" / "b" / "c");

    const auto found = bendiff::core::FindGitRepoRoot(base / "a" / "b" / "c");
    EXPECT_FALSE(found.has_value());

    fs::remove_all(base);
}

TEST(RepoDiscovery, FindsDotGitDirectoryWalkingUp)
{
    const auto base = fs::temp_directory_path() / "bendiff_repo_discovery_repo";
    const auto repo = base / "repo";
    const auto nested = repo / "src" / "deep";

    fs::remove_all(base);
    fs::create_directories(nested);
    fs::create_directories(repo / ".git");

    const auto found = bendiff::core::FindGitRepoRoot(nested);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, fs::absolute(repo));

    fs::remove_all(base);
}
