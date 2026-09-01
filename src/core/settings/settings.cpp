#include "core/settings/settings.h"

#include "core/logging.h"
#include "core/settings/settings_keys.h"
#include "core/spellcheck.h"
#include "core/zoom_policy.h"

#include <QDir>
#include <QLocale>
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
constexpr bool kDefaultWebrtcPublicOnly = false;
constexpr bool kDefaultSpellCheckEnabled = true;
constexpr bool kDefaultLockOnStart = false;
constexpr bool kDefaultLockOnHide = false;
constexpr int kDefaultLockIdleMinutes = 0;
constexpr ProxyMode kDefaultProxyMode = ProxyMode::System;
constexpr ProxyType kDefaultProxyType = ProxyType::Http;
constexpr int kDefaultProxyPort = 8080;

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

ProxyMode proxyModeFromInt(int value)
{
    switch (value) {
    case static_cast<int>(ProxyMode::None):
        return ProxyMode::None;
    case static_cast<int>(ProxyMode::Manual):
        return ProxyMode::Manual;
    default:
        return ProxyMode::System;
    }
}

ProxyType proxyTypeFromInt(int value)
{
    return value == static_cast<int>(ProxyType::Socks5) ? ProxyType::Socks5 : ProxyType::Http;
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
    // A deliberate choice resets the auto-fallback: picking "Automatic" gives
    // the GPU a fresh trial, On/Off are honoured directly (ADR-032).
    setGpuAutoDisabled(false);
    setGpuProbeStrikes(0);
    Q_EMIT hardwareAccelerationChanged(mode);
}

bool Settings::webrtcPublicInterfacesOnly() const
{
    return boolValue(keys::kWebrtcPublicOnly, kDefaultWebrtcPublicOnly);
}

void Settings::setWebrtcPublicInterfacesOnly(bool enabled)
{
    if (storeBool(keys::kWebrtcPublicOnly, kDefaultWebrtcPublicOnly, enabled)) {
        Q_EMIT webrtcPublicInterfacesOnlyChanged(enabled);
    }
}

bool Settings::gpuAutoDisabled() const
{
    return boolValue(keys::kGpuAutoDisabled, false);
}

void Settings::setGpuAutoDisabled(bool disabled)
{
    if (storeBool(keys::kGpuAutoDisabled, false, disabled)) {
        Q_EMIT gpuAutoDisabledChanged(disabled);
    }
}

int Settings::gpuProbeStrikes() const
{
    return std::max(0, m_store->value(keys::kGpuProbeStrikes, 0).toInt());
}

void Settings::setGpuProbeStrikes(int strikes)
{
    m_store->setValue(keys::kGpuProbeStrikes, std::max(0, strikes));
}

// ---- proxy/ (FEATURES P3, M12b) --------------------------------------------

ProxyMode Settings::proxyMode() const
{
    return proxyModeFromInt(m_store->value(keys::kProxyMode, static_cast<int>(kDefaultProxyMode)).toInt());
}

void Settings::setProxyMode(ProxyMode mode)
{
    if (mode == proxyMode()) {
        return;
    }
    m_store->setValue(keys::kProxyMode, static_cast<int>(mode));
    Q_EMIT proxyChanged();
}

ProxyType Settings::proxyType() const
{
    return proxyTypeFromInt(m_store->value(keys::kProxyType, static_cast<int>(kDefaultProxyType)).toInt());
}

void Settings::setProxyType(ProxyType type)
{
    if (type == proxyType()) {
        return;
    }
    m_store->setValue(keys::kProxyType, static_cast<int>(type));
    Q_EMIT proxyChanged();
}

QString Settings::proxyHost() const
{
    return m_store->value(keys::kProxyHost).toString();
}

void Settings::setProxyHost(const QString& host)
{
    if (host == proxyHost()) {
        return;
    }
    m_store->setValue(keys::kProxyHost, host);
    Q_EMIT proxyChanged();
}

int Settings::proxyPort() const
{
    return std::clamp(m_store->value(keys::kProxyPort, kDefaultProxyPort).toInt(), 1, 65535);
}

