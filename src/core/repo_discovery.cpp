#include "repo_discovery.h"

#include <system_error>

namespace fs = std::filesystem;

namespace bendiff::core {

std::optional<fs::path> FindGitRepoRoot(fs::path startDir)
{
    std::error_code ec;

    if (startDir.empty()) {
        return std::nullopt;
    }

    // Normalize to an absolute directory path when possible.
    fs::path current = startDir;
    if (fs::exists(current, ec) && !ec) {
        if (fs::is_regular_file(current, ec) && !ec) {
            current = current.parent_path();
        }
    }
    ec.clear();

    // If the caller passed a relative path, prefer absolute for stable parent traversal.
    const fs::path abs = fs::absolute(current, ec);
    if (!ec && !abs.empty()) {
        current = abs;
    }
    ec.clear();

    // Walk up parents, checking for a ".git" directory.
    while (!current.empty()) {
        const fs::path dotGit = current / ".git";
        if (fs::exists(dotGit, ec) && !ec && fs::is_directory(dotGit, ec) && !ec) {
            return current;
        }
        ec.clear();

        const fs::path parent = current.parent_path();
        if (parent.empty() || parent == current) {
            break;
        }
        current = parent;
    }

    return std::nullopt;
}

} // namespace bendiff::core
