#pragma once

#include <QJsonArray>
#include <QString>
#include <QStringList>

// Drag-and-drop file attach (FEATURES M6). Dropped local files are read and
// handed to WhatsApp Web as a synthetic DragEvent carrying real File objects
// (file-drop.js), which works even where Chromium's native drop path fails
// (Wayland / Flatpak portal paths, Y#32). The heavy lifting is split into a
// pure, testable payload builder here plus the widget wiring in WebView.
namespace whatsie::web {

struct DropOutcome
{
    QJsonArray files;    ///< [{name, type, b64}] for files within the cap
    QStringList skipped; ///< names skipped (too big / unreadable)
    qint64 totalBytes = 0;
};

/// Reads `paths`, base64-encoding each file under `maxBytesPerFile`. Pure over
/// the filesystem; safe to call off the GUI thread.
[[nodiscard]] DropOutcome buildDropPayload(const QStringList& paths, qint64 maxBytesPerFile);

/// MIME type for a file by content/extension (helper, exposed for tests).
[[nodiscard]] QString guessMimeType(const QString& path);

inline constexpr qint64 kMaxDropBytesPerFile = qint64{64} * 1024 * 1024; // 64 MiB

} // namespace whatsie::web
