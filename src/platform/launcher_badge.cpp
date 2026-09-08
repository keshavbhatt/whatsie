#include "platform/launcher_badge.h"

using namespace Qt::StringLiterals;

namespace whatsie::platform {

QString launcherEntryUri(const QString& desktopId)
{
    if (desktopId.isEmpty()) {
        return {};
    }
    return u"application://%1.desktop"_s.arg(desktopId);
}

} // namespace whatsie::platform
