#include "ui/notification_hub.h"

#include "core/notifications/notification_service.h"
#include "platform/notifier_factory.h"
#include "ui/logging.h"
#include "ui/tray_controller.h"
#include "ui/tray_notifier.h"
#include "web/notification_presenter.h"
#include "web/web_view.h"

using namespace Qt::StringLiterals;

namespace whatsie::ui {

NotificationHub::NotificationHub(core::Settings& settings, core::DndController& dnd, TrayController& tray,
                                 web::WebView& view, QObject* parent)
    : QObject(parent)
    , m_platformNotifier(platform::createPlatformNotifier())
    , m_trayNotifier(std::make_unique<TrayNotifier>(tray))
    , m_service(std::make_unique<core::NotificationService>(settings, dnd, m_platformNotifier.get(),
                                                            m_trayNotifier.get()))
    , m_presenter(std::make_unique<web::NotificationPresenter>(view.profile(), *m_service))
{
    connect(m_presenter.get(), &web::NotificationPresenter::activated, this, &NotificationHub::activated);
    // Test notifications (no presenter entry) also raise the window on click.
    connect(m_service.get(), &core::NotificationService::activated, this, [this](quint64 id) {
        if (id == m_testId) {
            Q_EMIT activated();
        }
    });
}

NotificationHub::~NotificationHub() = default;

void NotificationHub::sendTest()
{
    core::Notification n;
    n.title = tr("Whatsie");
    n.body = tr("This is how a message notification looks.");
    n.sender = tr("Whatsie");
    m_testId = m_service->notify(n);
    qCInfo(lcUi) << "test notification id" << m_testId << "via" << m_service->backendName();
}

} // namespace whatsie::ui
