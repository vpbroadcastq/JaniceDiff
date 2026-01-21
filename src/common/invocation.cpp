#include "invocation.h"

#include <system_error>

namespace fs = std::filesystem;

namespace bendiff {
namespace {

bool dir_exists(const fs::path& path)
{
    std::error_code ec;
    return fs::exists(path, ec) && !ec && fs::is_directory(path, ec) && !ec;
}

} // namespace

const char* to_string(AppMode mode)
{
    switch (mode) {
    case AppMode::RepoMode:
        return "RepoMode";
    case AppMode::FolderDiffMode:
        return "FolderDiffMode";
    case AppMode::Invalid:
        return "Invalid";
    }
    return "Invalid";
}

Invocation parse_invocation(const std::vector<std::string>& args)
{
    Invocation out;

    if (args.empty()) {
        std::error_code ec;
        out.mode = AppMode::RepoMode;
        out.repoPath = fs::current_path(ec);
        if (ec) {
            out.mode = AppMode::Invalid;
            out.error = "Failed to read current working directory";
        }
        return out;
    }

    if (args.size() == 1) {
        out.repoPath = fs::path(args[0]);
        if (!dir_exists(out.repoPath)) {
            out.mode = AppMode::Invalid;
            out.error = "Repo path does not exist or is not a directory";
            return out;
        }

        out.mode = AppMode::RepoMode;
        return out;
    }

    if (args.size() == 2) {
        out.leftPath = fs::path(args[0]);
        out.rightPath = fs::path(args[1]);

        if (!dir_exists(out.leftPath) || !dir_exists(out.rightPath)) {
            out.mode = AppMode::Invalid;
            out.error = "Folder diff paths must exist and be directories";
            return out;
        }

        out.mode = AppMode::FolderDiffMode;
        return out;
    }

    out.mode = AppMode::Invalid;
    out.error = "Invalid argument count (expected 0, 1, or 2 arguments)";
    return out;
}

} // namespace bendiff
