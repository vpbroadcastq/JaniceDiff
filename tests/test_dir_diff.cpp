#include <dir_diff.h>

#include <gtest/gtest.h>

#include <filesystem>
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
