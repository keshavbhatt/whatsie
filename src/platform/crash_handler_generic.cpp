#include "platform/crash_handler.h"

namespace whatsie::platform {

void installCrashHandler(const QString& /*crashFilePath*/) {}

QString lastCrashReport()
{
    return {};
}

} // namespace whatsie::platform
