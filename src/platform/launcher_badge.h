#pragma once

#include <QString>

// Launcher/dock unread badge (FEATURES T8, issue #122). On Linux the count is
// broadcast over the com.canonical.Unity.LauncherEntry D-Bus protocol, which KDE
// Plasma's task manager, GNOME's Dash-to-Dock and others paint on the app's
// launcher button. A no-op where the protocol is unavailable.
namespace whatsie::platform {

/// The LauncherEntry app URI for a desktop id (the .desktop basename, no suffix):
/// "com.ktechpit.whatsie" → "application://com.ktechpit.whatsie.desktop". Empty in,
/// empty out. Pure; unit-tested.
[[nodiscard]] QString launcherEntryUri(const QString& desktopId);

/// Shows `count` as an unread badge on the app's launcher/taskbar button.
/// `desktopId` is the .desktop basename (pass QGuiApplication::desktopFileName()).
/// count <= 0 hides the badge. No-op on platforms without the protocol.
void setLauncherCount(const QString& desktopId, int count);

} // namespace whatsie::platform