void Settings::setProxyPort(int port)
{
    const int clamped = std::clamp(port, 1, 65535);
    if (clamped == proxyPort()) {
        return;
    }
    m_store->setValue(keys::kProxyPort, clamped);
    Q_EMIT proxyChanged();
}

QString Settings::proxyUser() const
{
    return m_store->value(keys::kProxyUser).toString();
}

void Settings::setProxyUser(const QString& user)
{
    if (user == proxyUser()) {
        return;
    }
    m_store->setValue(keys::kProxyUser, user);
    Q_EMIT proxyChanged();
}

QString Settings::proxyPassword() const
{
    return m_proxyPassword;
}

void Settings::setProxyPassword(const QString& password)
{
    if (password == m_proxyPassword) {
        return;
    }
    m_proxyPassword = password; // memory only — never written to m_store
    Q_EMIT proxyChanged();
}

ProxyConfig Settings::proxyConfig() const
{
    return ProxyConfig{proxyMode(), proxyType(), proxyHost(), proxyPort(), proxyUser(), m_proxyPassword};
}

bool Settings::spellCheckEnabled() const
{
    return boolValue(keys::kSpellCheckEnabled, kDefaultSpellCheckEnabled);
}

void Settings::setSpellCheckEnabled(bool enabled)
{
    if (storeBool(keys::kSpellCheckEnabled, kDefaultSpellCheckEnabled, enabled)) {
        Q_EMIT spellCheckEnabledChanged(enabled);
    }
}

QStringList Settings::spellCheckLanguages() const
{
    const QVariant stored = m_store->value(keys::kSpellCheckLanguages);
    if (stored.isValid() && !stored.toStringList().isEmpty()) {
        return stored.toStringList();
    }
    return {dictionaryNameForLocale(QLocale::system())};
}

void Settings::setSpellCheckLanguages(const QStringList& languages)
{
    if (languages == spellCheckLanguages()) {
        return;
    }
    m_store->setValue(keys::kSpellCheckLanguages, languages);
    Q_EMIT spellCheckLanguagesChanged(languages);
}

// ---- lock/ (FEATURES P1) ---------------------------------------------------

bool Settings::hasPasscode() const
{
    return passcodeRecord().isValid();
}

PasscodeRecord Settings::passcodeRecord() const
{
    PasscodeRecord record;
    record.salt = m_store->value(keys::kLockSalt).toByteArray();
    record.hash = m_store->value(keys::kLockHash).toByteArray();
    record.iterations = m_store->value(keys::kLockIterations, 0).toInt();
    return record;
}

void Settings::setPasscode(const PasscodeRecord& record)
{
    if (!record.isValid()) {
        return;
    }
    m_store->setValue(keys::kLockSalt, record.salt);
    m_store->setValue(keys::kLockHash, record.hash);
    m_store->setValue(keys::kLockIterations, record.iterations);
    Q_EMIT lockConfigChanged();
}

void Settings::clearPasscode()
{
    if (!hasPasscode()) {
        return;
    }
    m_store->remove(keys::kLockSalt.data());
    m_store->remove(keys::kLockHash.data());
    m_store->remove(keys::kLockIterations.data());
    Q_EMIT lockConfigChanged();
}

bool Settings::lockOnStart() const
{
    return boolValue(keys::kLockOnStart, kDefaultLockOnStart);
}

void Settings::setLockOnStart(bool enabled)
{
    if (storeBool(keys::kLockOnStart, kDefaultLockOnStart, enabled)) {
        Q_EMIT lockConfigChanged();
    }
}

bool Settings::lockOnHide() const
{
    return boolValue(keys::kLockOnHide, kDefaultLockOnHide);
}

void Settings::setLockOnHide(bool enabled)
{
    if (storeBool(keys::kLockOnHide, kDefaultLockOnHide, enabled)) {
        Q_EMIT lockConfigChanged();
    }
}

int Settings::lockIdleMinutes() const
{
    return std::max(0, m_store->value(keys::kLockIdleMinutes, kDefaultLockIdleMinutes).toInt());
}

void Settings::setLockIdleMinutes(int minutes)
{
    const int clamped = std::max(0, minutes);
    if (clamped == lockIdleMinutes()) {
        return;
    }
    m_store->setValue(keys::kLockIdleMinutes, clamped);
    Q_EMIT lockConfigChanged();
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
