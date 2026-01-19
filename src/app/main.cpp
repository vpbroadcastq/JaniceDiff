#include "MainWindow.h"

#include <logging.h>
#include <QApplication>

int main(int argc, char** argv)
{
    bendiff::logging::init();
    bendiff::logging::info("BenDiff starting...");

    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
