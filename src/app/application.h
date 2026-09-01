#pragma once

#include "app/cli_options.h"

#include <QApplication>

#include <memory>

namespace whatsie::core {
class Settings;
class ThemeService;
} // namespace whatsie::core

namespace whatsie::app {

class SingleInstance;

/// Application object. Sets identity (names/version, profile suffix), parses
/// the command line, owns Settings / ThemeService / SingleInstance and hands
/// them out by reference — the only sanctioned "global".
class Application : public QApplication
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Application)

public:
    Application(int& argc, char** argv);
    ~Application() override;

    /// True when main() should return immediately (help/version printed,
    /// parse error, or commands were forwarded to a running instance).
    [[nodiscard]] bool shouldExit() const { return m_exitCode.has_value(); }
    [[nodiscard]] int exitCode() const { return m_exitCode.value_or(0); }

    [[nodiscard]] const CliOptions& cliOptions() const { return m_cli; }
    [[nodiscard]] core::Settings& settings();
    [[nodiscard]] core::ThemeService& themeService();
    [[nodiscard]] SingleInstance& singleInstance();

    /// Dispatches a JSON command (from IPC or from our own CLI) to the UI via
    /// the signals below.
    void dispatchCommand(const QJsonObject& command);

    /// Called once the window has stayed up long enough to deem the GPU stable;
    /// clears the crash probe so the next start is not treated as a crash.
    void markGpuStable();

Q_SIGNALS:
    void raiseRequested();
    void newChatRequested(const QString& target);
    void settingsRequested();
    void quitRequested();

private:
    void applyIdentity();
    void setupLogging();
    void applyChromiumFlags();
    void evaluateGpuStability();
    [[nodiscard]] QString gpuProbeMarkerPath() const;
    void configureDictionaries();
    void honourClearSessionMarker();
    [[nodiscard]] bool forwardToPrimary();

    CliOptions m_cli;
    std::optional<int> m_exitCode;
    bool m_gpuTrialActive = false;
    std::unique_ptr<SingleInstance> m_instance;
    std::unique_ptr<core::Settings> m_settings;
    std::unique_ptr<core::ThemeService> m_theme;
};

} // namespace whatsie::app
