#pragma once

#include <QLoggingCategory>

namespace whatsie::web {

Q_DECLARE_LOGGING_CATEGORY(lcWeb)
/// Console output of the page itself (console.log/warn/error from WhatsApp Web
/// and from our injected scripts).
Q_DECLARE_LOGGING_CATEGORY(lcWebJs)

} // namespace whatsie::web
