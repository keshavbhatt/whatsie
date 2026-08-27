#pragma once

#include "core/notifications/notification.h"

#include <QHash>
#include <QObject>

namespace whatsie::core {

class DndController;
class Settings;

/// Decides whether a notification is shown, assigns ids, fills defaults
/// (identicon, sound, timeout, desktop entry) and routes it to the primary
/// backend with an optional fallback (FEATURES N1, N5, N6, N7, D5).
class NotificationService : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(NotificationService)

public:
    /// `primary` may be null (then `fallback` is used directly); `fallback` may be null.
    NotificationService(Settings& settings, DndController& dnd, INotifier* primary, INotifier* fallback,
                        QObject* parent = nullptr);
    ~NotificationService() override = default;

    /// Returns the id, or 0 when the notification was suppressed.
    quint64 notify(Notification notification);
    void close(quint64 id);

    [[nodiscard]] bool isSuppressed() const;
    [[nodiscard]] int activeCount() const { return static_cast<int>(m_active.size()); }
    [[nodiscard]] QString backendName() const;

Q_SIGNALS:
    void activated(quint64 id);
    void closed(quint64 id);

private:
    void attach(INotifier* notifier);
    [[nodiscard]] INotifier* pick() const;
    void handleFailed(quint64 id, const QString& reason);
    void handleClosed(quint64 id);

    Settings& m_settings;
    DndController& m_dnd;
    INotifier* m_primary;
    INotifier* m_fallback;
    QHash<quint64, Notification> m_active;
    QHash<quint64, INotifier*> m_owner;
    quint64 m_nextId = 1;
};

} // namespace whatsie::core
