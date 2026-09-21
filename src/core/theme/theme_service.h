#pragma once

#include <QObject>
#include <QPalette>

#include <functional>
#include <optional>

namespace whatsie::core {

class Settings;

/// Resolves the Theme setting (System/Light/Dark) into an effective colour
/// scheme, following the platform live when set to System (FEATURES A1, A2).
/// Applying palettes to widgets is the UI layer's job; this class only decides.
class ThemeService : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ThemeService)

public:
    explicit ThemeService(Settings& settings, QObject* parent = nullptr);
    ~ThemeService() override = default;

    /// Authoritative source of the desktop colour scheme for the System theme,
    /// injected by the platform layer (core must not depend on D-Bus). Returns
    /// nullopt when it cannot decide, in which case QStyleHints is used. Setting
    /// it re-evaluates immediately. Needed because QStyleHints is unreliable on
    /// Linux at start-up and across resume (#367, #374).
    using SchemeProbe = std::function<std::optional<Qt::ColorScheme>()>;
    void setSchemeProbe(SchemeProbe probe);

    [[nodiscard]] Qt::ColorScheme effectiveScheme() const;
    [[nodiscard]] bool isDark() const { return effectiveScheme() == Qt::ColorScheme::Dark; }
    [[nodiscard]] bool followsSystem() const;

    /// Re-reads the effective scheme now and emits effectiveSchemeChanged if it
    /// changed. Call when the desktop's colour-scheme signal fires.
    void reevaluate();

    [[nodiscard]] static QPalette lightPalette();
    [[nodiscard]] static QPalette darkPalette();
    [[nodiscard]] QPalette palette() const { return isDark() ? darkPalette() : lightPalette(); }

Q_SIGNALS:
    void effectiveSchemeChanged(Qt::ColorScheme scheme);

private:
    [[nodiscard]] Qt::ColorScheme platformScheme() const;
    [[nodiscard]] Qt::ColorScheme resolve() const;

    Settings& m_settings;
    SchemeProbe m_probe;
    Qt::ColorScheme m_current = Qt::ColorScheme::Light;
};

} // namespace whatsie::core
