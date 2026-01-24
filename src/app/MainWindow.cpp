#include "MainWindow.h"

#include <logging.h>

#include <dir_diff.h>
#include <file_list_rows.h>
#include <model.h>
#include <repo_discovery.h>
#include <repo_status.h>

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QFileDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QWidget* make_placeholder_panel(const QString& title, QLabel** outLabel, QWidget* parent)
{
    auto* frame = new QFrame(parent);
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Sunken);
    frame->setMinimumWidth(120);

    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(12, 12, 12, 12);

    auto* label = new QLabel(title, frame);
    label->setAlignment(Qt::AlignCenter);
    label->setWordWrap(true);
    if (outLabel) {
        *outLabel = label;
    }

    layout->addStretch(1);
    layout->addWidget(label);
    layout->addStretch(1);

    return frame;
}

void log_not_implemented(const char* what)
{
    bendiff::logging::info(std::string("Not implemented: ") + what);
}

const char* to_string(bendiff::core::ChangeKind kind)
{
    using bendiff::core::ChangeKind;
    switch (kind) {
    case ChangeKind::Modified:
        return "Modified";
    case ChangeKind::Added:
        return "Added";
    case ChangeKind::Deleted:
        return "Deleted";
    case ChangeKind::Renamed:
        return "Renamed";
    case ChangeKind::Unmerged:
        return "Unmerged";
    case ChangeKind::Unknown:
        return "Unknown";
    }
    return "Unknown";
}

const char* to_string(bendiff::core::DirEntryStatus status)
{
    using bendiff::core::DirEntryStatus;
    switch (status) {
    case DirEntryStatus::Same:
        return "Same";
    case DirEntryStatus::Different:
        return "Different";
    case DirEntryStatus::LeftOnly:
        return "LeftOnly";
    case DirEntryStatus::RightOnly:
        return "RightOnly";
    case DirEntryStatus::Unreadable:
        return "Unreadable";
    }
    return "Unknown";
}

bool validate_dir_path(const std::filesystem::path& p)
{
    std::error_code ec;
    if (p.empty()) {
        return false;
    }
    if (!std::filesystem::exists(p, ec) || ec) {
        return false;
    }
    ec.clear();
    if (!std::filesystem::is_directory(p, ec) || ec) {
        return false;
    }
    return true;
}

} // namespace

MainWindow::MainWindow(const bendiff::Invocation& invocation, QWidget* parent)
    : QMainWindow(parent)
    , m_invocation(invocation)
{
    setWindowTitle("BenDiff");

    setup_menus();
    setup_toolbar();
    setup_central();

    set_pane_mode(PaneMode::Inline);
    refresh_repo_discovery();
    refresh_file_list();
    reset_placeholders();
    update_status_bar();
}

