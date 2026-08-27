#pragma once

namespace whatsie::platform {

/// Launch-at-login (FEATURES P4). Linux: an XDG autostart .desktop entry;
/// Windows backend lands in M6. `startMinimized` writes `--minimized` into the
/// autostart command so login start-up goes straight to the tray.
[[nodiscard]] bool autostartEnabled();
bool setAutostartEnabled(bool enabled, bool startMinimized);

} // namespace whatsie::platform
