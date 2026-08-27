#pragma once

#include <QObject>
#include <QWebEngineNotification>

#include <memory>
#include <unordered_map>

namespace whatsie::core {
class NotificationService;
}

namespace whatsie::web {

class WebProfile;

/// Bridges WhatsApp Web's `new Notification(...)` calls to the
/// NotificationService and feeds clicks back so WhatsApp's own onclick opens
/// the chat (FEATURES N1, N10). Owns the QWebEngineNotification objects.
class NotificationPresenter : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(NotificationPresenter)

public:
    NotificationPresenter(WebProfile& profile, core::NotificationService& service, QObject* parent = nullptr);
    ~NotificationPresenter() override;

Q_SIGNALS:
    /// A web notification was clicked; the window should come to the front.
    void activated();

private:
    void present(std::unique_ptr<QWebEngineNotification> notification);
    void handleActivated(quint64 id);
    void handleClosed(quint64 id);

    WebProfile& m_profile;
    core::NotificationService& m_service;
    std::unordered_map<quint64, std::unique_ptr<QWebEngineNotification>> m_active;
};

} // namespace whatsie::web
