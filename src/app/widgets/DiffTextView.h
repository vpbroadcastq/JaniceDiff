#pragma once

#include <QPlainTextEdit>

#include <render/diff_render_model.h>

#include <optional>

class DiffTextView final : public QPlainTextEdit
{
    Q_OBJECT

public:
    enum class Mode {
        Inline,
        SideBySideLeft,
        SideBySideRight,
    };

    explicit DiffTextView(QWidget* parent = nullptr);

    void setMessage(const QString& message);

    void setRenderDocument(const bendiff::core::render::RenderDocument& doc, Mode mode);

private:
    void applyExtraSelections(const bendiff::core::render::RenderDocument& doc, Mode mode);

    static QString formatLineNumber(std::optional<std::size_t> oneBasedLine, int width);
    static int computeLineNumberWidth(const bendiff::core::render::RenderDocument& doc, Mode mode);
    static bool shouldColorLine(bendiff::core::diff::LineOp op, Mode mode);
    static QColor backgroundForOp(bendiff::core::diff::LineOp op);
};
