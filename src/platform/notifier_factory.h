#pragma once

#include <memory>

class QObject;

namespace whatsie::core {
class INotifier;
}

namespace whatsie::platform {

/// The native notification backend for this OS, or nullptr when none is
/// usable (the UI then falls back to tray balloons). Linux: portal inside
/// Flatpak, org.freedesktop.Notifications otherwise.
[[nodiscard]] std::unique_ptr<core::INotifier> createPlatformNotifier(QObject* parent = nullptr);

} // namespace whatsie::platform
