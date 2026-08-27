#pragma once

#include "core/notifications/notification.h"

#include <QDBusConnection>
#include <QHash>

namespace whatsie::platform::linux_ {

/// org.freedesktop.Notifications client over QtDBus (ADR-009, FEATURES N1).
/// Tracks the server-assigned ids so ActionInvoked / NotificationClosed are
/// matched to *our* notifications only.
class FreedesktopNotifier : public core::INotifier
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(FreedesktopNotifier)

public:
    explicit FreedesktopNotifier(const QDBusConnection& bus = QDBusConnection::sessionBus(),
                                 QObject* parent = nullptr);
    ~FreedesktopNotifier() override = default;

    [[nodiscard]] QString name() const override { return QStringLiteral("freedesktop"); }
    [[nodiscard]] bool isAvailable() const override;
    void show(quint64 id, const core::Notification& notification) override;
    void close(quint64 id) override;

private Q_SLOTS:
    // String-based slots: QDBusConnection::connect() only accepts SLOT() names.
    void onActionInvoked(uint dbusId, const QString& actionKey);
    void onNotificationClosed(uint dbusId, uint reason);

private:
    QDBusConnection m_bus;
    QHash<quint32, quint64> m_localByDBus;
    QHash<quint64, quint32> m_dbusByLocal;
};

} // namespace whatsie::platform::linux_
