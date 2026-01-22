#include "file_list_rows.h"

#include <algorithm>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

namespace bendiff::core {
namespace {

static std::string group_key_for_path(const std::string& repoRelativePath)
{
    if (repoRelativePath.empty()) {
        return std::string();
    }

    fs::path p(repoRelativePath);

    // Normalize to '/' separators for stable UI grouping.
    p = p.lexically_normal();

    fs::path parent = p.parent_path();
    if (parent.empty() || parent == ".") {
        return std::string();
    }

    return parent.generic_string();
}

static std::string header_text_for_group(const std::string& groupKey)
{
    return groupKey.empty() ? std::string("(root)") : groupKey;
}

} // namespace

std::vector<FileListRow> BuildGroupedFileListRows(const std::vector<ChangedFile>& files)
{
    // Group into an ordered map so groups are sorted alphabetically.
    std::map<std::string, std::vector<ChangedFile>> groups;

    for (const auto& f : files) {
        groups[group_key_for_path(f.repoRelativePath)].push_back(f);
    }

    std::vector<FileListRow> out;

    for (auto& [groupKey, items] : groups) {
        std::sort(items.begin(), items.end(), [](const ChangedFile& a, const ChangedFile& b) {
            return a.repoRelativePath < b.repoRelativePath;
        });

        FileListRow header;
        header.kind = FileListRowKind::Header;
        header.groupKey = groupKey;
        header.displayText = header_text_for_group(groupKey);
        header.selectable = false;
        header.file.reset();
        out.push_back(std::move(header));

        for (auto& item : items) {
            FileListRow row;
            row.kind = FileListRowKind::File;
            row.groupKey = groupKey;
            row.displayText = item.repoRelativePath;
            row.file = std::move(item);
            row.selectable = true;
            out.push_back(std::move(row));
        }
    }

    return out;
}

} // namespace bendiff::core
