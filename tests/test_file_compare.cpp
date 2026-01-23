#include <file_compare.h>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

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

TEST(FileCompare, SameContentIsSame)
{
    const auto root = make_unique_temp_dir("bendiff_file_compare_same");
    const auto left = root / "left.bin";
    const auto right = root / "right.bin";

    write_file(left, std::string("\0\x01hello\n", 8));
    write_file(right, std::string("\0\x01hello\n", 8));

    EXPECT_EQ(bendiff::core::CompareFilesBytewise(left, right), bendiff::core::FileCompareResult::Same);

    fs::remove_all(root);
}

TEST(FileCompare, DifferentContentIsDifferent)
{
    const auto root = make_unique_temp_dir("bendiff_file_compare_diff");
    const auto left = root / "left.txt";
    const auto right = root / "right.txt";

    write_file(left, "hello\n");
    write_file(right, "hello!\n");

    EXPECT_EQ(bendiff::core::CompareFilesBytewise(left, right), bendiff::core::FileCompareResult::Different);

    fs::remove_all(root);
}

TEST(FileCompare, MissingFileIsUnreadable)
{
    const auto root = make_unique_temp_dir("bendiff_file_compare_missing");
    const auto left = root / "exists.txt";
    const auto missing = root / "does_not_exist.txt";

    write_file(left, "x");

    EXPECT_EQ(bendiff::core::CompareFilesBytewise(left, missing), bendiff::core::FileCompareResult::Unreadable);

    fs::remove_all(root);
}
