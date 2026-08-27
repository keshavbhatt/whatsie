#pragma once

#include <QObject>

namespace whatsie::core {
class ThemeService;
}

namespace whatsie::ui {

/// Pushes the ThemeService's palette onto the QApplication (FEATURES A1, A2).
/// Fusion style is used everywhere so palettes render identically across
/// desktops; page-side theming is added in M3.
class ThemeApplier : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(ThemeApplier)

public:
    explicit ThemeApplier(core::ThemeService& theme, QObject* parent = nullptr);
    ~ThemeApplier() override = default;

private:
    void apply();

    core::ThemeService& m_theme;
};

} // namespace whatsie::ui
