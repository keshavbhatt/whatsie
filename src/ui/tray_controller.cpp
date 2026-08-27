#include "ui/tray_controller.h"

#include "core/settings/settings.h"
#include "core/unread_badge.h"
#include "ui/actions.h"
#include "ui/logging.h"

#include <QAction>
#include <QMenu>
#include <QPixmap>
#include <QSystemTrayIcon>

using namespace Qt::StringLiterals;

namespace whatsie::ui {

TrayController::TrayController(core::Settings& settings, Actions& actions, QObject* parent)
    : QObject(parent)
    , m_settings(settings)
    , m_actions(actions)
    , m_baseImage(u":/icons/hicolor/128x128/apps/com.ktechpit.whatsie.png"_s)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qCWarning(lcUi) << "no system tray available; window will never auto-hide";
        rebuildIcon();
        return;
    }

    m_menu = new QMenu();
    m_menu->addAction(m_actions.showHide);
    m_menu->addAction(m_actions.newChat);
    m_menu->addAction(m_actions.reload);
    m_menu->addSeparator();
    m_menu->addAction(m_actions.settings);
    m_menu->addAction(m_actions.about);
    m_menu->addSeparator();
    m_menu->addAction(m_actions.quit);

    m_tray = new QSystemTrayIcon(this);
    m_tray->setContextMenu(m_menu);
    rebuildIcon();
    connect(m_tray, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason != QSystemTrayIcon::Trigger && reason != QSystemTrayIcon::DoubleClick) {
            return;
        }
        if (m_settings.trayLeftClickToggles()) {
            Q_EMIT toggleRequested();
        } else {
            Q_EMIT showRequested();
        }
    });
    m_tray->show();
    qCDebug(lcUi) << "tray icon shown";
}

TrayController::~TrayController()
{
    delete m_menu;
}

bool TrayController::isAvailable() const
{
    return m_tray != nullptr && QSystemTrayIcon::isSystemTrayAvailable();
}

void TrayController::setUnreadCount(int count)
{
    if (count == m_unread) {
        return;
    }
    m_unread = count;
    rebuildIcon();
}

void TrayController::setWindowVisible(bool visible)
{
    m_actions.showHide->setText(visible ? tr("Hide to tray") : tr("Show window"));
}

void TrayController::rebuildIcon()
{
    m_icon = QIcon(QPixmap::fromImage(core::composeUnreadBadge(m_baseImage, m_unread)));
    if (m_tray == nullptr) {
        return;
    }
    m_tray->setIcon(m_icon);
    m_tray->setToolTip(m_unread > 0 ? tr("WhatsApp — %n unread", nullptr, m_unread) : u"WhatsApp"_s);
}

} // namespace whatsie::ui
