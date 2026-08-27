#include "core/settings/settings.h"

#include "core/logging.h"
#include "core/settings/settings_keys.h"
#include "core/zoom_policy.h"

#include <QDir>
#include <QStandardPaths>

#include <algorithm>

namespace whatsie::core {

namespace {

// ---- Defaults: the single source of truth ---------------------------------
constexpr double kDefaultZoomFactor = 1.0;
constexpr double kDefaultZoomFactorMaximized = 1.0;
constexpr Theme kDefaultTheme = Theme::System;
constexpr CloseAction kDefaultCloseAction = CloseAction::MinimizeToTray;
constexpr bool kDefaultStartMinimized = false;
constexpr bool kDefaultTrayLeftClickToggles = true;
constexpr bool kDefaultTraySymbolicIcon = false;
constexpr bool kDefaultTrayHidden = false;
constexpr bool kDefaultTrayDimWhenDisconnected = true;
constexpr bool kDefaultAutostart = false;
constexpr double kDefaultInterfaceScale = 1.0;
constexpr bool kDefaultSmoothScrolling = false; // FEATURES A14: off by default
constexpr bool kDefaultNotificationsEnabled = true;
constexpr bool kDefaultNotificationSound = true;
constexpr int kDefaultNotificationTimeoutSec = 0; // desktop default
constexpr int kMaxNotificationTimeoutSec = 120;
constexpr bool kDefaultMuted = false;
constexpr int kDefaultMessageBlurLevel = 0;
constexpr bool kDefaultAskWhereToSave = false;
constexpr bool kDefaultShowDownloadsOnStart = true;
constexpr HardwareAcceleration kDefaultHardwareAcceleration = HardwareAcceleration::Auto;

HardwareAcceleration hardwareAccelerationFromInt(int value)
{
    switch (value) {
    case static_cast<int>(HardwareAcceleration::On):
        return HardwareAcceleration::On;
    case static_cast<int>(HardwareAcceleration::Off):
        return HardwareAcceleration::Off;
    default:
        return HardwareAcceleration::Auto;
    }
}

Theme themeFromInt(int value)
{
    switch (value) {
    case static_cast<int>(Theme::Light):
        return Theme::Light;
    case static_cast<int>(Theme::Dark):
        return Theme::Dark;
    default:
        return Theme::System;
    }
}

CloseAction closeActionFromInt(int value)
{
    return value == static_cast<int>(CloseAction::Quit) ? CloseAction::Quit : CloseAction::MinimizeToTray;
}

} // namespace

Settings::Settings(QObject* parent)
    : QObject(parent)
    , m_store(std::make_unique<QSettings>())
{
    qCDebug(lcSettings) << "settings store:" << m_store->fileName();
}

Settings::Settings(const QString& iniFilePath, QObject* parent)
    : QObject(parent)
    , m_store(std::make_unique<QSettings>(iniFilePath, QSettings::IniFormat))
{
    qCDebug(lcSettings) << "settings store (ini):" << m_store->fileName();
}

Settings::~Settings()
{
    m_store->sync();
}

// ---- helpers ---------------------------------------------------------------

bool Settings::boolValue(QLatin1StringView key, bool def) const
{
    return m_store->value(key, def).toBool();
}

bool Settings::storeBool(QLatin1StringView key, bool def, bool value)
{
    if (boolValue(key, def) == value) {
        return false;
    }
    m_store->setValue(key, value);
    return true;
}

// ---- window/ ---------------------------------------------------------------

QByteArray Settings::windowGeometry() const
{
    return m_store->value(keys::kWindowGeometry).toByteArray();
}

void Settings::setWindowGeometry(const QByteArray& geometry)
{
    m_store->setValue(keys::kWindowGeometry, geometry);
}

QByteArray Settings::windowState() const
{
    return m_store->value(keys::kWindowState).toByteArray();
}

void Settings::setWindowState(const QByteArray& state)
{
    m_store->setValue(keys::kWindowState, state);
}

CloseAction Settings::closeAction() const
{
    return closeActionFromInt(
        m_store->value(keys::kCloseAction, static_cast<int>(kDefaultCloseAction)).toInt());
}

void Settings::setCloseAction(CloseAction action)
{
    if (action == closeAction()) {
        return;
    }
    m_store->setValue(keys::kCloseAction, static_cast<int>(action));
    Q_EMIT closeActionChanged(action);
}

bool Settings::startMinimized() const
{
    return boolValue(keys::kStartMinimized, kDefaultStartMinimized);
}

void Settings::setStartMinimized(bool enabled)
{
    if (storeBool(keys::kStartMinimized, kDefaultStartMinimized, enabled)) {
        Q_EMIT startMinimizedChanged(enabled);
    }
}

// ---- tray/ -----------------------------------------------------------------

bool Settings::trayLeftClickToggles() const
{
    return boolValue(keys::kTrayLeftClickToggles, kDefaultTrayLeftClickToggles);
}

void Settings::setTrayLeftClickToggles(bool enabled)
{
    if (storeBool(keys::kTrayLeftClickToggles, kDefaultTrayLeftClickToggles, enabled)) {
        Q_EMIT trayLeftClickTogglesChanged(enabled);
    }
}

bool Settings::traySymbolicIcon() const
{
    return boolValue(keys::kTraySymbolicIcon, kDefaultTraySymbolicIcon);
}

void Settings::setTraySymbolicIcon(bool enabled)
{
    if (storeBool(keys::kTraySymbolicIcon, kDefaultTraySymbolicIcon, enabled)) {
        Q_EMIT traySymbolicIconChanged(enabled);
    }
}

bool Settings::trayHidden() const
{
    return boolValue(keys::kTrayHidden, kDefaultTrayHidden);
}

void Settings::setTrayHidden(bool hidden)
{
    if (storeBool(keys::kTrayHidden, kDefaultTrayHidden, hidden)) {
        Q_EMIT trayHiddenChanged(hidden);
    }
}

bool Settings::trayDimWhenDisconnected() const
{
    return boolValue(keys::kTrayDimWhenDisconnected, kDefaultTrayDimWhenDisconnected);
}

void Settings::setTrayDimWhenDisconnected(bool enabled)
{
    if (storeBool(keys::kTrayDimWhenDisconnected, kDefaultTrayDimWhenDisconnected, enabled)) {
        Q_EMIT trayDimWhenDisconnectedChanged(enabled);
    }
}

bool Settings::autostart() const
{
    return boolValue(keys::kAutostart, kDefaultAutostart);
}

void Settings::setAutostart(bool enabled)
{
    if (storeBool(keys::kAutostart, kDefaultAutostart, enabled)) {
        Q_EMIT autostartChanged(enabled);
    }
}

// ---- view/ -----------------------------------------------------------------

double Settings::zoomFactor() const
{
    return clampZoom(m_store->value(keys::kZoomFactor, kDefaultZoomFactor).toDouble());
}

void Settings::setZoomFactor(double factor)
{
    const double clamped = clampZoom(factor);
    if (qFuzzyCompare(clamped, zoomFactor())) {
        return;
    }
    m_store->setValue(keys::kZoomFactor, clamped);
    Q_EMIT zoomFactorChanged(clamped);
}

double Settings::zoomFactorMaximized() const
{
    return clampZoom(m_store->value(keys::kZoomFactorMaximized, kDefaultZoomFactorMaximized).toDouble());
}

void Settings::setZoomFactorMaximized(double factor)
{
    const double clamped = clampZoom(factor);
    if (qFuzzyCompare(clamped, zoomFactorMaximized())) {
        return;
    }
    m_store->setValue(keys::kZoomFactorMaximized, clamped);
    Q_EMIT zoomFactorMaximizedChanged(clamped);
}

bool Settings::smoothScrolling() const
{
    return boolValue(keys::kSmoothScrolling, kDefaultSmoothScrolling);
}

void Settings::setSmoothScrolling(bool enabled)
{
    if (storeBool(keys::kSmoothScrolling, kDefaultSmoothScrolling, enabled)) {
        Q_EMIT smoothScrollingChanged(enabled);
    }
}

double Settings::interfaceScale() const
{
    const double stored = m_store->value(keys::kInterfaceScale, kDefaultInterfaceScale).toDouble();
    return std::clamp(stored, kMinInterfaceScale, kMaxInterfaceScale);
}

void Settings::setInterfaceScale(double scale)
{
    const double clamped = std::clamp(scale, kMinInterfaceScale, kMaxInterfaceScale);
    if (qFuzzyCompare(clamped, interfaceScale())) {
        return;
    }
    m_store->setValue(keys::kInterfaceScale, clamped);
    Q_EMIT interfaceScaleChanged(clamped);
}

bool Settings::muted() const
{
    return boolValue(keys::kMuted, kDefaultMuted);
}

void Settings::setMuted(bool muted)
{
    if (storeBool(keys::kMuted, kDefaultMuted, muted)) {
        Q_EMIT mutedChanged(muted);
    }
}

int Settings::messageBlurLevel() const
{
    const int stored = m_store->value(keys::kMessageBlurLevel, kDefaultMessageBlurLevel).toInt();
    return std::clamp(stored, 0, kMaxBlurLevel);
}

void Settings::setMessageBlurLevel(int level)
{
    const int clamped = std::clamp(level, 0, kMaxBlurLevel);
    if (clamped == messageBlurLevel()) {
        return;
    }
    m_store->setValue(keys::kMessageBlurLevel, clamped);
    Q_EMIT messageBlurLevelChanged(clamped);
}

// ---- downloads/ ------------------------------------------------------------

QString Settings::downloadDirectory() const
{
    const QString stored = m_store->value(keys::kDownloadDirectory).toString();
    if (!stored.isEmpty() && QDir(stored).isAbsolute()) {
        return QDir::cleanPath(stored);
    }
    const QString fallback = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    return fallback.isEmpty() ? QDir::homePath() : fallback;
}

void Settings::setDownloadDirectory(const QString& directory)
{
    const QString clean = QDir::cleanPath(directory.trimmed());
    if (clean == downloadDirectory()) {
        return;
    }
    m_store->setValue(keys::kDownloadDirectory, clean);
    Q_EMIT downloadDirectoryChanged(downloadDirectory());
}

bool Settings::askWhereToSave() const
{
    return boolValue(keys::kAskWhereToSave, kDefaultAskWhereToSave);
}

void Settings::setAskWhereToSave(bool ask)
{
    if (storeBool(keys::kAskWhereToSave, kDefaultAskWhereToSave, ask)) {
        Q_EMIT askWhereToSaveChanged(ask);
    }
}

bool Settings::showDownloadsOnStart() const
{
    return boolValue(keys::kShowDownloadsOnStart, kDefaultShowDownloadsOnStart);
}

void Settings::setShowDownloadsOnStart(bool show)
{
    if (storeBool(keys::kShowDownloadsOnStart, kDefaultShowDownloadsOnStart, show)) {
        Q_EMIT showDownloadsOnStartChanged(show);
    }
}

// ---- files/ ----------------------------------------------------------------

QString Settings::lastOpenDirectory() const
{
    const QString stored = m_store->value(keys::kLastOpenDirectory).toString();
    return (!stored.isEmpty() && QDir(stored).exists()) ? stored : QDir::homePath();
}

void Settings::setLastOpenDirectory(const QString& directory)
{
    m_store->setValue(keys::kLastOpenDirectory, directory);
}

// ---- advanced/ -------------------------------------------------------------

HardwareAcceleration Settings::hardwareAcceleration() const
{
    return hardwareAccelerationFromInt(
        m_store->value(keys::kHardwareAcceleration, static_cast<int>(kDefaultHardwareAcceleration)).toInt());
}

void Settings::setHardwareAcceleration(HardwareAcceleration mode)
{
    if (mode == hardwareAcceleration()) {
        return;
    }
    m_store->setValue(keys::kHardwareAcceleration, static_cast<int>(mode));
    Q_EMIT hardwareAccelerationChanged(mode);
}

// ---- appearance/ -----------------------------------------------------------

Theme Settings::theme() const
{
    return themeFromInt(m_store->value(keys::kTheme, static_cast<int>(kDefaultTheme)).toInt());
}

void Settings::setTheme(Theme theme)
{
    if (theme == this->theme()) {
        return;
    }
    m_store->setValue(keys::kTheme, static_cast<int>(theme));
    Q_EMIT themeChanged(theme);
}

// ---- notifications/ --------------------------------------------------------

bool Settings::notificationsEnabled() const
{
    return boolValue(keys::kNotificationsEnabled, kDefaultNotificationsEnabled);
}

void Settings::setNotificationsEnabled(bool enabled)
{
    if (storeBool(keys::kNotificationsEnabled, kDefaultNotificationsEnabled, enabled)) {
        Q_EMIT notificationsEnabledChanged(enabled);
    }
}

bool Settings::notificationSound() const
{
    return boolValue(keys::kNotificationSound, kDefaultNotificationSound);
}

void Settings::setNotificationSound(bool enabled)
{
    if (storeBool(keys::kNotificationSound, kDefaultNotificationSound, enabled)) {
        Q_EMIT notificationSoundChanged(enabled);
    }
}

int Settings::notificationTimeoutSec() const
{
    const int stored = m_store->value(keys::kNotificationTimeoutSec, kDefaultNotificationTimeoutSec).toInt();
    return std::clamp(stored, 0, kMaxNotificationTimeoutSec);
}

void Settings::setNotificationTimeoutSec(int seconds)
{
    const int clamped = std::clamp(seconds, 0, kMaxNotificationTimeoutSec);
    if (clamped == notificationTimeoutSec()) {
        return;
    }
    m_store->setValue(keys::kNotificationTimeoutSec, clamped);
    Q_EMIT notificationTimeoutSecChanged(clamped);
}

// ---- misc ------------------------------------------------------------------

void Settings::sync()
{
    m_store->sync();
}

QString Settings::fileName() const
{
    return m_store->fileName();
}

} // namespace whatsie::core
