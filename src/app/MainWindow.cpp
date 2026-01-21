#include "MainWindow.h"

#include <logging.h>

#include <QAction>
#include <QActionGroup>
#include <QComboBox>
#include <QFrame>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QWidget* make_placeholder_panel(const QString& title, QWidget* parent)
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

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("BenDiff");

    setup_menus();
    setup_toolbar();
    setup_central();

    statusBar()->showMessage("Ready");
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

    connect(m_actionOpenRepo, &QAction::triggered, this, [] { log_not_implemented("Open Repo"); });
    connect(m_actionOpenFolders, &QAction::triggered, this, [] { log_not_implemented("Open Folders"); });
}

void MainWindow::setup_toolbar()
{
    auto* toolbar = addToolBar("Main");
    toolbar->setMovable(false);

    m_actionOpenRepo = m_actionOpenRepo ? m_actionOpenRepo : new QAction("Open Repo", this);
    m_actionOpenFolders = m_actionOpenFolders ? m_actionOpenFolders : new QAction("Open Folders", this);
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

    connect(m_actionInlineMode, &QAction::triggered, this, [] { log_not_implemented("Switch to inline diff (2-pane)"); });
    connect(m_actionSideBySideMode, &QAction::triggered, this, [] { log_not_implemented("Switch to side-by-side diff (3-pane)"); });
}

void MainWindow::setup_central()
{
    // Central placeholder layout:
    // - Left pane: file list placeholder
    // - Right side: diff placeholder container (one or two panes will be implemented in M1-T2)
    m_rootSplitter = new QSplitter(Qt::Horizontal, this);

    auto* fileList = make_placeholder_panel("File list (placeholder)", m_rootSplitter);
    m_rootSplitter->addWidget(fileList);

    m_diffSplitter = new QSplitter(Qt::Horizontal, m_rootSplitter);
    auto* diffA = make_placeholder_panel("Diff view (placeholder)", m_diffSplitter);
    auto* diffB = make_placeholder_panel("Diff view (placeholder)", m_diffSplitter);
    m_diffSplitter->addWidget(diffA);
    m_diffSplitter->addWidget(diffB);

    // Default to an "inline" feel: keep the second diff pane hidden for now.
    // Mode switching behavior is implemented in M1-T2.
    diffB->setVisible(false);
    m_diffSplitter->setStretchFactor(0, 1);
    m_diffSplitter->setStretchFactor(1, 1);

    m_rootSplitter->addWidget(m_diffSplitter);
    m_rootSplitter->setStretchFactor(0, 0);
    m_rootSplitter->setStretchFactor(1, 1);
    m_rootSplitter->setSizes({240, 760});

    setCentralWidget(m_rootSplitter);
}
