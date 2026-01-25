#include "diff_render_model.h"

#include <algorithm>

namespace bendiff::core::render {
namespace {

RenderLine MakeLineFromRow(const LoadedTextFile& left,
                           const LoadedTextFile& right,
                           const diff::AlignedRow& row)
{
    RenderLine out;
    out.op = row.op;

    if (row.left) {
        const auto idx = *row.left;
        out.leftLine = idx + 1;
        if (idx < left.lines.size()) {
            out.leftText = left.lines[idx];
        }
    }

    if (row.right) {
        const auto idx = *row.right;
        out.rightLine = idx + 1;
        if (idx < right.lines.size()) {
            out.rightText = right.lines[idx];
        }
    }

    return out;
}

void PushLine(RenderDocument& doc, RenderBlockSide side, RenderLine line)
{
    if (doc.blocks.empty() || doc.blocks.back().side != side) {
        RenderBlock b;
        b.side = side;
        doc.blocks.push_back(std::move(b));
    }

    doc.blocks.back().lines.push_back(std::move(line));
}

} // namespace

RenderDocument BuildSideBySideRender(const LoadedTextFile& left,
                                    const LoadedTextFile& right,
                                    const diff::DiffResult& d)
{
    RenderDocument doc;

    if (left.status != LoadStatus::Ok || right.status != LoadStatus::Ok) {
        return doc;
    }

    const auto rows = diff::BuildAlignedRows(d);

    RenderBlock block;
    block.side = RenderBlockSide::Both;
    block.lines.reserve(rows.size());

    for (const auto& row : rows) {
        block.lines.push_back(MakeLineFromRow(left, right, row));
    }

    doc.blocks.push_back(std::move(block));
    return doc;
}

RenderDocument BuildInlineRender(const LoadedTextFile& left,
                                const LoadedTextFile& right,
                                const diff::DiffResult& d)
{
    RenderDocument doc;

    if (left.status != LoadStatus::Ok || right.status != LoadStatus::Ok) {
        return doc;
    }

    const auto rows = diff::BuildAlignedRows(d);

    for (const auto& row : rows) {
        const auto line = MakeLineFromRow(left, right, row);

        switch (row.op) {
            case diff::LineOp::Equal:
                PushLine(doc, RenderBlockSide::Both, line);
                break;
            case diff::LineOp::Delete:
                PushLine(doc, RenderBlockSide::Left, line);
                break;
            case diff::LineOp::Insert:
                PushLine(doc, RenderBlockSide::Right, line);
                break;
        }
    }

    return doc;
}

} // namespace bendiff::core::render
