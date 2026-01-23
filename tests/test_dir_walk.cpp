#include <dir_walk.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

fs::path make_unique_temp_dir(const std::string& prefix)
{
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    const auto stamp = std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(now).count());

    fs::path dir = fs::temp_directory_path() / (prefix + "_" + stamp);
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir;
}

void write_file(const fs::path& p, const std::string& bytes)
{
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary);
    ASSERT_TRUE(out.good()) << p;
    out << bytes;
}

} // namespace

TEST(DirWalk, ListsAllFilesRecursivelyWithNormalizedSeparators)
{
    const auto root = make_unique_temp_dir("bendiff_dir_walk");

    write_file(root / "alpha.txt", "a");
    write_file(root / "dir1" / "beta.txt", "b");
    write_file(root / "dir1" / "dir2" / "gamma.bin", std::string("\0\x01\x02", 3));

    // Ensure we don't accidentally ignore ".git".
    write_file(root / ".git" / "HEAD", "ref: refs/heads/main\n");

    auto got = bendiff::core::ListFilesRecursive(root);
    std::sort(got.begin(), got.end());

    std::vector<std::string> expected = {
        ".git/HEAD",
        "alpha.txt",
        "dir1/beta.txt",
        "dir1/dir2/gamma.bin",
    };
    std::sort(expected.begin(), expected.end());

    EXPECT_EQ(got, expected);

    for (const auto& s : got) {
        EXPECT_EQ(s.find('\\'), std::string::npos) << "Path was not normalized: " << s;
    }

    fs::remove_all(root);
}
