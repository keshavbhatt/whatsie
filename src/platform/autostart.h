#pragma once

namespace whatsie::platform {

/// Launch-at-login. Linux: an XDG autostart .desktop entry; Windows: a
/// per-user registry Run entry. `startMinimized` writes `--minimized` into the
/// autostart command so login start-up goes straight to the tray.
[[nodiscard]] bool autostartEnabled();
bool setAutostartEnabled(bool enabled, bool startMinimized);

} // namespace whatsie::platform
