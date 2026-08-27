#pragma once

#include <QLoggingCategory>

// Logging categories for the core layer. Every layer declares its own in its
// `logging.h`; category names are dotted under "whatsie." so they can be
// enabled with QT_LOGGING_RULES="whatsie.*.debug=true".
namespace whatsie::core {

Q_DECLARE_LOGGING_CATEGORY(lcCore)
Q_DECLARE_LOGGING_CATEGORY(lcSettings)

} // namespace whatsie::core
