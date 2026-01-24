#pragma once

#include "loaded_text_file.h"

#include <string>
#include <vector>

namespace bendiff::core {

enum class RenderLineKind {
    Context,
    Added,
    Deleted,
};

struct RenderLine {
    RenderLineKind kind = RenderLineKind::Context;
    std::string text;
};

struct RenderPane {
    std::string title;
    std::vector<RenderLine> lines;
};

enum class DiffViewMode {
    Inline,
    SideBySide,
};

struct DiffRenderModel {
    DiffViewMode mode = DiffViewMode::Inline;

    // Inline: use `inlinePane`.
    RenderPane inlinePane;

    // Side-by-side: use `leftPane` and `rightPane`.
    RenderPane leftPane;
    RenderPane rightPane;

    bool isDeletedFile = false;
};

// Builds a render model for a deleted file.
//
// v1 semantics (pre-diff-engine):
// - Inline mode: all committed lines are emitted as Deleted.
// - Side-by-side: left pane shows committed lines (Deleted), right pane is empty
//   and indicates deletion.
DiffRenderModel BuildDeletedFileRenderModel(const LoadedTextFile& committed, DiffViewMode mode);

} // namespace bendiff::core
