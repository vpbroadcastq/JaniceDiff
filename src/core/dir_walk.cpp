#include "dir_walk.h"

#include <system_error>

namespace fs = std::filesystem;

namespace bendiff::core {

std::vector<std::string> ListFilesRecursive(fs::path root)
{
    std::vector<std::string> results;

    std::error_code ec;
    if (root.empty()) {
        return results;
    }

    if (!fs::exists(root, ec) || ec) {
        return results;
    }
    ec.clear();

    if (!fs::is_directory(root, ec) || ec) {
        return results;
    }
    ec.clear();

    // Prefer an absolute root for stable relative-path computation.
    const fs::path abs = fs::absolute(root, ec);
    if (!ec && !abs.empty()) {
        root = abs;
    }
    ec.clear();

    const auto opts = fs::directory_options::skip_permission_denied;
    for (fs::recursive_directory_iterator it(root, opts, ec), end; it != end; it.increment(ec)) {
        if (ec) {
            ec.clear();
            continue;
        }

        if (!it->is_regular_file(ec) || ec) {
            ec.clear();
            continue;
        }
        ec.clear();

        const fs::path p = it->path();
        const fs::path rel = fs::relative(p, root, ec);
        if (ec) {
            ec.clear();
            continue;
        }

        const std::string relStr = rel.generic_string();
        if (relStr.empty() || relStr == ".") {
            continue;
        }

        results.push_back(relStr);
    }

    return results;
}

} // namespace bendiff::core