void MainWindow::setup_menus()
{
    auto* fileMenu = menuBar()->addMenu("&File");
    auto* viewMenu = menuBar()->addMenu("&View");
    auto* diffMenu = menuBar()->addMenu("&Diff");
    auto* helpMenu = menuBar()->addMenu("&Help");

    (void)viewMenu;
    (void)diffMenu;
    (void)helpMenu;

    m_actionOpenRepo = new QAction("Open Repo...", this);
    m_actionOpenFolders = new QAction("Open Folders...", this);

    fileMenu->addAction(m_actionOpenRepo);
    fileMenu->addAction(m_actionOpenFolders);

    connect(m_actionOpenRepo, &QAction::triggered, this, [this] {
        const QString selected = QFileDialog::getExistingDirectory(
            this,
            "Open Repo",
            QString::fromStdString(m_invocation.repoPath.empty() ? std::string() : m_invocation.repoPath.string()),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (selected.isEmpty()) {
            return;
        }
        enter_repo_mode(std::filesystem::path(selected.toStdString()));
    });

    connect(m_actionOpenFolders, &QAction::triggered, this, [this] {
        const QString left = QFileDialog::getExistingDirectory(
            this,
            "Open Left Folder",
            QString::fromStdString(m_invocation.leftPath.empty() ? std::string() : m_invocation.leftPath.string()),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (left.isEmpty()) {
            return;
        }

        const QString right = QFileDialog::getExistingDirectory(
            this,
            "Open Right Folder",
            QString::fromStdString(m_invocation.rightPath.empty() ? std::string() : m_invocation.rightPath.string()),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (right.isEmpty()) {
            return;
        }

        enter_folder_diff_mode(std::filesystem::path(left.toStdString()), std::filesystem::path(right.toStdString()));
    });
}

void MainWindow::setup_toolbar()
{
    auto* toolbar = addToolBar("Main");
    toolbar->setMovable(false);

    // Re-use the File menu actions so they stay consistent.
    m_actionOpenRepo = m_actionOpenRepo ? m_actionOpenRepo : new QAction("Open Repo...", this);
    m_actionOpenFolders = m_actionOpenFolders ? m_actionOpenFolders : new QAction("Open Folders...", this);
    m_actionRefresh = new QAction("Refresh", this);
    m_actionNextChange = new QAction("Next Change", this);
    m_actionPrevChange = new QAction("Previous Change", this);

    // v1 refresh semantics:
    // - Repo mode: automatic refresh comes later (Milestone 7), so this is a no-op.
    // - Folder diff mode: manual refresh via toolbar or F5.
    m_actionRefresh->setShortcut(QKeySequence(Qt::Key_F5));
    m_actionRefresh->setShortcutContext(Qt::ApplicationShortcut);

    toolbar->addAction(m_actionOpenRepo);
    toolbar->addAction(m_actionOpenFolders);
    toolbar->addSeparator();
    toolbar->addAction(m_actionRefresh);
    toolbar->addSeparator();
    toolbar->addAction(m_actionPrevChange);
    toolbar->addAction(m_actionNextChange);
    toolbar->addSeparator();

    m_whitespaceCombo = new QComboBox(toolbar);
    m_whitespaceCombo->addItem("Exact");
    m_whitespaceCombo->addItem("Ignore trailing whitespace");
    m_whitespaceCombo->addItem("Ignore all whitespace");
    toolbar->addWidget(m_whitespaceCombo);
    toolbar->addSeparator();

    auto* paneModeGroup = new QActionGroup(this);
    paneModeGroup->setExclusive(true);

    m_actionInlineMode = new QAction("Inline diff (2-pane)", this);
    m_actionSideBySideMode = new QAction("Side-by-side diff (3-pane)", this);
    m_actionInlineMode->setCheckable(true);
    m_actionSideBySideMode->setCheckable(true);
    m_actionInlineMode->setChecked(true);

    paneModeGroup->addAction(m_actionInlineMode);
    paneModeGroup->addAction(m_actionSideBySideMode);

    toolbar->addAction(m_actionInlineMode);
    toolbar->addAction(m_actionSideBySideMode);

    connect(m_actionRefresh, &QAction::triggered, this, [this] {
        if (m_invocation.mode == bendiff::AppMode::FolderDiffMode) {
            refresh_file_list();
            reset_placeholders();
            update_status_bar();
            return;
        }

        // Repo mode refresh is automatic in a later milestone.
        bendiff::logging::info("Refresh requested in repo mode (no-op in v1 milestones before M7)");
        statusBar()->showMessage("Repo refresh is automatic (manual refresh not implemented yet)");
    });
    connect(m_actionNextChange, &QAction::triggered, this, [] { log_not_implemented("Next Change"); });
    connect(m_actionPrevChange, &QAction::triggered, this, [] { log_not_implemented("Previous Change"); });

    connect(m_whitespaceCombo, &QComboBox::currentTextChanged, this, [](const QString& text) {
        bendiff::logging::info(std::string("Whitespace mode set to: ") + text.toStdString());
    });

    connect(m_actionInlineMode, &QAction::triggered, this, [this] {
        set_pane_mode(PaneMode::Inline);
    });
    connect(m_actionSideBySideMode, &QAction::triggered, this, [this] {
        set_pane_mode(PaneMode::SideBySide);
    });
}

void MainWindow::setup_central()
{
    // Central placeholder layout:
    // - Left pane: file list placeholder
    // - Right side: diff placeholder container (one or two panes will be implemented in M1-T2)
    m_rootSplitter = new QSplitter(Qt::Horizontal, this);

    // Left pane: placeholder file list (real widget, selectable)
    {
        auto* leftFrame = new QFrame(m_rootSplitter);
        leftFrame->setFrameShape(QFrame::StyledPanel);
        leftFrame->setFrameShadow(QFrame::Sunken);

        auto* leftLayout = new QVBoxLayout(leftFrame);
        leftLayout->setContentsMargins(8, 8, 8, 8);
        leftLayout->setSpacing(8);

        auto* header = new QLabel("Files", leftFrame);
        header->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        m_fileListWidget = new QListWidget(leftFrame);
        m_fileListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

        leftLayout->addWidget(header);
        leftLayout->addWidget(m_fileListWidget, 1);

        m_rootSplitter->addWidget(leftFrame);
    }

    m_diffSplitter = new QSplitter(Qt::Horizontal, m_rootSplitter);
    m_diffPaneA = make_placeholder_panel("Diff view (placeholder)", &m_diffLabelA, m_diffSplitter);
    m_diffPaneB = make_placeholder_panel("Diff view (placeholder)", &m_diffLabelB, m_diffSplitter);
    m_diffSplitter->addWidget(m_diffPaneA);
    m_diffSplitter->addWidget(m_diffPaneB);

    // Default to inline mode (2-pane topology): left file list + one diff pane.
    // Side-by-side (3-pane) is implemented via showing the second diff pane.
    if (m_diffPaneB) {
        m_diffPaneB->setVisible(false);
    }
    m_diffSplitter->setStretchFactor(0, 1);
    m_diffSplitter->setStretchFactor(1, 1);

    m_rootSplitter->addWidget(m_diffSplitter);
    m_rootSplitter->setStretchFactor(0, 0);
    m_rootSplitter->setStretchFactor(1, 1);
    m_rootSplitter->setSizes({240, 760});

    setCentralWidget(m_rootSplitter);

    // Selection wiring (M1-T6): selecting a file updates diff placeholder labels.
    // Selection wiring (M1-T6/M2-T7): selecting a file updates diff placeholder labels.
    if (m_fileListWidget) {
        connect(m_fileListWidget, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* current, QListWidgetItem*) {
            if (current == nullptr) {
                reset_placeholders();
                return;
            }

            // Header rows are non-selectable, but be defensive.
            if ((current->flags() & Qt::ItemIsSelectable) == 0) {
                reset_placeholders();
                return;
            }

            if (m_invocation.mode == bendiff::AppMode::RepoMode) {
                const QString path = current->data(Qt::UserRole).toString();
                const int kindInt = current->data(Qt::UserRole + 1).toInt();
                const QString renameFrom = current->data(Qt::UserRole + 2).toString();

                if (path.isEmpty()) {
                    reset_placeholders();
                    return;
                }

                const auto kind = static_cast<bendiff::core::ChangeKind>(kindInt);
                bendiff::logging::info(std::string("Selected repo file: ") + path.toStdString() + " kind=" + to_string(kind));

                QString detail = QString("%1\nKind: %2").arg(path).arg(to_string(kind));
                if (!renameFrom.isEmpty()) {
                    detail += QString("\nRenamed from: %1").arg(renameFrom);
                }

                if (m_diffLabelA && m_diffLabelB) {
                    if (m_paneMode == PaneMode::Inline) {
                        m_diffLabelA->setText(QString("Inline diff placeholder\n\n%1").arg(detail));
                        m_diffLabelB->setText(QString());
                    } else {
                        m_diffLabelA->setText(QString("Side-by-side diff placeholder (left)\n\n%1").arg(detail));
                        m_diffLabelB->setText(QString("Side-by-side diff placeholder (right)\n\n%1").arg(detail));
                    }
                }
            } else if (m_invocation.mode == bendiff::AppMode::FolderDiffMode) {
                const QString rel = current->data(Qt::UserRole).toString();
                const int statusInt = current->data(Qt::UserRole + 1).toInt();
                const QString leftFull = current->data(Qt::UserRole + 2).toString();
                const QString rightFull = current->data(Qt::UserRole + 3).toString();

                if (rel.isEmpty()) {
                    reset_placeholders();
                    return;
                }

                const auto status = static_cast<bendiff::core::DirEntryStatus>(statusInt);
                bendiff::logging::info(std::string("Selected folder entry: ") + rel.toStdString() + " status=" + to_string(status));

                const QString detail = QString("%1\nStatus: %2\n\nLeft: %3\nRight: %4")
                                           .arg(rel)
                                           .arg(to_string(status))
                                           .arg(leftFull.isEmpty() ? QString("(missing)") : leftFull)
                                           .arg(rightFull.isEmpty() ? QString("(missing)") : rightFull);

                if (m_diffLabelA && m_diffLabelB) {
                    if (m_paneMode == PaneMode::Inline) {
                        m_diffLabelA->setText(QString("Folder diff placeholder\n\n%1").arg(detail));
                        m_diffLabelB->setText(QString());
                    } else {
                        m_diffLabelA->setText(QString("Folder diff placeholder (left)\n\n%1").arg(detail));
                        m_diffLabelB->setText(QString("Folder diff placeholder (right)\n\n%1").arg(detail));
                    }
                }
            } else {
                reset_placeholders();
            }
        });

        // Double-click wiring (M3-T7): in folder diff mode, open a placeholder diff view.
        connect(m_fileListWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
            if (item == nullptr) {
                return;
            }

            // Be defensive: repo mode has non-selectable header rows.
            if ((item->flags() & Qt::ItemIsSelectable) == 0) {
                return;
            }

            if (m_invocation.mode != bendiff::AppMode::FolderDiffMode) {
                return;
            }

            const QString rel = item->data(Qt::UserRole).toString();
            const int statusInt = item->data(Qt::UserRole + 1).toInt();
            const QString leftFull = item->data(Qt::UserRole + 2).toString();
            const QString rightFull = item->data(Qt::UserRole + 3).toString();

            if (rel.isEmpty()) {
                return;
            }

            const auto status = static_cast<bendiff::core::DirEntryStatus>(statusInt);

            QString header = QString("Folder diff placeholder\n\n%1\nStatus: %2")
                                 .arg(rel)
                                 .arg(to_string(status));

            const bool canDiffBothSides = (status == bendiff::core::DirEntryStatus::Same ||
                                          status == bendiff::core::DirEntryStatus::Different);

            if (!canDiffBothSides) {
                header += "\n\nCannot diff both sides for this entry.";
            }

            const QString detail = QString("\n\nLeft: %1\nRight: %2")
                                       .arg(leftFull.isEmpty() ? QString("(missing)") : leftFull)
                                       .arg(rightFull.isEmpty() ? QString("(missing)") : rightFull);

            if (m_diffLabelA && m_diffLabelB) {
                if (m_paneMode == PaneMode::Inline) {
                    m_diffLabelA->setText(header + detail);
                    m_diffLabelB->setText(QString());
                } else {
                    m_diffLabelA->setText(header + "\n\n(left pane)" + detail);
                    m_diffLabelB->setText(header + "\n\n(right pane)" + detail);
                }
            }
        });
    }
}

void MainWindow::enter_repo_mode(const std::filesystem::path& repoPath)
{
    m_invocation.mode = bendiff::AppMode::RepoMode;
    m_invocation.repoPath = repoPath;
    m_invocation.leftPath.clear();
    m_invocation.rightPath.clear();
    m_invocation.error.clear();

    bendiff::logging::info(std::string("UI mode set: RepoMode repoPath=\"") + repoPath.string() + "\"");
    refresh_repo_discovery();
    refresh_file_list();
    reset_placeholders();
    update_status_bar();
}

void MainWindow::enter_folder_diff_mode(const std::filesystem::path& leftPath, const std::filesystem::path& rightPath)
{
    m_invocation.mode = bendiff::AppMode::FolderDiffMode;
    m_invocation.leftPath = leftPath;
    m_invocation.rightPath = rightPath;
    m_invocation.repoPath.clear();
    m_invocation.error.clear();

    bendiff::logging::info(std::string("UI mode set: FolderDiffMode leftPath=\"") + leftPath.string() +
                           "\" rightPath=\"" + rightPath.string() + "\"");

    m_repoRoot.reset();
    refresh_file_list();
    reset_placeholders();
    update_status_bar();
}

void MainWindow::refresh_file_list()
{
    if (!m_fileListWidget) {
        return;
    }

    m_fileListWidget->blockSignals(true);
    m_fileListWidget->clear();

    if (m_invocation.mode == bendiff::AppMode::RepoMode) {
        // If not a git repo, no files are listed.
        if (!m_repoRoot.has_value()) {
            m_fileListWidget->blockSignals(false);
            return;
        }

        const auto r = bendiff::core::GetRepoStatusWithDiagnostics(*m_repoRoot);
        if (r.process.exitCode != 0) {
            const bool cannotExec = (r.process.exitCode == 127);

            const QString title = cannotExec ? "Git could not be executed" : "Git status failed";
            QString message;
            if (cannotExec) {
                message = "BenDiff could not execute the Git executable.\n\n"
                          "Make sure `git` is installed and available on PATH.";
            } else {
                message = QString("Git returned a non-zero exit code (%1) while reading repository status.")
                              .arg(r.process.exitCode);
            }

            if (!r.process.stderrText.empty()) {
                message += QString("\n\nDetails:\n%1").arg(QString::fromStdString(r.process.stderrText));
            }

            QMessageBox::critical(this, title, message);

            // Keep list empty on failure.
            m_fileListWidget->blockSignals(false);
            return;
        }

        const auto rows = bendiff::core::BuildGroupedFileListRows(r.status.files);

        for (const auto& row : rows) {
            auto* item = new QListWidgetItem(QString::fromStdString(row.displayText));

            if (row.kind == bendiff::core::FileListRowKind::Header) {
                QFont f = item->font();
                f.setBold(true);
                item->setFont(f);
                item->setForeground(QBrush(QColor(120, 120, 120)));
                item->setBackground(QBrush(QColor(245, 245, 245)));

                // Non-selectable header.
                item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
                item->setData(Qt::UserRole, QString());
                item->setData(Qt::UserRole + 1, static_cast<int>(bendiff::core::ChangeKind::Unknown));
                item->setData(Qt::UserRole + 2, QString());
            } else {
                // File row: store metadata for selection handling.
                if (row.file.has_value()) {
                    const auto& f = *row.file;
                    item->setData(Qt::UserRole, QString::fromStdString(f.repoRelativePath));
                    item->setData(Qt::UserRole + 1, static_cast<int>(f.kind));
                    item->setData(Qt::UserRole + 2, f.renameFrom.has_value() ? QString::fromStdString(*f.renameFrom) : QString());

                    // Annotate renames in the UI list itself.
                    if (f.kind == bendiff::core::ChangeKind::Renamed && f.renameFrom.has_value()) {
                        item->setText(QString("%1 (renamed from %2)")
                                          .arg(QString::fromStdString(f.repoRelativePath))
                                          .arg(QString::fromStdString(*f.renameFrom)));
                    }
                }
            }

            m_fileListWidget->addItem(item);
        }
    } else if (m_invocation.mode == bendiff::AppMode::FolderDiffMode) {
        if (!validate_dir_path(m_invocation.leftPath) || !validate_dir_path(m_invocation.rightPath)) {
            QMessageBox::critical(
                this,
                "Folder diff failed",
                QString("One or both folder paths are invalid.\n\nLeft: %1\nRight: %2")
                    .arg(QString::fromStdString(m_invocation.leftPath.string()))
                    .arg(QString::fromStdString(m_invocation.rightPath.string())));

            m_fileListWidget->blockSignals(false);
            return;
        }

        const auto diff = bendiff::core::DiffDirectories(m_invocation.leftPath, m_invocation.rightPath);
        for (const auto& e : diff.entries) {
            const QString rel = QString::fromStdString(e.relativePath);
            const QString statusText = QString::fromLatin1(to_string(e.status));

            QString display = rel;
            display += QString(" [%1]").arg(statusText);

            auto* item = new QListWidgetItem(display);

            // Store metadata for selection handling.
            item->setData(Qt::UserRole, rel);
            item->setData(Qt::UserRole + 1, static_cast<int>(e.status));

            const std::filesystem::path leftFull = diff.leftRoot / std::filesystem::path(e.relativePath).make_preferred();
            const std::filesystem::path rightFull = diff.rightRoot / std::filesystem::path(e.relativePath).make_preferred();

            if (e.status == bendiff::core::DirEntryStatus::RightOnly) {
                item->setData(Qt::UserRole + 2, QString());
                item->setData(Qt::UserRole + 3, QString::fromStdString(rightFull.string()));
            } else if (e.status == bendiff::core::DirEntryStatus::LeftOnly) {
                item->setData(Qt::UserRole + 2, QString::fromStdString(leftFull.string()));
                item->setData(Qt::UserRole + 3, QString());
            } else {
                item->setData(Qt::UserRole + 2, QString::fromStdString(leftFull.string()));
                item->setData(Qt::UserRole + 3, QString::fromStdString(rightFull.string()));
            }

            // Visual hint for status.
            if (e.status == bendiff::core::DirEntryStatus::Different) {
                item->setForeground(QBrush(QColor(120, 60, 0)));
            } else if (e.status == bendiff::core::DirEntryStatus::Unreadable) {
                item->setForeground(QBrush(QColor(160, 0, 0)));
            }

            m_fileListWidget->addItem(item);
        }
    } else {
        m_fileListWidget->addItem("(no mode selected)");
    }

    m_fileListWidget->blockSignals(false);
}

void MainWindow::refresh_repo_discovery()
{
    m_repoRoot.reset();

    if (m_invocation.mode != bendiff::AppMode::RepoMode) {
        return;
    }

    // v1: repo discovery is simply walking up parents looking for a .git directory.
    // No git execution here (that's M2-T3/M2-T4).
    const auto root = bendiff::core::FindGitRepoRoot(m_invocation.repoPath);
    if (root.has_value()) {
        m_repoRoot = *root;
        bendiff::logging::info(std::string("Repo root found: \"") + m_repoRoot->string() + "\"");
    } else {
        bendiff::logging::info("No git repo found for repo mode start path");
    }
}

void MainWindow::reset_placeholders()
{
    // No real logic yet; just keep the UI consistent and visibly reset.

    if (m_fileListWidget) {
        m_fileListWidget->clearSelection();
        m_fileListWidget->blockSignals(false);
    }

    if (m_diffLabelA && m_diffLabelB) {
        if (m_paneMode == PaneMode::Inline) {
            m_diffLabelA->setText("Diff view (inline placeholder)");
            m_diffLabelB->setText("Diff view (placeholder)");
        } else {
            m_diffLabelA->setText("Diff view (left placeholder)");
            m_diffLabelB->setText("Diff view (right placeholder)");
        }
    }
}

void MainWindow::set_pane_mode(PaneMode mode)
{
    m_paneMode = mode;

    if (m_actionInlineMode && m_actionSideBySideMode) {
        if (mode == PaneMode::Inline) {
            m_actionInlineMode->setChecked(true);
        } else {
            m_actionSideBySideMode->setChecked(true);
        }
    }

    const bool showSecondPane = (mode == PaneMode::SideBySide);
    if (m_diffPaneB) {
        m_diffPaneB->setVisible(showSecondPane);
    }

    if (m_diffLabelA && m_diffLabelB) {
        if (mode == PaneMode::Inline) {
            m_diffLabelA->setText("Diff view (inline placeholder)");
            m_diffLabelB->setText("Diff view (placeholder)");
        } else {
            m_diffLabelA->setText("Diff view (left placeholder)");
            m_diffLabelB->setText("Diff view (right placeholder)");
        }
    }

    if (m_rootSplitter) {
        // Keep the file list pane visible; just adjust the distribution a bit.
        m_rootSplitter->setSizes({240, 760});
    }

    // If nothing is selected, ensure placeholders reflect the new topology.
    if (!m_fileListWidget || m_fileListWidget->currentRow() < 0) {
        reset_placeholders();
    }

    update_status_bar();
}

void MainWindow::update_status_bar()
{
    const char* paneText = (m_paneMode == PaneMode::Inline) ? "Inline" : "Side-by-side";

    QString modeText = "File";
    QString paths;
    if (m_invocation.mode == bendiff::AppMode::RepoMode) {
        modeText = "File";
        if (m_repoRoot.has_value()) {
            paths = QString("Repo root: %1").arg(QString::fromStdString(m_repoRoot->string()));
        } else {
            paths = "No git repo found";
        }
    } else if (m_invocation.mode == bendiff::AppMode::FolderDiffMode) {
        modeText = "Folder";
        if (!m_invocation.leftPath.empty() || !m_invocation.rightPath.empty()) {
            paths = QString("Left: %1 | Right: %2")
                        .arg(QString::fromStdString(m_invocation.leftPath.string()))
                        .arg(QString::fromStdString(m_invocation.rightPath.string()));
        }
    }

    QString message = QString("Mode: %1 | View: %2").arg(modeText).arg(paneText);
    if (!paths.isEmpty()) {
        message += " | ";
        message += paths;
    }
    statusBar()->showMessage(message);
}
