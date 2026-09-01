#include "app/application.h"
#include "app/version.h"
#include "core/graphics_fallback.h"
#include "core/log_sink.h"
#include "core/settings/settings.h"
#include "core/settings/settings_keys.h"
#include "ui/main_window.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QProcess>
#include <QSettings>
#include <QTimer>

#include <algorithm>
#include <atomic>
#include <cmath>

namespace {

// Interface scale (FEATURES A7) must reach Qt through QT_SCALE_FACTOR, which is
// read once during QApplication construction — so we peek at the stored value
// before the application object exists. This mirrors the default profile only;
// a per-profile override would need the profile parsed here, which is not worth
// the complexity for a global appearance preference.
void applyInterfaceScaleEnv()
{
    if (qEnvironmentVariableIsSet("QT_SCALE_FACTOR")) {
        return; // never override an explicit user/env value
    }
    const QSettings store(QString::fromLatin1(whatsie::app::version::kOrganizationName),
                          QString::fromLatin1(whatsie::app::version::kApplicationName));
    const double stored = store.value(whatsie::core::keys::kInterfaceScale, 1.0).toDouble();
    const double scale = std::clamp(stored, whatsie::core::Settings::kMinInterfaceScale,
                                    whatsie::core::Settings::kMaxInterfaceScale);
    if (std::abs(scale - 1.0) > 0.001) {
        qputenv("QT_SCALE_FACTOR", QByteArray::number(scale, 'g', 4));
    }
}

// FEATURES S20: watch the log stream for a graphics-backend init failure so we
// can fall back from a broken Wayland RHI to XCB, once.
std::atomic<bool> g_graphicsFailed{false};
QtMessageHandler g_previousHandler = nullptr;

void graphicsWatchHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    if (whatsie::core::isGraphicsInitFailure(message)) {
        g_graphicsFailed.store(true);
    }
    if (g_previousHandler != nullptr) {
        g_previousHandler(type, context, message);
    }
}

void relaunchUnderXcb()
{
    qputenv("QT_QPA_PLATFORM", "xcb");
    qputenv("WHATSIE_XCB_RETRY", "1");
    const QStringList args = QCoreApplication::arguments().mid(1);
    if (QProcess::startDetached(QCoreApplication::applicationFilePath(), args)) {
        QCoreApplication::quit();
    }
}

} // namespace

int main(int argc, char* argv[])
{
    // Capture everything from the first line on; the file sink is attached
    // once the application identity (and thus the log location) is known.
    whatsie::core::LogSink::install();

    // Both must run before the QApplication constructor reads them.
    applyInterfaceScaleEnv();
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    whatsie::app::Application app(argc, argv);
    if (app.shouldExit()) {
        return app.exitCode();
    }

    // Chain onto the sink Application just installed, so a later graphics failure
    // during window/WebEngine creation is observed (FEATURES S20).
    g_previousHandler = qInstallMessageHandler(graphicsWatchHandler);

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

    // If the GPU is on trial (ADR-032), a clean 20 s means it is stable — clear
    // the crash probe so the next start is not treated as a GPU crash.
    QTimer::singleShot(20000, &app, [&app] { app.markGpuStable(); });

    // Give the GPU stack time to fail, then fall back to XCB if it did (once).
    const bool retried = qEnvironmentVariableIsSet("WHATSIE_XCB_RETRY");
    if (!retried && QGuiApplication::platformName() == QLatin1StringView("wayland")) {
        QTimer::singleShot(3000, &window, [] {
            if (whatsie::core::shouldRetryUnderXcb(QLatin1StringView("wayland"), false,
                                                   g_graphicsFailed.load())) {
                relaunchUnderXcb();
            }
        });
    }

    // Commands given on our own command line (we are the primary instance).
    for (const QJsonObject& command : whatsie::app::commandsFor(cli)) {
        if (command.value(QLatin1StringView(whatsie::app::cmd::kKey)).toString() !=
            QLatin1StringView(whatsie::app::cmd::kRaise)) {
            app.dispatchCommand(command);
        }
    }

    return whatsie::app::Application::exec();
}
