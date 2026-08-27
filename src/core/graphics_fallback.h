#pragma once

#include <QLatin1StringView>
#include <QString>

// One-shot fallback from a failed Wayland RHI/GPU init to the XCB platform
// (FEATURES S20). Pure decision helpers; the wiring lives in main().
namespace whatsie::core {

/// True when a Qt/Chromium log line signals the graphics backend failed to
/// initialize (EGL/GLX/OpenGL/RHI). Matched by substring, case-insensitively.
[[nodiscard]] bool isGraphicsInitFailure(const QString& message);

/// Relaunch under XCB only on Wayland, only once, and only if a graphics
/// failure was actually observed.
[[nodiscard]] bool shouldRetryUnderXcb(QLatin1StringView platformName, bool alreadyRetried, bool failureSeen);

} // namespace whatsie::core
