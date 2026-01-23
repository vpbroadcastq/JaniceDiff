#include "dir_diff.h"

#include "dir_walk.h"
#include "file_compare.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <set>
#include <system_error>

namespace fs = std::filesystem;

namespace bendiff::core {

namespace {

fs::path make_abs_if_possible(fs::path p)
{
    std::error_code ec;
    const fs::path abs = fs::absolute(p, ec);
    if (!ec && !abs.empty()) {
        return abs;
    }
    return p;
}

bool can_open_for_read(const fs::path& p)
{
    std::ifstream in(p, std::ios::binary);
    return in.is_open();
}

fs::path full_path(const fs::path& root, const std::string& relativePath)
{
    fs::path rel = fs::path(relativePath);
    rel.make_preferred();
    return root / rel;
}

} // namespace

DirDiffResult DiffDirectories(const fs::path& leftRootIn, const fs::path& rightRootIn)
{
    DirDiffResult result;
    result.leftRoot = make_abs_if_possible(leftRootIn);
    result.rightRoot = make_abs_if_possible(rightRootIn);

    const auto leftFiles = ListFilesRecursive(result.leftRoot);
    const auto rightFiles = ListFilesRecursive(result.rightRoot);

    std::set<std::string> leftSet(leftFiles.begin(), leftFiles.end());
    std::set<std::string> rightSet(rightFiles.begin(), rightFiles.end());

    std::set<std::string> all;
    all.insert(leftSet.begin(), leftSet.end());
    all.insert(rightSet.begin(), rightSet.end());

    result.entries.reserve(all.size());

    for (const auto& rel : all) {
        const bool inLeft = leftSet.contains(rel);
        const bool inRight = rightSet.contains(rel);

        DirEntry entry;
        entry.relativePath = rel;

        if (inLeft && inRight) {
            const fs::path leftFull = full_path(result.leftRoot, rel);
            const fs::path rightFull = full_path(result.rightRoot, rel);

            const auto cmp = CompareFilesBytewise(leftFull, rightFull);
            switch (cmp) {
                case FileCompareResult::Same:
                    entry.status = DirEntryStatus::Same;
                    break;
                case FileCompareResult::Different:
                    entry.status = DirEntryStatus::Different;
                    break;
                case FileCompareResult::Unreadable:
                    entry.status = DirEntryStatus::Unreadable;
                    break;
            }
        } else if (inLeft) {
            const fs::path leftFull = full_path(result.leftRoot, rel);
            entry.status = can_open_for_read(leftFull) ? DirEntryStatus::LeftOnly : DirEntryStatus::Unreadable;
        } else {
            const fs::path rightFull = full_path(result.rightRoot, rel);
            entry.status = can_open_for_read(rightFull) ? DirEntryStatus::RightOnly : DirEntryStatus::Unreadable;
        }

        result.entries.push_back(std::move(entry));
    }

    // `all` is already lexicographically sorted, but keep this explicit for stability if implementation changes.
    std::sort(result.entries.begin(), result.entries.end(), [](const DirEntry& a, const DirEntry& b) {
        return a.relativePath < b.relativePath;
    });

    return result;
}

} // namespace bendiff::core
