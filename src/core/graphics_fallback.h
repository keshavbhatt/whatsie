#pragma once

#include <QLatin1StringView>
#include <QString>

// One-shot fallback from a failed Wayland RHI/GPU init to the XCB platform
// (FEATURES S20). Pure decision helpers; the wiring lives in main().
namespace whatsie::core {

/// True when a Qt/Chromium log line signals the graphics backend failed to
/// initialize (EGL/GLX/OpenGL/RHI). Matched by substring, case-insensitively.
[[nodiscard]] bool isGraphicsInitFailure(const QString& message);

/// True for the Chromium log line emitted on NVIDIA/Wayland when GBM buffers are
/// unavailable and it falls back to Vulkan rendering — which paints the web view
/// black while the GPU process stays healthy (issue #351). Not a hard init
/// failure, so detected separately; on Wayland it warrants one retry under
/// XWayland, where the NVIDIA driver renders correctly.
[[nodiscard]] bool isGbmVulkanFallback(const QString& message);

/// Relaunch under XCB only on Wayland, only once, and only if a graphics
/// failure was actually observed.
[[nodiscard]] bool shouldRetryUnderXcb(QLatin1StringView platformName, bool alreadyRetried, bool failureSeen);

} // namespace whatsie::core
