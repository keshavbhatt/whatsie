#include "core/settings/settings.h"

#include "core/logging.h"
#include "core/settings/settings_keys.h"

#include <QMetaEnum>

#include <algorithm>

namespace whatsie::core {

namespace {

// ---- Defaults: the single source of truth ---------------------------------
constexpr double kDefaultZoomFactor = 1.0;
constexpr double kMinZoomFactor = 0.25;
constexpr double kMaxZoomFactor = 5.0;
constexpr Theme kDefaultTheme = Theme::System;

int toInt(Theme theme)
{
    return static_cast<int>(theme);
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

double Settings::zoomFactor() const
{
    const double stored = m_store->value(keys::kZoomFactor, kDefaultZoomFactor).toDouble();
    return std::clamp(stored, kMinZoomFactor, kMaxZoomFactor);
}

void Settings::setZoomFactor(double factor)
{
    const double clamped = std::clamp(factor, kMinZoomFactor, kMaxZoomFactor);
    if (qFuzzyCompare(clamped, zoomFactor())) {
        return;
    }
    m_store->setValue(keys::kZoomFactor, clamped);
    Q_EMIT zoomFactorChanged(clamped);
}

Theme Settings::theme() const
{
    return themeFromInt(m_store->value(keys::kTheme, toInt(kDefaultTheme)).toInt());
}

void Settings::setTheme(Theme theme)
{
    if (theme == this->theme()) {
        return;
    }
    m_store->setValue(keys::kTheme, toInt(theme));
    Q_EMIT themeChanged(theme);
}

void Settings::sync()
{
    m_store->sync();
}

QString Settings::fileName() const
{
    return m_store->fileName();
}

} // namespace whatsie::core
