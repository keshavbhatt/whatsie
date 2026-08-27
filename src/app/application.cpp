#include "app/application.h"

#include "app/logging.h"
#include "app/single_instance.h"
#include "app/version.h"
#include "core/chromium_flags.h"
#include "core/log_sink.h"
#include "core/logging.h"
#include "core/settings/settings.h"
#include "core/storage_policy.h"
#include "core/theme/theme_service.h"
#include "platform/platform_info.h"

#include <QFile>
#include <QIcon>
#include <QStandardPaths>
#include <QTextStream>

using namespace Qt::StringLiterals;

namespace whatsie::app {

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
    // Identity first: QSettings, QStandardPaths and the log location derive from it.
    setApplicationName(QString::fromLatin1(version::kApplicationName));
    setApplicationDisplayName(QString::fromLatin1(version::kDisplayName));
    setApplicationVersion(QString::fromLatin1(version::kVersion));
    setOrganizationName(QString::fromLatin1(version::kOrganizationName));
    setOrganizationDomain(QString::fromLatin1(version::kOrganizationDomain));
    setDesktopFileName(QString::fromLatin1(version::kDesktopId));
    setWindowIcon(QIcon(u":/icons/whatsie.svg"_s));
    setQuitOnLastWindowClosed(false); // the tray keeps us alive; MainWindow decides

    const CliParseResult parsed = parseCliOptions(arguments());
    QTextStream out(stdout);
    if (!parsed.ok()) {
        QTextStream(stderr) << parsed.errorText << '\n';
        m_exitCode = 2;
        return;
    }
    if (parsed.helpRequested) {
        out << parsed.helpText;
        m_exitCode = 0;
        return;
    }
    if (parsed.versionRequested) {
        out << applicationName() << ' ' << applicationVersion() << " (" << version::kGitRevision << ")\n";
        m_exitCode = 0;
        return;
    }
    m_cli = parsed.options;
    applyIdentity();
    setupLogging();

    m_instance = std::make_unique<SingleInstance>(instanceKeyFor(m_cli.profile), this);
    if (!m_instance->isPrimary()) {
        m_exitCode = forwardToPrimary() ? 0 : 1;
        return;
    }
    connect(m_instance.get(), &SingleInstance::commandReceived, this, &Application::dispatchCommand);

    m_settings = std::make_unique<core::Settings>();
    m_theme = std::make_unique<core::ThemeService>(*m_settings);
    applyChromiumFlags();
    honourClearSessionMarker();

    qCInfo(core::lcCore).noquote() << u"whatsie %1 (%2) profile=%3 on %4"_s.arg(
        QString::fromLatin1(version::kVersion), QString::fromLatin1(version::kGitRevision),
        m_cli.profile.isEmpty() ? u"default"_s : m_cli.profile, platform::describeHost());
    qCInfo(core::lcCore) << "settings:" << m_settings->fileName() << "log:" << core::LogSink::logFilePath();
}

Application::~Application()
{
    if (m_settings) {
        m_settings->sync();
    }
}

void Application::applyIdentity()
{
    // FEATURES X1: a named profile gets its own settings file, data and cache
    // directories simply by changing the application name.
    if (!m_cli.profile.isEmpty()) {
        setApplicationName(u"%1-%2"_s.arg(QString::fromLatin1(version::kApplicationName), m_cli.profile));
    }
}

void Application::setupLogging()
{
    core::LogSink::install();
    if (m_cli.noLogFile) {
        return;
    }
    core::LogSink::setLogFile(m_cli.logFile.value_or(core::LogSink::defaultLogFilePath()));
}

void Application::applyChromiumFlags()
{
    // Must happen before the first QWebEngineProfile is created (FEATURES P6).
    const QString existing = qEnvironmentVariable("QTWEBENGINE_CHROMIUM_FLAGS");
    const QString merged =
        core::mergeChromiumFlags(existing, core::chromiumFlags(m_settings->hardwareAcceleration()));
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", merged.toUtf8());
    qCInfo(lcApp) << "chromium flags:" << merged;
}

void Application::honourClearSessionMarker()
{
    // Settings → "Log out & clear session" leaves this marker; the profile
    // directories are removed here, before the web engine touches them.
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    const QString marker = dataDir + u"/clear-session"_s;
    if (!QFile::exists(marker)) {
        return;
    }
    qCInfo(lcApp) << "clearing session as requested";
    core::removeDirectorySafely(dataDir + u"/profile"_s, {dataDir});
    core::removeDirectorySafely(cacheDir + u"/profile"_s, {cacheDir});
    QFile::remove(marker);
}

bool Application::forwardToPrimary()
{
    bool allSent = true;
    for (const QJsonObject& command : commandsFor(m_cli)) {
        allSent = m_instance->sendToPrimary(command) && allSent;
    }
    qCInfo(lcApp) << (allSent ? "forwarded commands to running instance"
                              : "failed to reach running instance");
    return allSent;
}

core::Settings& Application::settings()
{
    return *m_settings;
}

core::ThemeService& Application::themeService()
{
    return *m_theme;
}

SingleInstance& Application::singleInstance()
{
    return *m_instance;
}

void Application::dispatchCommand(const QJsonObject& command)
{
    const QString name = command.value(QLatin1StringView(cmd::kKey)).toString();
    qCDebug(lcApp) << "command:" << name;
    if (name == QLatin1StringView(cmd::kRaise)) {
        Q_EMIT raiseRequested();
    } else if (name == QLatin1StringView(cmd::kNewChat)) {
        Q_EMIT newChatRequested(command.value(QLatin1StringView(cmd::kUrlKey)).toString());
    } else if (name == QLatin1StringView(cmd::kSettings)) {
        Q_EMIT settingsRequested();
    } else if (name == QLatin1StringView(cmd::kQuit)) {
        Q_EMIT quitRequested();
    } else {
        qCWarning(lcApp) << "unknown command" << name;
    }
}

} // namespace whatsie::app
