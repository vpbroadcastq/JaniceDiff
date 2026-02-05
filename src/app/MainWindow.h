#pragma once

#include <QMainWindow>

#include <diff/diff.h>
#include <navigation/change_navigation.h>
#include <render/diff_render_model.h>

#include <invocation.h>

#include <optional>
#include <vector>

class QAction;
class QComboBox;
class QListWidget;
class QSplitter;
class QTimer;
class QWidget;

class DiffTextView;

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

    void update_repo_auto_refresh_timer();
    void repo_auto_refresh_tick(bool force);

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
    DiffTextView* m_diffTextA = nullptr;
    DiffTextView* m_diffTextB = nullptr;

    bool m_syncingDiffScroll = false;

    QTimer* m_repoRefreshTimer = nullptr;
    bool m_repoRefreshInProgress = false;
    bool m_repoAutoRefreshSuppressed = false;
    std::string m_lastRepoStatusSignature;

    // M7: cached navigation state for the currently selected item.
    std::optional<bendiff::core::diff::DiffResult> m_currentDiff;
    std::optional<bendiff::core::render::RenderDocument> m_currentRenderDoc;
    std::vector<bendiff::core::navigation::ChangeLocation> m_currentChanges;
    std::optional<std::size_t> m_currentChangeIndex;

    bool m_currentSelectionUnsupported = false;
};
