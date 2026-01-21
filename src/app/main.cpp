#include "MainWindow.h"

#include <invocation.h>
#include <logging.h>
#include <startup_policy.h>
#include <QApplication>
#include <QMessageBox>

#include <sstream>
#include <cstdlib>

int main(int argc, char** argv)
{
    bendiff::logging::init();
    bendiff::logging::info("BenDiff starting...");

    QApplication app(argc, argv);

    // M1-T3: Command-line parsing and invocation classification.
    // Parse Qt-stripped arguments (excludes Qt-specific flags like -platform).
    bendiff::Invocation invocation;
    {
        const auto qargs = QCoreApplication::arguments();
        std::vector<std::string> args;
        args.reserve(static_cast<std::size_t>(qargs.size() > 0 ? (qargs.size() - 1) : 0));
        for (int i = 1; i < qargs.size(); ++i) {
            args.push_back(qargs[i].toStdString());
        }

        invocation = bendiff::parse_invocation(args);

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

    // M1-T4: Error handling and exit-code policy.
    // Force a runtime startup error via env var for now (simulated path).
    // Example: BENDIFF_FORCE_STARTUP_ERROR=1 bendiff
    const bool forceRuntimeError = (std::getenv("BENDIFF_FORCE_STARTUP_ERROR") != nullptr);

    if (const auto startupError = bendiff::startup_error_for(invocation, forceRuntimeError)) {
        bendiff::logging::error(startupError->title + ": " + startupError->message);
        QMessageBox::critical(nullptr,
                              QString::fromStdString(startupError->title),
                              QString::fromStdString(startupError->message));
        return startupError->exitCode;
    }

    MainWindow window(invocation);
    window.show();

    return app.exec();
}
