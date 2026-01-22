#include <process.h>

#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

TEST(ProcessRunner, MissingExecutableReturnsNonZero)
{
    const auto wd = fs::temp_directory_path();
    const auto r = bendiff::core::RunProcess({"bendiff_this_command_should_not_exist_0f3f2d"}, wd);
    EXPECT_NE(r.exitCode, 0);
    EXPECT_FALSE(r.stderrText.empty());
}

TEST(ProcessRunner, GitVersionIfEnabled)
{
    if (std::getenv("BENDIFF_TEST_RUN_GIT") == nullptr) {
        GTEST_SKIP() << "Set BENDIFF_TEST_RUN_GIT=1 to enable";
    }

    const auto wd = fs::current_path();
    const auto r = bendiff::core::RunProcess({"git", "--version"}, wd);
    EXPECT_EQ(r.exitCode, 0);
    EXPECT_TRUE(r.stdoutText.find("git version") != std::string::npos);
}
