#include "web/notification_presenter.h"

#include "core/notifications/notification_service.h"
#include "web/logging.h"
#include "web/web_profile.h"

namespace whatsie::web {

NotificationPresenter::NotificationPresenter(WebProfile& profile, core::NotificationService& service,
                                             QObject* parent)
    : QObject(parent)
    , m_profile(profile)
    , m_service(service)
{
    m_profile.setNotificationPresenter(
        [this](std::unique_ptr<QWebEngineNotification> n) { present(std::move(n)); });
    connect(&m_service, &core::NotificationService::activated, this, &NotificationPresenter::handleActivated);
    connect(&m_service, &core::NotificationService::closed, this, &NotificationPresenter::handleClosed);
}

NotificationPresenter::~NotificationPresenter()
{
    m_profile.setNotificationPresenter(nullptr);
}

void NotificationPresenter::present(std::unique_ptr<QWebEngineNotification> n)
{
    core::Notification model;
    model.title = n->title();
    model.body = n->message();
    model.image = n->icon();
    model.sender = n->title();

    const quint64 id = m_service.notify(model);
    if (id == 0) {
        n->close(); // suppressed: tell the page so it does not keep it around
        return;
    }
    qCDebug(lcWeb) << "web notification" << id << "from" << n->origin();
    QWebEngineNotification* raw = n.get();
    m_active.emplace(id, std::move(n));
    connect(raw, &QWebEngineNotification::closed, this, [this, id] {
        m_service.close(id);
        m_active.erase(id);
    });
}

void NotificationPresenter::handleActivated(quint64 id)
{
    const auto it = m_active.find(id);
    if (it == m_active.end()) {
        return;
    }
    it->second->click(); // WhatsApp Web's own handler focuses the chat
    Q_EMIT activated();
}

void NotificationPresenter::handleClosed(quint64 id)
{
    const auto it = m_active.find(id);
    if (it == m_active.end()) {
        return;
    }
    it->second->close();
    m_active.erase(it);
}

} // namespace whatsie::web
