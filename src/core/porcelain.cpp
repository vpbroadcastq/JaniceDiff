#include "porcelain.h"

#include <cctype>
#include <string>

namespace bendiff::core {
namespace {

// Returns a sequence of fields split by delimiter; omits the final empty field if input ends in delimiter.
static std::vector<std::string_view> split_view(std::string_view text, char delim)
{
    std::vector<std::string_view> out;

    std::size_t start = 0;
    while (start <= text.size()) {
        const std::size_t end = text.find(delim, start);
        if (end == std::string_view::npos) {
            if (start < text.size()) {
                out.push_back(text.substr(start));
            }
            break;
        }

        out.push_back(text.substr(start, end - start));
        start = end + 1;
    }

    // If text ended with delim, the last push would have been an empty view; drop it.
    if (!out.empty() && out.back().empty() && !text.empty() && text.back() == delim) {
        out.pop_back();
    }

    return out;
}

static std::string_view ltrim_spaces(std::string_view s)
{
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
        s.remove_prefix(1);
    }
    return s;
}

static bool is_ignored_entry(char x, char y)
{
    return x == '!' && y == '!';
}

static bool is_untracked_entry(char x, char y)
{
    return x == '?' && y == '?';
}

static bool is_submodule_marker(char x, char y)
{
    // Heuristic per milestone text: treat 'S' as submodule indicator.
    return x == 'S' || y == 'S';
}

static bool is_unmerged_entry(char x, char y)
{
    // Git uses a range of unmerged codes; include any U and common AA/DD.
    if (x == 'U' || y == 'U') {
        return true;
    }
    if ((x == 'A' && y == 'A') || (x == 'D' && y == 'D')) {
        return true;
    }
    return false;
}

static ChangeKind classify_kind(char x, char y)
{
    if (is_unmerged_entry(x, y)) {
        return ChangeKind::Unmerged;
    }

    // Rename/copy indicators can appear in either column depending on staged/unstaged.
    if (x == 'R' || y == 'R') {
        return ChangeKind::Renamed;
    }

    // Deleted if either side indicates deletion.
    if (x == 'D' || y == 'D') {
        return ChangeKind::Deleted;
    }

    // Added includes untracked and staged adds.
    if (is_untracked_entry(x, y) || x == 'A' || y == 'A') {
        return ChangeKind::Added;
    }

    if (x == 'M' || y == 'M') {
        return ChangeKind::Modified;
    }

    return ChangeKind::Unknown;
}

static std::optional<ChangedFile> parse_record_xy_path(std::string_view record)
{
    // Record shapes:
    // - "XY <path>"  (most)
    // - "?? <path>"  (untracked)
    // - "!! <path>"  (ignored)
    if (record.size() < 3) {
        return std::nullopt;
    }

    const char x = record[0];
    const char y = record[1];

    if (is_ignored_entry(x, y) || is_submodule_marker(x, y)) {
        return std::nullopt;
    }

    // There is typically a space after XY, but be defensive.
    std::string_view rest = record.substr(2);
    rest = ltrim_spaces(rest);
    if (rest.empty()) {
        return std::nullopt;
    }

    ChangedFile f;
    f.repoRelativePath = std::string(rest);
    f.kind = classify_kind(x, y);

    return f;
}

static void add_entry(std::vector<ChangedFile>& out, ChangedFile&& f)
{
    // Filter out entries we consider irrelevant for v1.
    if (f.repoRelativePath.empty()) {
        return;
    }
    out.push_back(std::move(f));
}

} // namespace

std::vector<ChangedFile> ParsePorcelainV1(std::string_view text, bool nulSeparated)
{
    std::vector<ChangedFile> out;

    if (text.empty()) {
        return out;
    }

    if (nulSeparated) {
        // With -z, records are NUL-delimited.
        // Rename records are represented as:
        //   "R<y> <old>\0<new>\0" (or status in either column)
        // which yields two consecutive NUL-terminated fields.
        const auto fields = split_view(text, '\0');

        for (std::size_t i = 0; i < fields.size(); ++i) {
            const std::string_view record = fields[i];
            if (record.empty()) {
                continue;
            }

            // Common case: parse "XY path".
            if (record.size() >= 2) {
                const char x = record[0];
                const char y = record[1];

                if (is_ignored_entry(x, y) || is_submodule_marker(x, y)) {
                    continue;
                }

                const ChangeKind kind = classify_kind(x, y);

                // Rename: next field is the new name.
                if (kind == ChangeKind::Renamed) {
                    // Parse old path from current record.
                    std::string_view oldPart = record.substr(2);
                    oldPart = ltrim_spaces(oldPart);
                    if (oldPart.empty()) {
                        continue;
                    }
                    if (i + 1 >= fields.size()) {
                        continue;
                    }
                    const std::string_view newPath = fields[i + 1];
                    if (newPath.empty()) {
                        continue;
                    }

                    ChangedFile f;
                    f.repoRelativePath = std::string(newPath);
                    f.kind = ChangeKind::Renamed;
                    f.renameFrom = std::string(oldPart);
                    add_entry(out, std::move(f));

                    // Consume the extra field.
                    ++i;
                    continue;
                }
            }

            if (auto parsed = parse_record_xy_path(record)) {
                add_entry(out, std::move(*parsed));
            }
        }

        return out;
    }

    // Non -z output is newline-delimited with paths possibly containing spaces.
    // Rename format is typically: "R  old -> new".
    // This mode isn't the recommended path, but keeping it helps if the caller uses it.
    const auto lines = split_view(text, '\n');
    for (auto line : lines) {
        if (line.empty()) {
            continue;
        }

        // Trim CR for CRLF.
        if (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }

        if (line.size() < 3) {
            continue;
        }

        const char x = line[0];
        const char y = line[1];
        if (is_ignored_entry(x, y) || is_submodule_marker(x, y)) {
            continue;
        }

        const ChangeKind kind = classify_kind(x, y);
        std::string_view rest = line.substr(2);
        rest = ltrim_spaces(rest);
        if (rest.empty()) {
            continue;
        }

        if (kind == ChangeKind::Renamed) {
            const std::string_view arrow = " -> ";
            const std::size_t pos = rest.find(arrow);
            if (pos == std::string_view::npos) {
                continue;
            }
            const std::string_view oldPath = rest.substr(0, pos);
            const std::string_view newPath = rest.substr(pos + arrow.size());
            if (oldPath.empty() || newPath.empty()) {
                continue;
            }

            ChangedFile f;
            f.repoRelativePath = std::string(newPath);
            f.kind = ChangeKind::Renamed;
            f.renameFrom = std::string(oldPath);
            add_entry(out, std::move(f));
            continue;
        }

        ChangedFile f;
        f.repoRelativePath = std::string(rest);
        f.kind = kind;
        add_entry(out, std::move(f));
    }

    return out;
}

} // namespace bendiff::core
