#include "app/application.h"
#include "core/log_sink.h"
#include "core/settings/settings.h"
#include "ui/main_window.h"

#include <QCoreApplication>

int main(int argc, char* argv[])
{
    // Capture everything from the first line on; the file sink is attached
    // once the application identity (and thus the log location) is known.
    whatsie::core::LogSink::install();

    // Required by Qt WebEngine before the QApplication exists.
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    whatsie::app::Application app(argc, argv);
    if (app.shouldExit()) {
        return app.exitCode();
    }

    whatsie::ui::MainWindow window(app.settings(), app.themeService());
    QObject::connect(&app, &whatsie::app::Application::raiseRequested, &window,
                     &whatsie::ui::MainWindow::showAndRaise);
    QObject::connect(&app, &whatsie::app::Application::newChatRequested, &window,
                     &whatsie::ui::MainWindow::openChat);
    QObject::connect(&app, &whatsie::app::Application::settingsRequested, &window,
                     &whatsie::ui::MainWindow::showSettings);
    QObject::connect(&app, &whatsie::app::Application::quitRequested, &window,
                     &whatsie::ui::MainWindow::quit);

    const whatsie::app::CliOptions& cli = app.cliOptions();
    window.start(cli.startMinimized || app.settings().startMinimized());

    // Commands given on our own command line (we are the primary instance).
    for (const QJsonObject& command : whatsie::app::commandsFor(cli)) {
        if (command.value(QLatin1StringView(whatsie::app::cmd::kKey)).toString() !=
            QLatin1StringView(whatsie::app::cmd::kRaise)) {
            app.dispatchCommand(command);
        }
    }

    return whatsie::app::Application::exec();
}
