#include "MainWindow.h"

#include <QStatusBar>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("BenDiff");

    auto* central = new QWidget(this);
    setCentralWidget(central);

    statusBar()->showMessage("Ready");
}
