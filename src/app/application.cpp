#include "app/application.h"

#include "app/logging.h"
#include "app/single_instance.h"
#include "app/version.h"
#include "core/chromium_flags.h"
#include "core/log_sink.h"
#include "core/logging.h"
#include "core/settings/settings.h"
#include "core/spellcheck.h"
#include "core/storage_policy.h"
#include "core/theme/theme_service.h"
#include "platform/platform_info.h"

#include <QDir>
#include <QFile>
#include <QIcon>
#include <QStandardPaths>
#include <QTextStream>

#include <cmath>

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
    configureDictionaries();
    honourClearSessionMarker();

    qCInfo(core::lcCore).noquote() << u"whatsie %1 (%2) profile=%3 on %4"_s.arg(
        QString::fromLatin1(version::kVersion), QString::fromLatin1(version::kGitRevision),
        m_cli.profile.isEmpty() ? u"default"_s : m_cli.profile, platform::describeHost());
    qCInfo(core::lcCore) << "settings:" << m_settings->fileName() << "log:" << core::LogSink::logFilePath();
}

Application::~Application()
{
    // A clean shutdown means this run was stable — clear the GPU probe so the
    // next start is not treated as a crash (ADR-032).
    if (m_gpuTrialActive) {
        QFile::remove(gpuProbeMarkerPath());
    }
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

QString Application::gpuProbeMarkerPath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + u"/gpu-probe"_s;
}

void Application::evaluateGpuStability()
{
    // ADR-032: detect a GPU that brings the app down early. When running the GPU
    // under the Automatic setting we drop a probe marker; a clean quit (and the
    // 20 s stability timer) removes it. Finding it at startup means the previous
    // GPU trial crashed before proving stable — after two such strikes we fall
    // back to software rendering (with SwiftShader, so calls still work).
    const auto accel = m_settings->hardwareAcceleration();
    const QString marker = gpuProbeMarkerPath();
    const bool crashedTrial = QFile::exists(marker);
    QFile::remove(marker);

    if (crashedTrial && accel == core::HardwareAcceleration::Auto && !m_settings->gpuAutoDisabled()) {
        const int strikes = m_settings->gpuProbeStrikes() + 1;
        if (strikes >= 2) {
            m_settings->setGpuAutoDisabled(true);
            m_settings->setGpuProbeStrikes(0);
            m_settings->setGpuFallbackNotice(true); // tell the user on this launch
            qCWarning(lcApp) << "GPU proved unstable across" << strikes
                             << "starts; falling back to software rendering (ADR-032)";
        } else {
            m_settings->setGpuProbeStrikes(strikes);
            qCWarning(lcApp) << "GPU trial did not reach stability; strike" << strikes << "of 2";
        }
    }

    // If this run will trial the GPU, arm the marker for next time.
    if (accel == core::HardwareAcceleration::Auto && !m_settings->gpuAutoDisabled()) {
        QDir().mkpath(QFileInfo(marker).absolutePath());
        QFile file(marker);
        if (file.open(QIODevice::WriteOnly)) {
            file.close();
        }
        m_gpuTrialActive = true;
    }
}

void Application::markGpuStable()
{
    if (m_gpuTrialActive) {
        QFile::remove(gpuProbeMarkerPath());
        m_settings->setGpuProbeStrikes(0);
        m_gpuTrialActive = false;
        qCInfo(lcApp) << "GPU stable; probe cleared";
    }
}

void Application::applyChromiumFlags()
{
    // Must happen before the first QWebEngineProfile is created (FEATURES P6).
    evaluateGpuStability();
    const QString existing = qEnvironmentVariable("QTWEBENGINE_CHROMIUM_FLAGS");
    QStringList ours = core::chromiumFlags(m_settings->hardwareAcceleration(), m_settings->gpuAutoDisabled(),
                                           m_settings->jsMemoryLimitMb());
    // In a snap we pass --no-sandbox here (isolation comes from snap confinement,
    // ADR-008) rather than hard-coding it in the snap's environment: — that let
    // it override the user's QTWEBENGINE_CHROMIUM_FLAGS and blocked the expert
    // escape hatch (P7). Setting it in code, and merging the user's value below,
    // restores that hatch inside the snap.
    if (qEnvironmentVariableIsSet("SNAP")) {
        ours << u"--no-sandbox"_s;
    }
    // Keep Chromium's device scale in step with QT_SCALE_FACTOR (FEATURES A7) so
    // page rendering stays crisp rather than bitmap-stretched.
    const double scale = m_settings->interfaceScale();
    if (std::abs(scale - 1.0) > 0.001) {
        ours << u"--force-device-scale-factor=%1"_s.arg(scale, 0, 'g', 4);
    }
    const QString merged = core::mergeChromiumFlags(existing, ours);
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", merged.toUtf8());
    qCInfo(lcApp) << "chromium flags:" << merged;
}

void Application::configureDictionaries()
{
    // Point QtWebEngine at bundled .bdic spell-check dictionaries (FEATURES L1),
    // unless the environment already sets it. Roots cover dev, /usr, snap and
    // flatpak; findDictionariesPath returns the qtwebengine_dictionaries dir.
    if (qEnvironmentVariableIsSet("QTWEBENGINE_DICTIONARIES_PATH")) {
        return;
    }
    QStringList roots;
    if (qEnvironmentVariableIsSet("SNAP")) {
        roots << qEnvironmentVariable("SNAP") + u"/usr/share/whatsie"_s;
    }
    const QString appDir = applicationDirPath();
    roots << QDir(appDir).filePath(u"../share/whatsie"_s) << appDir << u"/app/share/whatsie"_s
          << u"/usr/share/whatsie"_s;
    const QString dir = core::findDictionariesPath(roots);
    if (!dir.isEmpty()) {
        qputenv("QTWEBENGINE_DICTIONARIES_PATH", QDir(dir).absolutePath().toUtf8());
        qCInfo(lcApp) << "spell-check dictionaries:" << dir;
    } else {
        qCInfo(lcApp) << "no bundled spell-check dictionaries found; relying on defaults";
    }
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
