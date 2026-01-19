#include "MainWindow.h"

#include <logging.h>

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QFileDialog>
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

    connect(m_actionRefresh, &QAction::triggered, this, [] { log_not_implemented("Refresh"); });
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

    auto* fileList = make_placeholder_panel("File list (placeholder)", &m_fileListLabel, m_rootSplitter);
    m_rootSplitter->addWidget(fileList);

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
}

void MainWindow::enter_repo_mode(const std::filesystem::path& repoPath)
{
    m_invocation.mode = bendiff::AppMode::RepoMode;
    m_invocation.repoPath = repoPath;
    m_invocation.leftPath.clear();
    m_invocation.rightPath.clear();
    m_invocation.error.clear();

    bendiff::logging::info(std::string("UI mode set: RepoMode repoPath=\"") + repoPath.string() + "\"");
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
    reset_placeholders();
    update_status_bar();
}

void MainWindow::reset_placeholders()
{
    // No real logic yet; just keep the UI consistent and visibly reset.
    if (m_fileListLabel) {
        if (m_invocation.mode == bendiff::AppMode::RepoMode) {
            m_fileListLabel->setText("File list (repo placeholder)");
        } else if (m_invocation.mode == bendiff::AppMode::FolderDiffMode) {
            m_fileListLabel->setText("File list (folder diff placeholder)");
        } else {
            m_fileListLabel->setText("File list (placeholder)");
        }
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

    update_status_bar();
}

void MainWindow::update_status_bar()
{
    const char* paneText = (m_paneMode == PaneMode::Inline) ? "Inline" : "Side-by-side";

    QString modeText = "File";
    QString paths;
    if (m_invocation.mode == bendiff::AppMode::RepoMode) {
        modeText = "File";
        if (!m_invocation.repoPath.empty()) {
            paths = QString("Repo: %1").arg(QString::fromStdString(m_invocation.repoPath.string()));
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
