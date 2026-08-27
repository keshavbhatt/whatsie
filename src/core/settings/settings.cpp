#include "core/settings/settings.h"

#include "core/logging.h"
#include "core/settings/settings_keys.h"
#include "core/zoom_policy.h"

namespace whatsie::core {

namespace {

// ---- Defaults: the single source of truth ---------------------------------
constexpr double kDefaultZoomFactor = 1.0;
constexpr double kDefaultZoomFactorMaximized = 1.0;
constexpr Theme kDefaultTheme = Theme::System;
constexpr CloseAction kDefaultCloseAction = CloseAction::MinimizeToTray;
constexpr bool kDefaultStartMinimized = false;
constexpr bool kDefaultTrayLeftClickToggles = true;
constexpr bool kDefaultSmoothScrolling = false; // FEATURES A14: off by default

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
