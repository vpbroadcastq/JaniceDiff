#include <diff/whitespace.h>

#include <cstddef>

namespace bendiff::core::diff {

namespace {

inline bool is_ws(char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

} // namespace

std::string MakeComparisonKey(std::string_view line, WhitespaceMode mode)
{
    switch (mode) {
    case WhitespaceMode::Exact:
        return std::string(line);

    case WhitespaceMode::IgnoreTrailing: {
        std::size_t end = line.size();
        while (end > 0 && is_ws(line[end - 1])) {
            --end;
        }
        return std::string(line.substr(0, end));
    }

    case WhitespaceMode::IgnoreAll: {
        std::string out;
        out.reserve(line.size());
        for (char c : line) {
            if (!is_ws(c)) {
                out.push_back(c);
            }
        }
        return out;
    }
    }

    // Defensive fallback.
    return std::string(line);
}

} // namespace bendiff::core::diff
