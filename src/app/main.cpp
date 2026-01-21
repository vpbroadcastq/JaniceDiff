#include "MainWindow.h"

#include <invocation.h>
#include <logging.h>
#include <QApplication>

#include <sstream>

int main(int argc, char** argv)
{
    bendiff::logging::init();
    bendiff::logging::info("BenDiff starting...");

    QApplication app(argc, argv);

    // M1-T3: Command-line parsing and invocation classification.
    // Parse Qt-stripped arguments (excludes Qt-specific flags like -platform).
    {
        const auto qargs = QCoreApplication::arguments();
        std::vector<std::string> args;
        args.reserve(static_cast<std::size_t>(qargs.size() > 0 ? (qargs.size() - 1) : 0));
        for (int i = 1; i < qargs.size(); ++i) {
            args.push_back(qargs[i].toStdString());
        }

        const bendiff::Invocation invocation = bendiff::parse_invocation(args);

        std::ostringstream msg;
        msg << "Invocation: mode=" << bendiff::to_string(invocation.mode);
        if (invocation.mode == bendiff::AppMode::RepoMode) {
            msg << " repoPath=\"" << invocation.repoPath.string() << "\"";
        } else if (invocation.mode == bendiff::AppMode::FolderDiffMode) {
            msg << " leftPath=\"" << invocation.leftPath.string() << "\"";
            msg << " rightPath=\"" << invocation.rightPath.string() << "\"";
        } else {
            msg << " error=\"" << invocation.error << "\"";
        }
        bendiff::logging::info(msg.str());
    }

    MainWindow window;
    window.show();

    return app.exec();
}
