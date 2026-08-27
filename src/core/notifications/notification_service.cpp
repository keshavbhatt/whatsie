#include "core/notifications/notification_service.h"

#include "core/logging.h"
#include "core/notifications/dnd_controller.h"
#include "core/notifications/identicon.h"
#include "core/settings/settings.h"

#include <QGuiApplication>

using namespace Qt::StringLiterals;

namespace whatsie::core {

NotificationService::NotificationService(Settings& settings, DndController& dnd, INotifier* primary,
                                         INotifier* fallback, QObject* parent)
    : QObject(parent)
    , m_settings(settings)
    , m_dnd(dnd)
    , m_primary(primary)
    , m_fallback(fallback)
{
    attach(m_primary);
    attach(m_fallback);
    qCInfo(lcCore) << "notification backend:" << backendName()
                   << (m_fallback != nullptr ? u"(fallback: %1)"_s.arg(m_fallback->name()) : QString());
}

void NotificationService::attach(INotifier* notifier)
{
    if (notifier == nullptr) {
        return;
    }
    connect(notifier, &INotifier::activated, this, [this](quint64 id) {
        if (m_active.contains(id)) {
            Q_EMIT activated(id);
        }
    });
    connect(notifier, &INotifier::closed, this, &NotificationService::handleClosed);
    connect(notifier, &INotifier::failed, this, &NotificationService::handleFailed);
}

INotifier* NotificationService::pick() const
{
    if (m_primary != nullptr && m_primary->isAvailable()) {
        return m_primary;
    }
    if (m_fallback != nullptr && m_fallback->isAvailable()) {
        return m_fallback;
    }
    return nullptr;
}

QString NotificationService::backendName() const
{
    INotifier* n = pick();
    return n != nullptr ? n->name() : u"none"_s;
}

bool NotificationService::isSuppressed() const
{
    return m_lockSuppressed || !m_settings.notificationsEnabled() || m_dnd.isActive();
}

quint64 NotificationService::notify(Notification notification)
{
    if (isSuppressed()) {
        qCDebug(lcCore) << "notification suppressed:" << notification.title;
        return 0;
    }
    INotifier* backend = pick();
    if (backend == nullptr) {
        qCWarning(lcCore) << "no notification backend available; dropping" << notification.title;
        return 0;
    }

    if (notification.image.isNull()) {
        notification.image =
            identicon(notification.sender.isEmpty() ? notification.title : notification.sender);
    }
    notification.sound = m_settings.notificationSound();
    const int timeoutSec = m_settings.notificationTimeoutSec();
    notification.timeoutMs = timeoutSec > 0 ? timeoutSec * 1000 : -1;
    if (notification.desktopEntry.isEmpty() && QGuiApplication::instance() != nullptr) {
        notification.desktopEntry = QGuiApplication::desktopFileName();
    }

    const quint64 id = m_nextId++;
    m_active.insert(id, notification);
    m_owner.insert(id, backend);
    backend->show(id, notification);
    return id;
}

void NotificationService::close(quint64 id)
{
    if (INotifier* owner = m_owner.value(id, nullptr)) {
        owner->close(id);
    }
    handleClosed(id);
}

void NotificationService::handleClosed(quint64 id)
{
    if (!m_active.remove(id)) {
        return;
    }
    m_owner.remove(id);
    Q_EMIT closed(id);
}

void NotificationService::handleFailed(quint64 id, const QString& reason)
{
    if (!m_active.contains(id)) {
        return;
    }
    INotifier* failedBackend = m_owner.value(id, nullptr);
    INotifier* next = (failedBackend == m_primary && m_fallback != nullptr && m_fallback->isAvailable())
                          ? m_fallback
                          : nullptr;
    if (next == nullptr) {
        qCWarning(lcCore) << "notification" << id << "failed with no fallback:" << reason;
        handleClosed(id);
        return;
    }
    qCWarning(lcCore) << "notification" << id << "failed on" << failedBackend->name() << ':' << reason
                      << "- retrying with" << next->name();
    m_owner.insert(id, next);
    next->show(id, m_active.value(id));
}

} // namespace whatsie::core
