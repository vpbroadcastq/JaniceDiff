#pragma once

#include "model.h"
#include "process.h"

#include <filesystem>
#include <string>

namespace bendiff::core {

// Represents a source of bytes/content for a diff side.
//
// v1 goals:
// - Folder mode: both sides are file paths on disk.
// - Repo mode: left side comes from `git show HEAD:<path>`, right side comes
//   from the working tree path on disk (or Missing for deletions).
struct ContentSource {
    enum class Kind {
        Missing,
        FileOnDisk,
        Bytes,
    };

    Kind kind = Kind::Missing;

    // For FileOnDisk or Missing, this is the absolute path in the working tree.
    std::filesystem::path absolutePath;

    // For Bytes, this holds the raw bytes.
    std::string bytes;

    // For Bytes produced by git, this holds process diagnostics.
    ProcessResult process;
};

struct ResolvedContentSides {
    ContentSource left;
    ContentSource right;
};

// Folder mode: resolve absolute file paths directly.
ResolvedContentSides ResolveFolderContent(std::filesystem::path leftRoot,
                                          std::filesystem::path rightRoot,
                                          std::string_view relativePath);

// Repo mode: resolve committed-vs-working-tree sources.
ResolvedContentSides ResolveRepoContent(std::filesystem::path repoRoot, const ChangedFile& file);

// Runs: git show HEAD:<repoRelativePath>
ProcessResult RunGitShowHeadPath(std::filesystem::path repoRoot, std::string_view repoRelativePath);

} // namespace bendiff::core
