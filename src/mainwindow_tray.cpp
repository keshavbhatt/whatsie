// Tray icon, actions, and window-title/notification-count handling.
#include "mainwindow.h"
#include "common.h"

#include <algorithm>

#include <QGuiApplication>
#include <QShortcut>
#include <QStyleHints>
#include <QWindow>

#if __has_include(<QtGui/qguiapplication_platform.h>)
#include <QtGui/qguiapplication_platform.h>
#endif

#include <X11/Xatom.h> // keep the X11 headers at the bottom, they define
#include <X11/Xlib.h>  // None, Status and Bool as macros

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
  connect(m_restoreAction, &QAction::triggered, this,
          &MainWindow::restoreWindow);
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

// ── Window visibility ─────────────────────────────────────────────────────────

namespace {

// Whether the window manager currently keeps the window off screen.
//
// Qt derives Qt::WindowMinimized from WM_STATE only, but a window manager is
// free to minimize a window without ever setting WM_STATE to Iconic. KWin on
// X11 does exactly that: it sets _NET_WM_STATE_HIDDEN and leaves the window
// mapped with WM_STATE at Normal, so the application keeps reporting a visible,
// non-minimized, still exposed window that the user cannot see anywhere.
// Reading the property the window manager itself sets is the only way to tell.
bool windowIsHiddenByWM(WId window) {
#if defined(Q_OS_UNIX) && __has_include(<QtGui/qguiapplication_platform.h>)
  const auto *x11 =
      qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
  if (x11 == nullptr || window == 0)
    return false;

  Display *display = x11->display();
  const Atom netWmState = XInternAtom(display, "_NET_WM_STATE", True);
  const Atom hiddenState = XInternAtom(display, "_NET_WM_STATE_HIDDEN", True);
  if (netWmState == None || hiddenState == None)
    return false;

  Atom type = None;
  int format = 0;
  unsigned long stateCount = 0, bytesAfter = 0;
  unsigned char *data = nullptr;
  if (XGetWindowProperty(display, window, netWmState, 0, 32, False, XA_ATOM,
                         &type, &format, &stateCount, &bytesAfter,
                         &data) != Success ||
      data == nullptr)
    return false;

  const Atom *states = reinterpret_cast<Atom *>(data);
  bool hidden = false;
  for (unsigned long i = 0; i < stateCount && !hidden; ++i)
    hidden = states[i] == hiddenState;

  XFree(data);
  return hidden;
#else
  Q_UNUSED(window)
  return false;
#endif
}

} // namespace

// Whether the main window is actually on screen for the user.
//
// QWidget::isVisible() alone does not answer that: a window minimized by the
// window manager stays "visible" for Qt, and so does QWindow::isExposed() when
// the window manager keeps the window mapped while minimizing it. Ask the
// window manager on top of Qt's own state.
bool MainWindow::isWindowShown() const {
  if (isHidden() || isMinimized())
    return false;

  const QWindow *handle = windowHandle();
  if (handle == nullptr || !handle->isExposed())
    return false;

  return !windowIsHiddenByWM(winId());
}

void MainWindow::restoreWindow() {
  // A window the window manager minimized behind Qt's back is still "shown" as
  // far as Qt is concerned, which makes show() a no-op and would leave the
  // window off screen. Re-mapping it is the one way back that works on every
  // window manager.
  if (!isWindowShown() && isVisible())
    hide();

  setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
  show();
  raise();
  activateWindow();
  if (QWindow *handle = windowHandle())
    handle->requestActivate();
}

void MainWindow::checkWindowState() {
  const bool shown = isWindowShown();
  m_minimizeAction->setDisabled(!shown);
  m_restoreAction->setDisabled(shown);
  m_lockAction->setDisabled(m_lockWidget && m_lockWidget->getIsLocked());
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason != QSystemTrayIcon::Trigger &&
      reason != QSystemTrayIcon::DoubleClick)
    return;

  // Hiding the window on a tray click stays opt-in, restoring it never is:
  // clicking the tray icon of a window the user cannot see has to bring that
  // window back, the way every other tray application behaves.
  const bool minimizeOnClick = SettingsManager::instance()
                                   .settings()
                                   .value("minimizeOnTrayIconClick", false)
                                   .toBool();
  if (minimizeOnClick && isWindowShown()) {
    hide();
    return;
  }
  restoreWindow();
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
