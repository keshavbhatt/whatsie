#pragma once

#include <QObject>

#include <optional>

// The desktop's light/dark preference, read from the xdg-desktop-portal over
// D-Bus. The portal is the source of truth on GNOME and KDE alike; Qt's
// QStyleHints::colorScheme() is unreliable there (on GNOME without a Qt platform
// theme it stays "Light" while the system is Dark, and it can report the wrong
// value across suspend/resume) — issues #367 and #374. Reading the portal, not
// gsettings, keeps this working inside a flatpak/snap sandbox.
namespace whatsie::platform {

/// The desktop colour-scheme preference, or std::nullopt when it cannot be
/// determined (or off Linux). Prefer this over QStyleHints for the System theme.
[[nodiscard]] std::optional<Qt::ColorScheme> readDesktopColorScheme();

/// Emits colorSchemeChanged() when the desktop colour-scheme preference changes,
/// via the portal's SettingChanged signal — which fires on GNOME where Qt's
/// QStyleHints::colorSchemeChanged() does not. A no-op object off Linux.
class DesktopThemeWatcher : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(DesktopThemeWatcher)

public:
    explicit DesktopThemeWatcher(QObject* parent = nullptr);
    ~DesktopThemeWatcher() override = default;

Q_SIGNALS:
    void colorSchemeChanged();

private Q_SLOTS:
    // Bound to the portal's SettingChanged(namespace, key, value); the trailing
    // variant is dropped by D-Bus argument truncation so no QtDBus type leaks
    // into this header.
    void onSettingChanged(const QString& nameSpace, const QString& key);
};

} // namespace whatsie::platform
