#include "app/application.h"
#include "app/single_instance.h"
#include "app/version.h"
#include "core/graphics_fallback.h"
#include "core/log_sink.h"
#include "core/settings/settings.h"
#include "core/settings/settings_keys.h"
#include "platform/crash_handler.h"
#include "platform/gpu_stderr_watch.h"
#include "ui/main_window.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QGuiApplication>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

#ifdef Q_OS_UNIX
#include <QSocketNotifier>
#include <csignal>
#include <unistd.h>
#endif

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

#ifdef Q_OS_UNIX
// Self-pipe so SIGTERM/SIGINT (session logout, shutdown, Ctrl+C) become a
// graceful QApplication quit instead of an abrupt death. Normal teardown then
// runs — settings are synced and the GPU-probe marker is cleared — so an
// ordinary shutdown within the ~20 s GPU trial window is not later miscounted
// as a crash strike (which, twice, would wrongly force software rendering).
int g_termPipe[2] = {-1, -1};

void writeTermSignal(int /*signum*/)
{
    const char byte = 1;
    const ssize_t ignored = ::write(g_termPipe[1], &byte, 1);
    static_cast<void>(ignored); // async-signal-safe; nothing useful to do on error
}

int installGracefulTermination()
{
    if (::pipe(g_termPipe) != 0) {
        return -1;
    }
    struct sigaction action{};
    action.sa_handler = writeTermSignal;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART;
    sigaction(SIGTERM, &action, nullptr);
    sigaction(SIGINT, &action, nullptr);
    return g_termPipe[0];
}
#endif

void relaunchUnderXcb(whatsie::app::Application& app)
{
    qputenv("QT_QPA_PLATFORM", "xcb");
    qputenv("WHATSIE_XCB_RETRY", "1");
    const QStringList args = QCoreApplication::arguments().mid(1);
    // Drop the single-instance lock first: otherwise the XCB child sees this
    // still-listening process as the primary, forwards a "raise" command and
    // exits as a secondary — leaving the app fully closed with no window.
    app.singleInstance().release();
    if (QProcess::startDetached(QCoreApplication::applicationFilePath(), args)) {
        QCoreApplication::quit();
    } else {
        qWarning("xcb relaunch failed to start; staying on the current platform");
    }
}

// A GPU context-loss storm (e.g. stopping a Wayland screen share) hangs the GPU
// process. Persist the fall-back to software rendering — and a one-shot notice so
// the user is told why — then relaunch onto the working configuration.
void relaunchForGpuFallback(whatsie::app::Application& app)
{
    qWarning("GPU context-loss storm detected; disabling hardware acceleration and relaunching");
    app.settings().setGpuAutoDisabled(true);
    app.settings().setGpuFallbackNotice(true);
    app.settings().sync(); // flush before the child starts
    app.singleInstance().release();
    const QStringList args = QCoreApplication::arguments().mid(1);
    if (QProcess::startDetached(QCoreApplication::applicationFilePath(), args)) {
        QCoreApplication::quit();
    } else {
        qWarning("GPU-fallback relaunch failed to start");
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

    // Record fatal signals so the next run can offer the crash in a bug report,
    // and pick up any report the previous run left behind.
    whatsie::platform::installCrashHandler(
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
        QLatin1String("/last-crash.txt"));

    // Chain onto the sink Application just installed, so a later graphics failure
    // during window/WebEngine creation is observed (FEATURES S20).
    g_previousHandler = qInstallMessageHandler(graphicsWatchHandler);

    // Watch stderr for a GPU context-loss storm (the EGL_BAD_DISPLAY flood that
    // hangs the app when a Wayland screen share is stopped) and self-heal onto
    // software rendering. Installed before the web engine spawns its GPU
    // subprocess so their inherited stderr is captured too.
    auto* gpuWatch = new whatsie::platform::GpuStderrWatch(&app);
    if (gpuWatch->install()) {
        QObject::connect(gpuWatch, &whatsie::platform::GpuStderrWatch::gpuContextLostStorm, &app,
                         [&app] { relaunchForGpuFallback(app); });
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

    // If the GPU is on trial (ADR-032), a clean 20 s means it is stable — clear
    // the crash probe so the next start is not treated as a GPU crash.
    QTimer::singleShot(20000, &app, [&app] { app.markGpuStable(); });

    // Give the GPU stack time to fail, then fall back to XCB if it did (once).
    const bool retried = qEnvironmentVariableIsSet("WHATSIE_XCB_RETRY");
    if (!retried && QGuiApplication::platformName() == QLatin1StringView("wayland")) {
        QTimer::singleShot(3000, &window, [&app] {
            if (whatsie::core::shouldRetryUnderXcb(QLatin1StringView("wayland"), false,
                                                   g_graphicsFailed.load())) {
                relaunchUnderXcb(app);
            }
        });
    }

#ifdef Q_OS_UNIX
    const int termFd = installGracefulTermination();
    if (termFd >= 0) {
        auto* termNotifier = new QSocketNotifier(termFd, QSocketNotifier::Read, &window);
        QObject::connect(termNotifier, &QSocketNotifier::activated, &window, [&window] {
            char byte = 0;
            const ssize_t ignored = ::read(g_termPipe[0], &byte, 1);
            static_cast<void>(ignored);
            window.quit();
        });
    }
#endif

    // Commands given on our own command line (we are the primary instance).
    for (const QJsonObject& command : whatsie::app::commandsFor(cli)) {
        if (command.value(QLatin1StringView(whatsie::app::cmd::kKey)).toString() !=
            QLatin1StringView(whatsie::app::cmd::kRaise)) {
            app.dispatchCommand(command);
        }
    }

    return whatsie::app::Application::exec();
}
