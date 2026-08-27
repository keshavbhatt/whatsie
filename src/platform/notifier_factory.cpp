#include "platform/notifier_factory.h"

#include "core/notifications/notification.h"
#include "platform/logging.h"

#ifdef Q_OS_LINUX
#include "platform/linux/freedesktop_notifier.h"
#include "platform/linux/portal_notifier.h"

#include <QtEnvironmentVariables>
#endif

namespace whatsie::platform {

std::unique_ptr<core::INotifier> createPlatformNotifier(QObject* parent)
{
#ifdef Q_OS_LINUX
    if (qEnvironmentVariableIsSet("FLATPAK_ID")) {
        auto portal = std::make_unique<linux_::PortalNotifier>(QDBusConnection::sessionBus(), parent);
        if (portal->isAvailable()) {
            return portal;
        }
        qCWarning(lcPlatform) << "notification portal unavailable inside Flatpak";
    }
    auto fdo = std::make_unique<linux_::FreedesktopNotifier>(QDBusConnection::sessionBus(), parent);
    if (fdo->isAvailable()) {
        return fdo;
    }
    qCWarning(lcPlatform) << "no org.freedesktop.Notifications service on the session bus";
    return nullptr;
#else
    Q_UNUSED(parent)
    return nullptr;
#endif
}

} // namespace whatsie::platform
