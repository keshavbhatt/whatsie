// Tray icon, actions, and window-title/notification-count handling.
#include "mainwindow.h"
#include "common.h"

#include <algorithm>

#include <QShortcut>
#include <QStyleHints>

// ── Actions ──────────────────────────────────────────────────────────────────

void MainWindow::createActions() {
  m_openUrlAction = new QAction("New Chat", this);
  m_openUrlAction->setShortcut(QKeySequence(Qt::Modifier::CTRL | Qt::Key_N));
  connect(m_openUrlAction, &QAction::triggered, this, &MainWindow::newChat);
  addAction(m_openUrlAction);

  m_fullscreenAction = new QAction(tr("Fullscreen"), this);
  m_fullscreenAction->setShortcut(Qt::Key_F11);
  connect(m_fullscreenAction, &QAction::triggered, m_fullscreenAction,
          [=]() { setWindowState(windowState() ^ Qt::WindowFullScreen); });
  addAction(m_fullscreenAction);

  m_minimizeAction = new QAction(tr("Mi&nimize to tray"), this);
  connect(m_minimizeAction, &QAction::triggered, this, &QMainWindow::hide);
  addAction(m_minimizeAction);

  QShortcut *minimizeShortcut = new QShortcut(
      QKeySequence(Qt::Modifier::CTRL | Qt::Key_W), this, SLOT(hide()));
  minimizeShortcut->setAutoRepeat(false);

  m_restoreAction = new QAction(tr("&Restore"), this);
  connect(m_restoreAction, &QAction::triggered, this, &QMainWindow::show);
  addAction(m_restoreAction);

  m_reloadAction = new QAction(tr("Re&load"), this);
  m_reloadAction->setShortcut(Qt::Key_F5);
  connect(m_reloadAction, &QAction::triggered, this,
          [=]() { this->doReload(); });
  addAction(m_reloadAction);

  m_lockAction = new QAction(tr("Loc&k"), this);
  m_lockAction->setShortcut(QKeySequence(Qt::Modifier::CTRL | Qt::Key_L));
  connect(m_lockAction, &QAction::triggered, this, &MainWindow::lockApp);
  addAction(m_lockAction);

  m_settingsAction = new QAction(tr("&Settings"), this);
  m_settingsAction->setShortcut(QKeySequence(Qt::Modifier::CTRL | Qt::Key_P));
  connect(m_settingsAction, &QAction::triggered, this,
          &MainWindow::showSettings);
  addAction(m_settingsAction);

  m_toggleThemeAction = new QAction(tr("&Toggle theme"), this);
  m_toggleThemeAction->setShortcut(
      QKeySequence(Qt::Modifier::CTRL | Qt::Key_T));
  connect(m_toggleThemeAction, &QAction::triggered, this,
          &MainWindow::toggleTheme);
  addAction(m_toggleThemeAction);

  m_aboutAction = new QAction(tr("&About"), this);
  connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

  m_quitAction = new QAction(tr("&Quit"), this);
  m_quitAction->setShortcut(QKeySequence(Qt::Modifier::CTRL | Qt::Key_Q));
  connect(m_quitAction, &QAction::triggered, this, &MainWindow::quitApp);
  addAction(m_quitAction);
}

// ── Tray icon ─────────────────────────────────────────────────────────────────

void MainWindow::createTrayIcon() {
  m_trayIconMenu = new QMenu(this);
  m_trayIconMenu->setObjectName("trayIconMenu");
  m_trayIconMenu->addAction(m_minimizeAction);
  m_trayIconMenu->addAction(m_restoreAction);
  m_trayIconMenu->addSeparator();
  m_trayIconMenu->addAction(m_reloadAction);
  m_trayIconMenu->addAction(m_lockAction);
  m_trayIconMenu->addSeparator();
  m_trayIconMenu->addAction(m_openUrlAction);
  m_trayIconMenu->addAction(m_toggleThemeAction);
  m_trayIconMenu->addAction(m_settingsAction);
  m_trayIconMenu->addAction(m_aboutAction);
  m_trayIconMenu->addSeparator();
  m_trayIconMenu->addAction(m_quitAction);

  m_systemTrayIcon = new QSystemTrayIcon(m_trayIconNormal, this);
  m_systemTrayIcon->setContextMenu(m_trayIconMenu);
  connect(m_trayIconMenu, &QMenu::aboutToShow, this,
          &MainWindow::checkWindowState);
  connect(m_systemTrayIcon, &QSystemTrayIcon::activated, this,
          &MainWindow::iconActivated);

  m_systemTrayIcon->show();

  if (qApp->styleHints()->showShortcutsInContextMenus()) {
    foreach (QAction *action, m_trayIconMenu->actions()) {
      action->setShortcutVisibleInContextMenu(true);
    }
  }
}

void MainWindow::checkWindowState() {
  QObject *tray_icon_menu = this->findChild<QObject *>("trayIconMenu");
  if (tray_icon_menu == nullptr)
    return;

  QMenu *menu = qobject_cast<QMenu *>(tray_icon_menu);
  if (this->isVisible()) {
    menu->actions().at(0)->setDisabled(false);
    menu->actions().at(1)->setDisabled(true);
  } else {
    menu->actions().at(0)->setDisabled(true);
    menu->actions().at(1)->setDisabled(false);
  }
  menu->actions().at(4)->setDisabled(m_lockWidget && m_lockWidget->getIsLocked());
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (SettingsManager::instance()
              .settings()
              .value("minimizeOnTrayIconClick", false)
              .toBool() == false ||
      reason == QSystemTrayIcon::Context)
    return;
  if (isVisible()) {
    hide();
  } else {
    show();
  }
}

const QIcon MainWindow::getTrayIcon(const int &notificationCount) const {
  if (notificationCount == 0)
    return themeIcon("whatsie-tray", ":/icons/app/notification/whatsie-notify.png");

  return themeIcon("whatsie-tray-attentions",
    QString(":/icons/app/notification/whatsie-notify-%1.png").arg(std::clamp(notificationCount, 1, 10)));
}

void MainWindow::handleWebViewTitleChanged(const QString &title) {
  setWindowTitle(QApplication::applicationName() + ": " + title);

  QRegularExpressionMatch notificationsTitleMatch =
      m_notificationsTitleRegExp.match(title);

  if (notificationsTitleMatch.hasMatch()) {
    QString capturedTitle = notificationsTitleMatch.captured(0);
    QRegularExpressionMatch unreadMessageCountMatch =
        m_unreadMessageCountRegExp.match(capturedTitle);

    if (unreadMessageCountMatch.hasMatch()) {
      QString unreadMessageCountStr = unreadMessageCountMatch.captured(1);
      int unreadMessageCount = unreadMessageCountStr.toInt();

      m_restoreAction->setText(
          tr("Restore") + " | " + unreadMessageCountStr + " " +
          (unreadMessageCount > 1 ? tr("messages") : tr("message")));

      m_systemTrayIcon->setIcon(getTrayIcon(unreadMessageCount));
      setWindowIcon(getTrayIcon(unreadMessageCount));
    }
  } else {
    m_systemTrayIcon->setIcon(m_trayIconNormal);
    setWindowIcon(themeIcon("whatsie", ":/icons/app/icon-64.png"));
  }
}
