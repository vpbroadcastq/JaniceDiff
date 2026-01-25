#include "DiffTextView.h"

#include <QFontDatabase>
#include <QTextBlock>
#include <QTextCursor>

#include <algorithm>

DiffTextView::DiffTextView(QWidget* parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(true);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

void DiffTextView::setMessage(const QString& message)
{
    setExtraSelections({});
    setPlainText(message);
    moveCursor(QTextCursor::Start);
}

QString DiffTextView::formatLineNumber(std::optional<std::size_t> oneBasedLine, int width)
{
    if (!oneBasedLine) {
        return QString(width, QLatin1Char(' '));
    }
    return QString::number(static_cast<qulonglong>(*oneBasedLine)).rightJustified(width, QLatin1Char(' '));
}

int DiffTextView::computeLineNumberWidth(const bendiff::core::render::RenderDocument& doc, Mode mode)
{
    std::size_t maxLine = 0;

    for (const auto& block : doc.blocks) {
        for (const auto& line : block.lines) {
            switch (mode) {
                case Mode::SideBySideLeft:
                    if (line.leftLine) {
                        maxLine = std::max(maxLine, *line.leftLine);
                    }
                    break;
                case Mode::SideBySideRight:
                    if (line.rightLine) {
                        maxLine = std::max(maxLine, *line.rightLine);
                    }
                    break;
                case Mode::Inline:
                    // Inline: show whichever side the block is on.
                    if (block.side == bendiff::core::render::RenderBlockSide::Right) {
                        if (line.rightLine) {
                            maxLine = std::max(maxLine, *line.rightLine);
                        }
                    } else {
                        if (line.leftLine) {
                            maxLine = std::max(maxLine, *line.leftLine);
                        }
                    }
                    break;
            }
        }
    }

    const int digits = QString::number(static_cast<qulonglong>(std::max<std::size_t>(1, maxLine))).size();
    return std::max(1, digits);
}

bool DiffTextView::shouldColorLine(bendiff::core::diff::LineOp op, Mode mode)
{
    using bendiff::core::diff::LineOp;

    if (op == LineOp::Equal) {
        return false;
    }

    // Side-by-side semantics:
    // - Deletes are red on the left view.
    // - Inserts are green on the right view.
    if (mode == Mode::SideBySideLeft) {
        return op == LineOp::Delete;
    }
    if (mode == Mode::SideBySideRight) {
        return op == LineOp::Insert;
    }

    // Inline: color both deletes and inserts.
    return true;
}

QColor DiffTextView::backgroundForOp(bendiff::core::diff::LineOp op)
{
    using bendiff::core::diff::LineOp;

    switch (op) {
        case LineOp::Insert:
            return QColor(200, 255, 200);
        case LineOp::Delete:
            return QColor(255, 200, 200);
        case LineOp::Equal:
            return QColor();
    }
    return QColor();
}

void DiffTextView::applyExtraSelections(const bendiff::core::render::RenderDocument& doc, Mode mode)
{
    using bendiff::core::diff::LineOp;

    QList<QTextEdit::ExtraSelection> sels;

    int visualRow = 0;
    for (const auto& block : doc.blocks) {
        for (const auto& line : block.lines) {
            if (shouldColorLine(line.op, mode)) {
                QTextCursor c(document()->findBlockByNumber(visualRow));
                c.select(QTextCursor::LineUnderCursor);

                QTextEdit::ExtraSelection sel;
                sel.cursor = c;
                sel.format.setBackground(backgroundForOp(line.op));
                sels.push_back(sel);
            }
            ++visualRow;
        }
    }

    setExtraSelections(sels);
}

void DiffTextView::setRenderDocument(const bendiff::core::render::RenderDocument& doc, Mode mode)
{
    const int width = computeLineNumberWidth(doc, mode);

    QString text;
    text.reserve(1024);

    for (const auto& block : doc.blocks) {
        for (const auto& line : block.lines) {
            QString lineNum;
            QString content;

            switch (mode) {
                case Mode::SideBySideLeft:
                    lineNum = formatLineNumber(line.leftLine, width);
                    content = QString::fromStdString(line.leftText);
                    break;
                case Mode::SideBySideRight:
                    lineNum = formatLineNumber(line.rightLine, width);
                    content = QString::fromStdString(line.rightText);
                    break;
                case Mode::Inline:
                    if (block.side == bendiff::core::render::RenderBlockSide::Right) {
                        lineNum = formatLineNumber(line.rightLine, width);
                        content = QString::fromStdString(line.rightText);
                    } else {
                        lineNum = formatLineNumber(line.leftLine, width);
                        content = QString::fromStdString(line.leftText);
                    }
                    break;
            }

            text += lineNum;
            text += " | ";
            text += content;
            text += "\n";
        }
    }

    setPlainText(text);
    moveCursor(QTextCursor::Start);

    applyExtraSelections(doc, mode);
}
