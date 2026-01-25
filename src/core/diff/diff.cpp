#include <diff/diff.h>

namespace bendiff::core::diff {

DiffResult DiffLines(std::span<const std::string> left,
                     std::span<const std::string> right,
                     WhitespaceMode mode)
{
    // M5-T1: API + data model only.
    // Algorithm, hunk generation, and whitespace handling arrive in later tasks.
    DiffResult r;
    r.mode = mode;
    r.leftLineCount = left.size();
    r.rightLineCount = right.size();
    return r;
}

} // namespace bendiff::core::diff
