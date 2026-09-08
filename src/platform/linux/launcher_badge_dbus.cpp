#include "platform/launcher_badge.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QVariantMap>

#include <algorithm>

using namespace Qt::StringLiterals;

namespace whatsie::platform {

void setLauncherCount(const QString& desktopId, int count)
{
    const QString uri = launcherEntryUri(desktopId);
    if (uri.isEmpty()) {
        return;
    }
    // A plain session-bus signal — no libunity dependency. Listeners match on the
    // app URI in the payload, so the object path is only for namespacing.
    QDBusMessage signal = QDBusMessage::createSignal(u"/com/ktechpit/whatsie/LauncherEntry"_s,
                                                     u"com.canonical.Unity.LauncherEntry"_s,
                                                     u"Update"_s);
    QVariantMap props;
    props.insert(u"count"_s, static_cast<qlonglong>(std::max(0, count)));
    props.insert(u"count-visible"_s, count > 0);
    signal << uri << props;
    QDBusConnection::sessionBus().send(signal);
}

} // namespace whatsie::platform
