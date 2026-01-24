#include "loaded_text_file.h"

#include <fstream>

namespace fs = std::filesystem;

namespace bendiff::core {

namespace {

// RFC 3629 style UTF-8 validation.
bool IsValidUtf8(std::string_view bytes)
{
    std::size_t i = 0;
    while (i < bytes.size()) {
        const unsigned char c = static_cast<unsigned char>(bytes[i]);

        if (c <= 0x7F) {
            ++i;
            continue;
        }

        auto is_cont = [&](std::size_t idx) {
            if (idx >= bytes.size()) {
                return false;
            }
            const unsigned char cc = static_cast<unsigned char>(bytes[idx]);
            return (cc & 0xC0) == 0x80;
        };

        if (c >= 0xC2 && c <= 0xDF) {
            // 2-byte sequence
            if (!is_cont(i + 1)) {
                return false;
            }
            i += 2;
            continue;
        }

        if (c == 0xE0) {
            // 3-byte sequence, special lower bound to avoid overlongs
            if (!is_cont(i + 1) || !is_cont(i + 2)) {
                return false;
            }
            const unsigned char c1 = static_cast<unsigned char>(bytes[i + 1]);
            if (c1 < 0xA0) {
                return false;
            }
            i += 3;
            continue;
        }

        if (c >= 0xE1 && c <= 0xEC) {
            if (!is_cont(i + 1) || !is_cont(i + 2)) {
                return false;
            }
            i += 3;
            continue;
        }

        if (c == 0xED) {
            // exclude UTF-16 surrogate halves
            if (!is_cont(i + 1) || !is_cont(i + 2)) {
                return false;
            }
            const unsigned char c1 = static_cast<unsigned char>(bytes[i + 1]);
            if (c1 >= 0xA0) {
                return false;
            }
            i += 3;
            continue;
        }

        if (c >= 0xEE && c <= 0xEF) {
            if (!is_cont(i + 1) || !is_cont(i + 2)) {
                return false;
            }
            i += 3;
            continue;
        }

        if (c == 0xF0) {
            // 4-byte, special lower bound
            if (!is_cont(i + 1) || !is_cont(i + 2) || !is_cont(i + 3)) {
                return false;
            }
            const unsigned char c1 = static_cast<unsigned char>(bytes[i + 1]);
            if (c1 < 0x90) {
                return false;
            }
            i += 4;
            continue;
        }

        if (c >= 0xF1 && c <= 0xF3) {
            if (!is_cont(i + 1) || !is_cont(i + 2) || !is_cont(i + 3)) {
                return false;
            }
            i += 4;
            continue;
        }

        if (c == 0xF4) {
            // 4-byte, special upper bound (max U+10FFFF)
            if (!is_cont(i + 1) || !is_cont(i + 2) || !is_cont(i + 3)) {
                return false;
            }
            const unsigned char c1 = static_cast<unsigned char>(bytes[i + 1]);
            if (c1 > 0x8F) {
                return false;
            }
            i += 4;
            continue;
        }

        return false;
    }

    return true;
}

} // namespace

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

LoadedTextFile LoadUtf8TextFile(fs::path absolutePath)
{
    LoadedTextFile out;
    out.absolutePath = std::move(absolutePath);
    out.status = LoadStatus::Unreadable;
    out.lines.clear();
    out.hadFinalNewline = false;

    std::ifstream in(out.absolutePath, std::ios::in | std::ios::binary);
    if (!in) {
        std::error_code ec;
        const bool exists = fs::exists(out.absolutePath, ec);

        // Some libstdc++/libc++ combinations report an error code for a
        // non-existent path here (instead of ec=0 + exists=false). Treat
        // ENOENT-style errors as NotFound.
        if ((!ec && !exists) || ec == std::errc::no_such_file_or_directory) {
            out.status = LoadStatus::NotFound;
        } else {
            out.status = LoadStatus::Unreadable;
        }
        return out;
    }

    std::string bytes;
    in.seekg(0, std::ios::end);
    if (in.good()) {
        const auto endPos = in.tellg();
        if (endPos > 0) {
            bytes.reserve(static_cast<std::size_t>(endPos));
        }
    }
    in.seekg(0, std::ios::beg);

    bytes.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
    if (!in.good() && !in.eof()) {
        out.status = LoadStatus::Unreadable;
        return out;
    }

    if (!IsValidUtf8(bytes)) {
        out.status = LoadStatus::NotUtf8;
        return out;
    }

    const auto split = SplitLinesNormalizeNewlines(bytes);
    out.lines = split.lines;
    out.hadFinalNewline = split.hadFinalNewline;
    out.status = LoadStatus::Ok;
    return out;
}

} // namespace bendiff::core
