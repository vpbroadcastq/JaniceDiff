#include "content_sources.h"

#include <system_error>

namespace fs = std::filesystem;

namespace bendiff::core {

ProcessResult RunGitShowHeadPath(fs::path repoRoot, std::string_view repoRelativePath)
{
    std::error_code ec;
    const fs::path abs = fs::absolute(repoRoot, ec);
    if (!ec && !abs.empty()) {
        repoRoot = abs;
    }

    // git expects pathspec-like forward slashes; porcelain already returns that.
    const std::string spec = std::string("HEAD:") + std::string(repoRelativePath);
    return RunProcess({"git", "show", spec}, repoRoot);
}

ResolvedContentSides ResolveFolderContent(fs::path leftRoot, fs::path rightRoot, std::string_view relativePath)
{
    std::error_code ec;
    const fs::path absLeft = fs::absolute(leftRoot, ec);
    if (!ec && !absLeft.empty()) {
        leftRoot = absLeft;
    }
    ec.clear();
    const fs::path absRight = fs::absolute(rightRoot, ec);
    if (!ec && !absRight.empty()) {
        rightRoot = absRight;
    }

    ResolvedContentSides out;

    out.left.kind = ContentSource::Kind::FileOnDisk;
    out.left.absolutePath = leftRoot / fs::path(relativePath);

    out.right.kind = ContentSource::Kind::FileOnDisk;
    out.right.absolutePath = rightRoot / fs::path(relativePath);

    return out;
}

ResolvedContentSides ResolveRepoContent(fs::path repoRoot, const ChangedFile& file)
{
    std::error_code ec;
    const fs::path abs = fs::absolute(repoRoot, ec);
    if (!ec && !abs.empty()) {
        repoRoot = abs;
    }

    ResolvedContentSides out;

    // Right side: working tree path (except deletions).
    out.right.absolutePath = repoRoot / fs::path(file.repoRelativePath);

    if (file.kind == ChangeKind::Deleted) {
        out.right.kind = ContentSource::Kind::Missing;
    } else {
        out.right.kind = ContentSource::Kind::FileOnDisk;
    }

    // Left side: committed version (except added/untracked).
    if (file.kind == ChangeKind::Added) {
        out.left.kind = ContentSource::Kind::Missing;
        return out;
    }

    std::string showPath = file.repoRelativePath;
    if (file.kind == ChangeKind::Renamed && file.renameFrom.has_value()) {
        showPath = *file.renameFrom;
    }

    out.left.kind = ContentSource::Kind::Bytes;
    out.left.process = RunGitShowHeadPath(repoRoot, showPath);
    out.left.bytes = out.left.process.stdoutText;

    return out;
}

} // namespace bendiff::core
