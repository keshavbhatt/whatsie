#include "platform/linux/freedesktop_notifier.h"

#include "platform/linux/dbus_image.h"
#include "platform/logging.h"

#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QGuiApplication>

using namespace Qt::StringLiterals;

namespace whatsie::platform::linux_ {

namespace {
const QString kService = u"org.freedesktop.Notifications"_s;
const QString kPath = u"/org/freedesktop/Notifications"_s;
const QString kInterface = u"org.freedesktop.Notifications"_s;
const QString kDefaultAction = u"default"_s;
constexpr uchar kUrgencyNormal = 1;
} // namespace

FreedesktopNotifier::FreedesktopNotifier(const QDBusConnection& bus, QObject* parent)
    : core::INotifier(parent)
    , m_bus(bus)
{
    registerDBusImageType();
    // NOLINTBEGIN: QDBusConnection::connect has no pointer-to-member overload.
    m_bus.connect(kService, kPath, kInterface, u"ActionInvoked"_s, this,
                  SLOT(onActionInvoked(uint, QString)));
    m_bus.connect(kService, kPath, kInterface, u"NotificationClosed"_s, this,
                  SLOT(onNotificationClosed(uint, uint)));
    // NOLINTEND
}

bool FreedesktopNotifier::isAvailable() const
{
    if (!m_bus.isConnected()) {
        return false;
    }
    QDBusConnectionInterface* iface = m_bus.interface();
    if (iface == nullptr) {
        return false;
    }
    if (iface->isServiceRegistered(kService)) {
        return true;
    }
    // Not running yet but D-Bus can activate it on first use.
    const QDBusReply<QStringList> activatable = iface->activatableServiceNames();
    return activatable.isValid() && activatable.value().contains(kService);
}

void FreedesktopNotifier::show(quint64 id, const core::Notification& n)
{
    QVariantMap hints;
    hints.insert(u"urgency"_s, QVariant::fromValue(kUrgencyNormal));
    hints.insert(u"category"_s, n.category);
    if (!n.desktopEntry.isEmpty()) {
        hints.insert(u"desktop-entry"_s, n.desktopEntry);
    }
    if (!n.image.isNull()) {
        hints.insert(u"image-data"_s, QVariant::fromValue(toDBusImage(n.image)));
    }
    if (n.sound) {
        hints.insert(u"sound-name"_s, u"message-new-instant"_s);
    }

    const QString appName =
        QGuiApplication::instance() != nullptr ? QGuiApplication::applicationDisplayName() : u"Whatsie"_s;
    const QString appIcon = n.desktopEntry; // themed icon name = desktop id when installed

    QDBusMessage call = QDBusMessage::createMethodCall(kService, kPath, kInterface, u"Notify"_s);
    call << appName << quint32(0) << appIcon << n.title << n.body << QStringList{kDefaultAction, tr("Open")}
         << hints << n.timeoutMs;

    auto* watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(call), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, id](QDBusPendingCallWatcher* w) {
        w->deleteLater();
        const QDBusPendingReply<quint32> reply = *w;
        if (reply.isError()) {
            qCWarning(lcPlatform) << "Notify failed:" << reply.error().message();
            Q_EMIT failed(id, reply.error().message());
            return;
        }
        const quint32 dbusId = reply.value();
        m_localByDBus.insert(dbusId, id);
        m_dbusByLocal.insert(id, dbusId);
        qCDebug(lcPlatform) << "notification" << id << "shown as dbus id" << dbusId;
    });
}

void FreedesktopNotifier::close(quint64 id)
{
    const auto it = m_dbusByLocal.constFind(id);
    if (it == m_dbusByLocal.constEnd()) {
        return;
    }
    QDBusMessage call = QDBusMessage::createMethodCall(kService, kPath, kInterface, u"CloseNotification"_s);
    call << it.value();
    m_bus.asyncCall(call);
}

void FreedesktopNotifier::onActionInvoked(uint dbusId, const QString& actionKey)
{
    const auto it = m_localByDBus.constFind(dbusId);
    if (it == m_localByDBus.constEnd()) {
        return; // someone else's notification — the W#278 bug, prevented
    }
    qCDebug(lcPlatform) << "action" << actionKey << "on notification" << it.value();
    Q_EMIT activated(it.value());
}

void FreedesktopNotifier::onNotificationClosed(uint dbusId, uint reason)
{
    const auto it = m_localByDBus.constFind(dbusId);
    if (it == m_localByDBus.constEnd()) {
        return;
    }
    const quint64 id = it.value();
    m_localByDBus.erase(it);
    m_dbusByLocal.remove(id);
    qCDebug(lcPlatform) << "notification" << id << "closed, reason" << reason;
    Q_EMIT closed(id);
}

} // namespace whatsie::platform::linux_
