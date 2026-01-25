#pragma once

#include <diff/alignment.h>
#include <diff/diff.h>
#include <loaded_text_file.h>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace bendiff::core::render {

enum class RenderBlockSide {
    Left,
    Right,
    Both,
};

struct RenderLine {
    std::optional<std::size_t> leftLine;  // 1-based line number
    std::optional<std::size_t> rightLine; // 1-based line number

    diff::LineOp op = diff::LineOp::Equal;

    // Text for each side (empty if not present).
    std::string leftText;
    std::string rightText;
};

struct RenderBlock {
    RenderBlockSide side = RenderBlockSide::Both;
    std::vector<RenderLine> lines;
};

struct RenderDocument {
    std::vector<RenderBlock> blocks;
};

RenderDocument BuildSideBySideRender(const LoadedTextFile& left,
                                    const LoadedTextFile& right,
                                    const diff::DiffResult& d);

RenderDocument BuildInlineRender(const LoadedTextFile& left,
                                const LoadedTextFile& right,
                                const diff::DiffResult& d);

} // namespace bendiff::core::render
