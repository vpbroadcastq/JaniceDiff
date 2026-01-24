#include "render_model.h"

namespace bendiff::core {

DiffRenderModel BuildDeletedFileRenderModel(const LoadedTextFile& committed, DiffViewMode mode)
{
    DiffRenderModel out;
    out.mode = mode;
    out.isDeletedFile = true;

    auto make_deleted_lines = [&] {
        std::vector<RenderLine> lines;
        lines.reserve(committed.lines.size());
        for (const auto& line : committed.lines) {
            RenderLine rl;
            rl.kind = RenderLineKind::Deleted;
            rl.text = line;
            lines.push_back(std::move(rl));
        }
        return lines;
    };

    if (mode == DiffViewMode::Inline) {
        out.inlinePane.title = "Deleted file";
        out.inlinePane.lines = make_deleted_lines();
        return out;
    }

    out.leftPane.title = "Deleted file (committed)";
    out.leftPane.lines = make_deleted_lines();

    out.rightPane.title = "Deleted file (working tree)";
    out.rightPane.lines.clear();

    return out;
}

} // namespace bendiff::core
