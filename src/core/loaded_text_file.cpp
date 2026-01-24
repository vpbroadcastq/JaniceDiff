#include "loaded_text_file.h"

namespace bendiff::core {

SplitLinesResult SplitLinesNormalizeNewlines(std::string_view text)
{
    SplitLinesResult out;

    if (text.empty()) {
        return out;
    }

    std::string current;
    current.reserve(text.size());

    auto flush = [&] {
        out.lines.push_back(current);
        current.clear();
    };

    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];

        if (c == '\n') {
            flush();
            continue;
        }

        if (c == '\r') {
            // Treat CRLF as a single line break.
            if (i + 1 < text.size() && text[i + 1] == '\n') {
                ++i;
            }
            flush();
            continue;
        }

        current.push_back(c);
    }

    const char last = text.back();
    out.hadFinalNewline = (last == '\n') || (last == '\r');

    if (!out.hadFinalNewline) {
        // No trailing terminator => keep final partial line (possibly empty).
        out.lines.push_back(current);
    }

    return out;
}

} // namespace bendiff::core
