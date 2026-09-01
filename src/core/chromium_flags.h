#pragma once

#include <QString>
#include <QStringList>

namespace whatsie::core {

enum class HardwareAcceleration;

/// True when the GPU should be off: the setting is Off, or it is Auto and the
/// GPU was auto-disabled after an unstable trial (ADR-032). On/false otherwise.
[[nodiscard]] bool useSoftwareGpu(HardwareAcceleration acceleration, bool autoDisabled);

/// Chromium switches derived from settings (FEATURES P6, M2). Applied to
/// QTWEBENGINE_CHROMIUM_FLAGS before the web engine starts; a user-provided
/// value of that variable is kept and ours are appended. When the GPU is off
/// (see useSoftwareGpu) it also enables SwiftShader so WhatsApp calls keep a
/// software WebGL context instead of a blank remote video (ADR-032).
[[nodiscard]] QStringList chromiumFlags(HardwareAcceleration acceleration, bool gpuAutoDisabled = false);

/// Merges `userFlags` (existing env var content) with ours, deduplicated.
[[nodiscard]] QString mergeChromiumFlags(const QString& userFlags, const QStringList& ours);

} // namespace whatsie::core
