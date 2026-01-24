#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace bendiff::core {

enum class LoadStatus {
    Ok,
    NotFound,
    Unreadable,
    NotUtf8,
};

struct LoadedTextFile {
    std::filesystem::path absolutePath;
    LoadStatus status = LoadStatus::Ok;
    std::vector<std::string> lines;
    bool hadFinalNewline = false;
};

struct SplitLinesResult {
    std::vector<std::string> lines;
    bool hadFinalNewline = false;
};

// Normalizes line endings and splits into lines.
//
// Rules:
// - Treat CRLF ("\r\n"), LF ("\n"), and lone CR ("\r") as line breaks.
// - Returned lines never include line-ending characters.
// - `hadFinalNewline` is true iff the input ends with a line break.
SplitLinesResult SplitLinesNormalizeNewlines(std::string_view utf8Text);

// Loads a file as UTF-8 text.
//
// Behavior:
// - If the file doesn't exist: status=NotFound
// - If the file exists but can't be read: status=Unreadable
// - If the file contains invalid UTF-8 bytes: status=NotUtf8
// - Otherwise: status=Ok, and line endings are normalized via
//   SplitLinesNormalizeNewlines().
LoadedTextFile LoadUtf8TextFile(std::filesystem::path absolutePath);

} // namespace bendiff::core
