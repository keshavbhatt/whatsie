#pragma once

#include <QString>

namespace whatsie::web {

/// Self-contained HTML for a Whatsie-branded load-failure page (replaces
/// Chromium's stock error page). Themed to match the app; the "Try again"
/// button reloads WhatsApp Web.
[[nodiscard]] QString errorPageHtml(bool dark, const QString& title, const QString& detail);

} // namespace whatsie::web
