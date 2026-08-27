#pragma once

#include <QImage>
#include <QString>

// Pure functions behind the tray/window unread badge (FEATURES T2).
namespace whatsie::core {

/// Parses WhatsApp Web's document title. "(3) WhatsApp" → 3, "WhatsApp" → 0,
/// "(99+) WhatsApp" → 99. Anything unrecognised → 0.
[[nodiscard]] int unreadCountFromTitle(const QString& title);

/// Returns `base` with a red counter bubble in the bottom-right corner.
/// `count <= 0` returns `base` unchanged. Counts above 99 render as "99+".
[[nodiscard]] QImage composeUnreadBadge(const QImage& base, int count);

} // namespace whatsie::core
