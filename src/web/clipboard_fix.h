#pragma once

class QMimeData;
class QClipboard;

namespace whatsie::web {

/// Chromium only pastes images it can read as image/png. Qt exposes copied
/// images through `application/x-qt-image` on many platforms, so WhatsApp's
/// composer silently ignores the paste (W#33, W#311). Before a paste, add a
/// PNG representation to the clipboard when it is missing. Returns true when
/// the clipboard was rewritten.
bool ensureClipboardImageIsPng(QClipboard& clipboard);

/// Pure helper: does `mime` carry an image without an image/png payload?
[[nodiscard]] bool needsPngRepresentation(const QMimeData& mime);

} // namespace whatsie::web
