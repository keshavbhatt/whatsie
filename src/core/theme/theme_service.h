#pragma once

#include <QObject>
#include <QPalette>

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

    [[nodiscard]] Qt::ColorScheme effectiveScheme() const;
    [[nodiscard]] bool isDark() const { return effectiveScheme() == Qt::ColorScheme::Dark; }

    [[nodiscard]] static QPalette lightPalette();
    [[nodiscard]] static QPalette darkPalette();
    [[nodiscard]] QPalette palette() const { return isDark() ? darkPalette() : lightPalette(); }

Q_SIGNALS:
    void effectiveSchemeChanged(Qt::ColorScheme scheme);

private:
    void reevaluate();

    Settings& m_settings;
    Qt::ColorScheme m_current = Qt::ColorScheme::Light;
};

} // namespace whatsie::core
