#pragma once

#include <QString>

namespace whatsie::platform {

/// Human-readable description of the host (OS, desktop, Qt version, session
/// type) for the About dialog and bug reports.
[[nodiscard]] QString describeHost();

/// True when running inside a snap or flatpak sandbox.
[[nodiscard]] bool isSandboxed();

/// How this build was packaged/installed: "snap" (with revision), "flatpak",
/// "AppImage", or "native". Useful in bug reports to reproduce the environment.
[[nodiscard]] QString packageType();

} // namespace whatsie::platform
