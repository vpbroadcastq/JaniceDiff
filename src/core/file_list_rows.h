#pragma once

#include "model.h"

#include <optional>
#include <string>
#include <vector>

namespace bendiff::core {

enum class FileListRowKind {
    Header,
    File,
};

struct FileListRow {
    FileListRowKind kind = FileListRowKind::File;

    // What the UI should render.
    std::string displayText;

    // For file rows, the underlying file entry.
    std::optional<ChangedFile> file;

    // Directory group key ("" for root/top-level).
    std::string groupKey;

    bool selectable = true;
};

// Builds a flat, UI-ready row list grouped by directory.
//
// Contract (v1):
// - Group key: directory portion of repoRelativePath ("" for top-level files)
// - Groups sorted alphabetically by group key
// - Files within each group sorted alphabetically by repoRelativePath
// - Flat list with non-selectable header rows, then file rows
std::vector<FileListRow> BuildGroupedFileListRows(const std::vector<ChangedFile>& files);

} // namespace bendiff::core
