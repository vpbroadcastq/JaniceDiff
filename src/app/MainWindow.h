#pragma once

#include <QMainWindow>

class QAction;
class QComboBox;
class QSplitter;

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    void setup_menus();
    void setup_toolbar();
    void setup_central();

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
};
