#include "ui/tray_notifier.h"

#include "ui/tray_controller.h"

#include <QPixmap>
#include <QSystemTrayIcon>

namespace whatsie::ui {

namespace {
constexpr int kDefaultBalloonMs = 8000;
}

TrayNotifier::TrayNotifier(TrayController& tray, QObject* parent)
    : core::INotifier(parent)
    , m_tray(tray)
{
    if (QSystemTrayIcon* icon = m_tray.systemTrayIcon()) {
        connect(icon, &QSystemTrayIcon::messageClicked, this, [this] {
            if (m_current != 0) {
                Q_EMIT activated(m_current);
                Q_EMIT closed(std::exchange(m_current, 0));
            }
        });
    }
}

bool TrayNotifier::isAvailable() const
{
    return m_tray.isAvailable() && QSystemTrayIcon::supportsMessages();
}

void TrayNotifier::show(quint64 id, const core::Notification& n)
{
    QSystemTrayIcon* icon = m_tray.systemTrayIcon();
    if (icon == nullptr) {
        Q_EMIT failed(id, QStringLiteral("no tray icon"));
        return;
    }
    if (m_current != 0) {
        Q_EMIT closed(std::exchange(m_current, 0)); // replaced by the new balloon
    }
    m_current = id;
    const int timeout = n.timeoutMs > 0 ? n.timeoutMs : kDefaultBalloonMs;
    icon->showMessage(n.title, n.body, QIcon(QPixmap::fromImage(n.image)), timeout);
}

void TrayNotifier::close(quint64 id)
{
    if (id == m_current) {
        m_current = 0;
        // QSystemTrayIcon has no API to dismiss a balloon.
    }
}

} // namespace whatsie::ui
