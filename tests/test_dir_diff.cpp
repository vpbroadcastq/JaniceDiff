#include <dir_diff.h>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace fs = std::filesystem;

namespace {

fs::path fixture_root()
{
    // tests/test_dir_diff.cpp -> tests/fixtures/dir_diff
    const fs::path here = fs::path(__FILE__).parent_path();
    return here / "fixtures" / "dir_diff";
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

void write_file(const fs::path& p, const std::string& bytes)
{
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary);
    ASSERT_TRUE(out.good()) << p;
    out << bytes;
}

} // namespace

TEST(DirDiff, ProducesUnionAndClassifiesStatuses)
{
    const fs::path root = fixture_root();
    const fs::path left = root / "left";
    const fs::path right = root / "right";

    const auto r = bendiff::core::DiffDirectories(left, right);

    // Stable sort guarantee.
    for (std::size_t i = 1; i < r.entries.size(); ++i) {
        ASSERT_LE(r.entries[i - 1].relativePath, r.entries[i].relativePath);
    }

    std::map<std::string, bendiff::core::DirEntryStatus> byPath;
    for (const auto& e : r.entries) {
        byPath[e.relativePath] = e.status;
    }

    EXPECT_EQ(byPath["left_only.txt"], bendiff::core::DirEntryStatus::LeftOnly);
    EXPECT_EQ(byPath["right_only.txt"], bendiff::core::DirEntryStatus::RightOnly);
    EXPECT_EQ(byPath["same.txt"], bendiff::core::DirEntryStatus::Same);
    EXPECT_EQ(byPath["different.txt"], bendiff::core::DirEntryStatus::Different);
    EXPECT_EQ(byPath["nested/n1.txt"], bendiff::core::DirEntryStatus::Same);
}

TEST(DirDiff, UnreadableBestEffort)
{
#if defined(_WIN32)
    GTEST_SKIP() << "Unreadable simulation via chmod is POSIX-only";
#else
    const auto root = make_unique_temp_dir("bendiff_dir_diff_unreadable");
    const auto left = root / "left";
    const auto right = root / "right";
    fs::create_directories(left);
    fs::create_directories(right);

    const auto rel = fs::path("unreadable.txt");
    const auto leftFile = left / rel;
    const auto rightFile = right / rel;

    write_file(leftFile, "secret\n");
    write_file(rightFile, "secret\n");

    // Remove all permissions from the left file. This should make it unreadable.
    std::error_code ec;
    fs::permissions(leftFile, fs::perms::none, fs::perm_options::replace, ec);
    if (ec) {
        fs::remove_all(root);
        GTEST_SKIP() << "Could not change permissions to simulate unreadable: " << ec.message();
    }

    const auto r = bendiff::core::DiffDirectories(left, right);
    auto it = std::find_if(r.entries.begin(), r.entries.end(), [](const bendiff::core::DirEntry& e) {
        return e.relativePath == "unreadable.txt";
    });
    ASSERT_TRUE(it != r.entries.end());
    EXPECT_EQ(it->status, bendiff::core::DirEntryStatus::Unreadable);

    // Best-effort cleanup: restore permissions so remove_all succeeds.
    ec.clear();
    fs::permissions(leftFile,
                    fs::perms::owner_read | fs::perms::owner_write,
                    fs::perm_options::replace,
                    ec);
    fs::remove_all(root);
#endif
}
