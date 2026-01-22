#include <repo_status.h>

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
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

} // namespace

TEST(RepoStatus, PorcelainEmptyInCleanRepo)
{
    if (!git_available()) {
        GTEST_SKIP() << "git not available on PATH";
    }

    const auto repo = make_unique_temp_dir("bendiff_repo_status_clean");
    const auto init = bendiff::core::RunProcess({"git", "init", "-q"}, repo);
    ASSERT_EQ(init.exitCode, 0) << init.stderrText;

    const auto r = bendiff::core::RunGitStatusPorcelainV1Z(repo);
    EXPECT_EQ(r.exitCode, 0) << r.stderrText;
    EXPECT_TRUE(r.stdoutText.empty());

    fs::remove_all(repo);
}

TEST(RepoStatus, PorcelainNonEmptyWhenUntrackedFileExists)
{
    if (!git_available()) {
        GTEST_SKIP() << "git not available on PATH";
    }

    const auto repo = make_unique_temp_dir("bendiff_repo_status_dirty");
    const auto init = bendiff::core::RunProcess({"git", "init", "-q"}, repo);
    ASSERT_EQ(init.exitCode, 0) << init.stderrText;

    {
        std::ofstream out(repo / "untracked.txt", std::ios::binary);
        ASSERT_TRUE(out.good());
        out << "hello\n";
    }

    const auto r = bendiff::core::RunGitStatusPorcelainV1Z(repo);
    EXPECT_EQ(r.exitCode, 0) << r.stderrText;
    EXPECT_FALSE(r.stdoutText.empty());

    fs::remove_all(repo);
}
