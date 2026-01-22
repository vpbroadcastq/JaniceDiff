#pragma once

#include <QMainWindow>

#include <invocation.h>

#include <optional>

class QAction;
class QComboBox;
class QLabel;
class QListWidget;
class QSplitter;
class QWidget;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const bendiff::Invocation& invocation, QWidget* parent = nullptr);

private:
    enum class PaneMode {
        Inline,
        SideBySide,
    };

    void setup_menus();
    void setup_toolbar();
    void setup_central();

    void enter_repo_mode(const std::filesystem::path& repoPath);
    void enter_folder_diff_mode(const std::filesystem::path& leftPath, const std::filesystem::path& rightPath);
    void refresh_file_list();
    void reset_placeholders();

    void refresh_repo_discovery();

    void set_pane_mode(PaneMode mode);
    void update_status_bar();

    bendiff::Invocation m_invocation;
    PaneMode m_paneMode = PaneMode::Inline;

    std::optional<std::filesystem::path> m_repoRoot;

    QAction* m_actionOpenRepo = nullptr;
    QAction* m_actionOpenFolders = nullptr;
    QAction* m_actionRefresh = nullptr;
    QAction* m_actionNextChange = nullptr;
    QAction* m_actionPrevChange = nullptr;

    QAction* m_actionInlineMode = nullptr;
    QAction* m_actionSideBySideMode = nullptr;

    QComboBox* m_whitespaceCombo = nullptr;

    QSplitter* m_rootSplitter = nullptr;
    QSplitter* m_diffSplitter = nullptr;

    QListWidget* m_fileListWidget = nullptr;

    QWidget* m_diffPaneA = nullptr;
    QWidget* m_diffPaneB = nullptr;
    QLabel* m_diffLabelA = nullptr;
    QLabel* m_diffLabelB = nullptr;
};
