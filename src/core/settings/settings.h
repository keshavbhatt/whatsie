#pragma once

#include <QByteArray>
#include <QObject>
#include <QSettings>
#include <QString>

#include <memory>

namespace whatsie::core {

enum class Theme
{
    System,
    Light,
    Dark,
};

enum class CloseAction
{
    MinimizeToTray,
    Quit,
};

enum class HardwareAcceleration
{
    Auto, ///< Chromium decides (GPU on unless blocklisted)
    On,   ///< ignore the GPU blocklist
    Off,  ///< --disable-gpu
};

enum class ProxyMode
{
    System, ///< follow the desktop's proxy configuration
    None,   ///< direct connection, ignore system proxy
    Manual, ///< the host/port below
};

enum class ProxyType
{
    Http,
    Socks5,
};

/// Everything needed to build a QNetworkProxy (FEATURES P3). The password is
/// session-only: it is never written to disk (ADR-030).
struct ProxyConfig
{
    ProxyMode mode = ProxyMode::System;
    ProxyType type = ProxyType::Http;
    QString host;
    int port = 8080;
    QString user;
    QString password; ///< transient, from the auth dialog / settings field
};

/// Typed facade over QSettings. The only place in the code base allowed to
/// construct a QSettings. Defaults live in one table (settings.cpp), every
/// setter emits a change signal so the rest of the app reacts instead of
/// polling.
class Settings : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Settings)

public:
    /// Uses the platform default location (org/app name from the application).
    explicit Settings(QObject* parent = nullptr);
    /// Uses an explicit INI file — for tests and portable mode.
    Settings(const QString& iniFilePath, QObject* parent = nullptr);
    ~Settings() override;

    // window/
    [[nodiscard]] QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray& geometry);
    [[nodiscard]] QByteArray windowState() const;
    void setWindowState(const QByteArray& state);
    [[nodiscard]] CloseAction closeAction() const;
    void setCloseAction(CloseAction action);
    [[nodiscard]] bool startMinimized() const;
    void setStartMinimized(bool enabled);

    // tray/
    [[nodiscard]] bool trayLeftClickToggles() const;
    void setTrayLeftClickToggles(bool enabled);
    [[nodiscard]] bool traySymbolicIcon() const;
    void setTraySymbolicIcon(bool enabled);
    [[nodiscard]] bool trayHidden() const;
    void setTrayHidden(bool hidden);
    [[nodiscard]] bool trayDimWhenDisconnected() const;
    void setTrayDimWhenDisconnected(bool enabled);

    // general/
    [[nodiscard]] bool autostart() const;
    void setAutostart(bool enabled);

    // view/
    [[nodiscard]] double zoomFactor() const;
    void setZoomFactor(double factor);
    [[nodiscard]] double zoomFactorMaximized() const;
    void setZoomFactorMaximized(double factor);
    [[nodiscard]] bool smoothScrolling() const;
    void setSmoothScrolling(bool enabled);
    /// UI scale factor (0.5..3.0). Applied at start-up; changing needs a restart.
    [[nodiscard]] double interfaceScale() const;
    void setInterfaceScale(double scale);
    static constexpr double kMinInterfaceScale = 0.5;
    static constexpr double kMaxInterfaceScale = 3.0;
    [[nodiscard]] bool muted() const;
    void setMuted(bool muted);

    // privacy/ : 0 = off, 1..kMaxBlurLevel = increasing blur (FEATURES A8)
    [[nodiscard]] int messageBlurLevel() const;
    void setMessageBlurLevel(int level);
    static constexpr int kMaxBlurLevel = 3;

    // downloads/
    /// Always an absolute directory; falls back to the platform Downloads folder.
    [[nodiscard]] QString downloadDirectory() const;
    void setDownloadDirectory(const QString& directory);
    [[nodiscard]] bool askWhereToSave() const;
    void setAskWhereToSave(bool ask);
    [[nodiscard]] bool showDownloadsOnStart() const;
    void setShowDownloadsOnStart(bool show);

    // files/
    [[nodiscard]] QString lastOpenDirectory() const;
    void setLastOpenDirectory(const QString& directory);

    // advanced/
    [[nodiscard]] HardwareAcceleration hardwareAcceleration() const;
    void setHardwareAcceleration(HardwareAcceleration mode);
    /// FEATURES P2: force WebRTC onto public interfaces only (privacy; can break
    /// LAN calls). Default off.
    [[nodiscard]] bool webrtcPublicInterfacesOnly() const;
    void setWebrtcPublicInterfacesOnly(bool enabled);

    // proxy/ (FEATURES P3, M12b)
    [[nodiscard]] ProxyMode proxyMode() const;
    void setProxyMode(ProxyMode mode);
    [[nodiscard]] ProxyType proxyType() const;
    void setProxyType(ProxyType type);
    [[nodiscard]] QString proxyHost() const;
    void setProxyHost(const QString& host);
    [[nodiscard]] int proxyPort() const;
    void setProxyPort(int port);
    [[nodiscard]] QString proxyUser() const;
    void setProxyUser(const QString& user);
    /// Session-only proxy password (never persisted).
    [[nodiscard]] QString proxyPassword() const;
    void setProxyPassword(const QString& password);
    /// The whole proxy configuration, password included.
    [[nodiscard]] ProxyConfig proxyConfig() const;

    // appearance/
    [[nodiscard]] Theme theme() const;
    void setTheme(Theme theme);

    // notifications/
    [[nodiscard]] bool notificationsEnabled() const;
    void setNotificationsEnabled(bool enabled);
    [[nodiscard]] bool notificationSound() const;
    void setNotificationSound(bool enabled);
    /// 0 = desktop default; otherwise seconds until the notification expires.
    [[nodiscard]] int notificationTimeoutSec() const;
    void setNotificationTimeoutSec(int seconds);

    /// Flushes pending writes to disk. Normally automatic; explicit before quit.
    void sync();

    /// Location of the backing store (file path or registry key), for diagnostics.
    [[nodiscard]] QString fileName() const;

