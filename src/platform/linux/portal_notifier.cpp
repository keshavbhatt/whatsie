#include "platform/linux/portal_notifier.h"

#include "platform/logging.h"

#include <QBuffer>
#include <QDBusArgument>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>

using namespace Qt::StringLiterals;

namespace whatsie::platform::linux_ {

namespace {
const QString kService = u"org.freedesktop.portal.Desktop"_s;
const QString kPath = u"/org/freedesktop/portal/desktop"_s;
const QString kInterface = u"org.freedesktop.portal.Notification"_s;
const QString kDefaultAction = u"open"_s;

QVariant iconVariant(const QImage& image)
{
    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    // (sv): ("bytes", <ay>)
    QDBusArgument arg;
    arg.beginStructure();
    arg << u"bytes"_s << QDBusVariant(png);
    arg.endStructure();
    return QVariant::fromValue(arg);
}
} // namespace

PortalNotifier::PortalNotifier(const QDBusConnection& bus, QObject* parent)
    : core::INotifier(parent)
    , m_bus(bus)
{
    // NOLINTNEXTLINE: QDBusConnection::connect has no pointer-to-member overload.
    m_bus.connect(kService, kPath, kInterface, u"ActionInvoked"_s, this,
                  SLOT(onActionInvoked(QString, QString, QVariantList)));
}

bool PortalNotifier::isAvailable() const
{
    if (!m_bus.isConnected() || m_bus.interface() == nullptr) {
        return false;
    }
    return m_bus.interface()->isServiceRegistered(kService);
}

QString PortalNotifier::portalId(quint64 id)
{
    return u"whatsie-%1"_s.arg(id);
}

void PortalNotifier::show(quint64 id, const core::Notification& n)
{
    QVariantMap options;
    options.insert(u"title"_s, n.title);
    options.insert(u"body"_s, n.body);
    options.insert(u"priority"_s, u"normal"_s);
    options.insert(u"default-action"_s, kDefaultAction);
    if (!n.image.isNull()) {
        options.insert(u"icon"_s, iconVariant(n.image));
    }

    QDBusMessage call = QDBusMessage::createMethodCall(kService, kPath, kInterface, u"AddNotification"_s);
    call << portalId(id) << options;

    auto* watcher = new QDBusPendingCallWatcher(m_bus.asyncCall(call), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, id](QDBusPendingCallWatcher* w) {
        w->deleteLater();
        const QDBusPendingReply<> reply = *w;
        if (reply.isError()) {
            qCWarning(lcPlatform) << "portal AddNotification failed:" << reply.error().message();
            Q_EMIT failed(id, reply.error().message());
            return;
        }
        m_active.insert(id);
    });
}

void PortalNotifier::close(quint64 id)
{
    if (!m_active.remove(id)) {
        return;
    }
    QDBusMessage call = QDBusMessage::createMethodCall(kService, kPath, kInterface, u"RemoveNotification"_s);
    call << portalId(id);
    m_bus.asyncCall(call);
}

void PortalNotifier::onActionInvoked(const QString& id, const QString& action, const QVariantList& parameters)
{
    Q_UNUSED(parameters)
    if (!id.startsWith(u"whatsie-"_s)) {
        return;
    }
    bool ok = false;
    const quint64 local = id.mid(8).toULongLong(&ok);
    if (!ok || !m_active.contains(local)) {
        return;
    }
    qCDebug(lcPlatform) << "portal action" << action << "on notification" << local;
    Q_EMIT activated(local);
    m_active.remove(local);
    Q_EMIT closed(local);
}

} // namespace whatsie::platform::linux_
