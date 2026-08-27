#include "ui/tray_controller.h"

#include "core/notifications/dnd_controller.h"
#include "core/settings/settings.h"
#include "core/unread_badge.h"
#include "ui/actions.h"
#include "ui/logging.h"

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QPixmap>
#include <QSystemTrayIcon>

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

namespace whatsie::ui {

TrayController::TrayController(core::Settings& settings, core::DndController& dnd, Actions& actions,
                               QObject* parent)
    : QObject(parent)
    , m_settings(settings)
    , m_dnd(dnd)
    , m_actions(actions)
    , m_baseImage(u":/icons/hicolor/128x128/apps/com.ktechpit.whatsie.png"_s)
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        qCWarning(lcUi) << "no system tray available; window will never auto-hide";
        rebuildIcon();
        return;
    }

    buildMenu();
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
    connect(&m_dnd, &core::DndController::stateChanged, this, [this](bool active) {
        if (!active && m_dndOff != nullptr) {
            m_dndOff->setChecked(true);
        }
        updateTooltip();
    });
    m_tray->show();
    qCDebug(lcUi) << "tray icon shown";
}

TrayController::~TrayController()
{
    delete m_menu;
}

void TrayController::buildMenu()
{
    m_menu = new QMenu();
    m_menu->addAction(m_actions.showHide);
    m_menu->addAction(m_actions.newChat);
    m_menu->addAction(m_actions.reload);
    m_menu->addAction(m_actions.downloads);
    m_menu->addSeparator();
    m_menu->addAction(m_actions.mute);
    m_menu->addAction(m_actions.blurMessages);
    m_menu->addMenu(buildDndMenu());
    m_menu->addSeparator();
    m_menu->addAction(m_actions.settings);
    m_menu->addAction(m_actions.about);
    m_menu->addSeparator();
    m_menu->addAction(m_actions.quit);
}

QMenu* TrayController::buildDndMenu()
{
    auto* menu = new QMenu(tr("Do not disturb"), m_menu);
    auto* group = new QActionGroup(menu);
    group->setExclusive(true);

    auto add = [&](const QString& text, auto&& apply) {
        QAction* action = menu->addAction(text);
        action->setCheckable(true);
        group->addAction(action);
        connect(action, &QAction::triggered, this, [apply](bool checked) {
            if (checked) {
                apply();
            }
        });
        return action;
    };
    m_dndOff = add(tr("Off"), [this] { m_dnd.disable(); });
    add(tr("For 1 hour"), [this] { m_dnd.enableFor(60min); });
    add(tr("For 2 hours"), [this] { m_dnd.enableFor(120min); });
    add(tr("Until I turn it off"), [this] { m_dnd.enableIndefinitely(); });
    m_dndOff->setChecked(true);
    return menu;
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
    updateTooltip();
}

void TrayController::updateTooltip()
{
    if (m_tray == nullptr) {
        return;
    }
    QStringList parts{u"WhatsApp"_s};
    if (m_unread > 0) {
        parts << tr("%n unread", nullptr, m_unread);
    }
    if (m_dnd.isActive()) {
        const auto until = m_dnd.until();
        parts << (until ? tr("Do not disturb until %1").arg(until->toString(u"HH:mm"_s))
                        : tr("Do not disturb"));
    }
    m_tray->setToolTip(parts.join(u" — "_s));
}

} // namespace whatsie::ui