Q_SIGNALS:
    void closeActionChanged(whatsie::core::CloseAction action);
    void startMinimizedChanged(bool enabled);
    void trayLeftClickTogglesChanged(bool enabled);
    void traySymbolicIconChanged(bool enabled);
    void trayHiddenChanged(bool hidden);
    void trayDimWhenDisconnectedChanged(bool enabled);
    void autostartChanged(bool enabled);
    void interfaceScaleChanged(double scale);
    void zoomFactorChanged(double factor);
    void zoomFactorMaximizedChanged(double factor);
    void smoothScrollingChanged(bool enabled);
    void webrtcPublicInterfacesOnlyChanged(bool enabled);
    void proxyChanged();
    void themeChanged(whatsie::core::Theme theme);
    void notificationsEnabledChanged(bool enabled);
    void notificationSoundChanged(bool enabled);
    void notificationTimeoutSecChanged(int seconds);
    void mutedChanged(bool muted);
    void messageBlurLevelChanged(int level);
    void downloadDirectoryChanged(const QString& directory);
    void askWhereToSaveChanged(bool ask);
    void showDownloadsOnStartChanged(bool show);
    void hardwareAccelerationChanged(whatsie::core::HardwareAcceleration mode);

private:
    [[nodiscard]] bool boolValue(QLatin1StringView key, bool def) const;
    /// Writes and returns true when the stored value actually changed.
    bool storeBool(QLatin1StringView key, bool def, bool value);

    std::unique_ptr<QSettings> m_store;
    QString m_proxyPassword; // session-only, never persisted
};

} // namespace whatsie::core

Q_DECLARE_METATYPE(whatsie::core::Theme)
Q_DECLARE_METATYPE(whatsie::core::CloseAction)
Q_DECLARE_METATYPE(whatsie::core::HardwareAcceleration)
