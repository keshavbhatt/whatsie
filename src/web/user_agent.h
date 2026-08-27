#pragma once

#include <QString>

namespace whatsie::web {

/// WhatsApp Web refuses or degrades "unknown" browsers. Qt WebEngine's default
/// user agent is a real Chrome UA with an extra "QtWebEngine/x.y.z" token;
/// removing that token is enough to be treated as Chrome.
///
/// Pure function so it can be unit-tested without a profile.
[[nodiscard]] QString sanitizeUserAgent(const QString& defaultUserAgent);

/// Returns `override` if non-empty, otherwise the sanitized engine default.
[[nodiscard]] QString effectiveUserAgent(const QString& engineDefault, const QString& userOverride);

} // namespace whatsie::web
