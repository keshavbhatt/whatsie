#pragma once

#include "core/notifications/notification.h"

#include <QDBusConnection>
#include <QSet>

namespace whatsie::platform::linux_ {

/// org.freedesktop.portal.Notification client for sandboxed (Flatpak) builds
/// (FEATURES N3). The call is awaited, so a failure
/// is reported and the service can fall back.
class PortalNotifier : public core::INotifier
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(PortalNotifier)

public:
    explicit PortalNotifier(const QDBusConnection& bus = QDBusConnection::sessionBus(),
                            QObject* parent = nullptr);
    ~PortalNotifier() override = default;

    [[nodiscard]] QString name() const override { return QStringLiteral("portal"); }
    [[nodiscard]] bool isAvailable() const override;
    void show(quint64 id, const core::Notification& notification) override;
    void close(quint64 id) override;

private Q_SLOTS:
    void onActionInvoked(const QString& id, const QString& action, const QVariantList& parameters);

private:
    [[nodiscard]] static QString portalId(quint64 id);

    QDBusConnection m_bus;
    QSet<quint64> m_active;
};

} // namespace whatsie::platform::linux_
