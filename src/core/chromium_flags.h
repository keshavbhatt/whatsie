#pragma once

#include <QString>
#include <QStringList>

namespace whatsie::core {

enum class HardwareAcceleration;

/// Chromium switches derived from settings (FEATURES P6, M2). Applied to
/// QTWEBENGINE_CHROMIUM_FLAGS before the web engine starts; a user-provided
/// value of that variable is kept and ours are appended.
[[nodiscard]] QStringList chromiumFlags(HardwareAcceleration acceleration);

/// Merges `userFlags` (existing env var content) with ours, deduplicated.
[[nodiscard]] QString mergeChromiumFlags(const QString& userFlags, const QStringList& ours);

} // namespace whatsie::core
