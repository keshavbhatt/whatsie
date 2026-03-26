// Core MainWindow: constructor, lifecycle events, settings UI, notifications,
// zoom, and navigation helpers.
// WebEngine, tray, and lock logic live in mainwindow_webengine/tray/lock.cpp.
#include "mainwindow.h"

#include <QInputDialog>
#include <QRegularExpression>
#include <QScreen>
#include <QStyleFactory>
#include <QUrlQuery>

#include "about.h"
#include "common.h"
#include "rateapp.h"
#include "theme.h"
#include "webengineprofilemanager.h"

extern double defaultZoomFactorMaximized;
extern int    defaultAppAutoLockDuration;
extern bool   defaultAppAutoLock;

// ── Constructor / destructor ──────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_trayIconNormal(":/icons/app/notification/whatsie-notify.png"),
      m_notificationsTitleRegExp("^\\([1-9]\\d*\\).*"),
      m_unreadMessageCountRegExp("\\([^\\d]*(\\d+)[^\\d]*\\)") {

  setObjectName("MainWindow");
  setWindowTitle(QApplication::applicationName());
  setWindowIcon(QIcon(":/icons/app/icon-64.png"));
  setMinimumWidth(525);
  setMinimumHeight(448);
  restoreMainWindow();
  createActions();
  createTrayIcon();
  createWebEngine();
  initSettingWidget();
  initRateWidget();
  QApplication::processEvents();
  tryLock();
  updateWindowTheme();
  initAutoLock();
}

MainWindow::~MainWindow() { m_webEngine->deleteLater(); }

// ── Window geometry ───────────────────────────────────────────────────────────

void MainWindow::restoreMainWindow() {
  if (SettingsManager::instance().settings().value("geometry").isValid()) {
    restoreGeometry(
        SettingsManager::instance().settings().value("geometry").toByteArray());
    QPoint pos = QCursor::pos();
    for (auto screen : QGuiApplication::screens()) {
      QRect screenRect = screen->geometry();
      if (screenRect.contains(pos)) {
        move(screenRect.center() - rect().center());
      }
    }
  } else {
    resize(800, 684);
  }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  if (!m_lockWidget || event->size() == event->oldSize())
    return;
  m_lockWidget->resize(size());
}

// ── Window state & zoom ───────────────────────────────────────────────────────

void MainWindow::changeEvent(QEvent *e) {
  if (e->type() == QEvent::WindowStateChange)
    handleZoomOnWindowStateChange(static_cast<QWindowStateChangeEvent *>(e));
  QMainWindow::changeEvent(e);
}

void MainWindow::handleZoomOnWindowStateChange(
    const QWindowStateChangeEvent *ev) {
  if (m_settingsWidget == nullptr)
    return;
  if (ev->oldState().testFlag(Qt::WindowMaximized) &&
      windowState().testFlag(Qt::WindowNoState)) {
    emit m_settingsWidget->zoomChanged();
  } else if ((!ev->oldState().testFlag(Qt::WindowMaximized) &&
              windowState().testFlag(Qt::WindowMaximized)) ||
             (!ev->oldState().testFlag(Qt::WindowMaximized) &&
              windowState().testFlag(Qt::WindowFullScreen))) {
    emit m_settingsWidget->zoomMaximizedChanged();
  }
}

void MainWindow::handleZoom() {
  if (windowState().testFlag(Qt::WindowMaximized) ||
      windowState().testFlag(Qt::WindowFullScreen)) {
    double currentFactor =
        SettingsManager::instance()
            .settings()
            .value("zoomFactorMaximized", defaultZoomFactorMaximized)
            .toDouble();
    m_webEngine->page()->setZoomFactor(currentFactor);
  } else if (windowState().testFlag(Qt::WindowNoState)) {
    double currentFactor = SettingsManager::instance()
                               .settings()
                               .value("zoomFactor", 1.0)
                               .toDouble();
    m_webEngine->page()->setZoomFactor(currentFactor);
  }
}

// ── Theme ─────────────────────────────────────────────────────────────────────

void MainWindow::updateWindowTheme() {
  qApp->setStyle(QStyleFactory::create(SettingsManager::instance()
                                           .settings()
                                           .value("widgetStyle", "Fusion")
                                           .toString()));
  const bool dark = SettingsManager::instance()
                        .settings()
                        .value("windowTheme", "light")
                        .toString() == "dark";
  if (dark) {
    qApp->setPalette(Theme::getDarkPalette());
    m_webEngine->setStyleSheet("QWebEngineView{background:rgb(17, 27, 33);}");
  } else {
    qApp->setPalette(Theme::getLightPalette());
    m_webEngine->setStyleSheet("QWebEngineView{background:#F0F0F0;}");
  }

  if (m_webEngine->page()) {
    m_webEngine->page()->setBackgroundColor(
        dark ? QColor(17, 27, 33) : QColor(240, 240, 240));
  }

  for (QWidget *w : findChildren<QWidget *>())
    w->setPalette(qApp->palette());

  setNotificationPresenter(m_webEngine->page()->profile());

  if (m_lockWidget != nullptr) {
    m_lockWidget->setStyleSheet(
        "QWidget#login{background-color:palette(window)};"
        "QWidget#signup{background-color:palette(window)};");
    m_lockWidget->applyThemeQuirks();
  }
  update();
}

// ── Settings widget ───────────────────────────────────────────────────────────

void MainWindow::initSettingWidget() {
  int screenNumber = qApp->screens().indexOf(screen());
  if (m_settingsWidget != nullptr)
    return;

  m_settingsWidget = new SettingsWidget(
      this, screenNumber, m_webEngine->page()->profile()->cachePath(),
      m_webEngine->page()->profile()->persistentStoragePath());
  m_settingsWidget->setWindowTitle(QApplication::applicationName() +
                                   " | Settings");
  m_settingsWidget->setWindowFlags(Qt::Dialog);

  connect(m_settingsWidget, &SettingsWidget::initLock, this,
          &MainWindow::initLock);
  connect(m_settingsWidget, &SettingsWidget::changeLockPassword, this,
          &MainWindow::changeLockPassword);
  connect(m_settingsWidget, &SettingsWidget::appAutoLockChanged, this,
          &MainWindow::appAutoLockChanged);
  connect(m_settingsWidget, &SettingsWidget::updateWindowTheme, this,
          &MainWindow::updateWindowTheme);
  connect(m_settingsWidget, &SettingsWidget::updatePageTheme, this,
          &MainWindow::updatePageTheme);
  connect(m_settingsWidget, &SettingsWidget::muteToggled, this,
          &MainWindow::toggleMute);

  connect(m_settingsWidget, &SettingsWidget::userAgentChanged,
          m_settingsWidget, [=](QString userAgentStr) {
            if (m_webEngine->page()->profile()->httpUserAgent() !=
                userAgentStr) {
              SettingsManager::instance().settings().setValue("useragent",
                                                              userAgentStr);
              updateSettingsUserAgentWidget();
              askToReloadPage();
            }
          });

  connect(m_settingsWidget, &SettingsWidget::autoPlayMediaToggled,
          m_settingsWidget, [=](bool checked) {
            WebEngineProfileManager::instance().profile()->settings()
                ->setAttribute(
                    QWebEngineSettings::PlaybackRequiresUserGesture, checked);
          });

  connect(m_settingsWidget, &SettingsWidget::dictChanged, m_settingsWidget,
          [=](QString dictName) {
            if (m_webEngine && m_webEngine->page())
              m_webEngine->page()->profile()->setSpellCheckLanguages(
                  QStringList() << dictName);
          });

  connect(m_settingsWidget, &SettingsWidget::spellCheckChanged,
          m_settingsWidget, [=](bool checked) {
            if (m_webEngine && m_webEngine->page())
              m_webEngine->page()->profile()->setSpellCheckEnabled(checked);
          });

  connect(m_settingsWidget, &SettingsWidget::zoomChanged, m_settingsWidget,
          [=]() {
            if (windowState() == Qt::WindowNoState ||
                !(windowState() & Qt::WindowMaximized)) {
              double currentFactor = SettingsManager::instance()
                                         .settings()
                                         .value("zoomFactor", 1.0)
                                         .toDouble();
              m_webEngine->page()->setZoomFactor(currentFactor);
            }
          });

  connect(m_settingsWidget, &SettingsWidget::zoomMaximizedChanged,
          m_settingsWidget, [=]() {
            if (windowState() & Qt::WindowMaximized ||
                windowState() & Qt::WindowFullScreen) {
              double currentFactor =
                  SettingsManager::instance()
                      .settings()
                      .value("zoomFactorMaximized", defaultZoomFactorMaximized)
                      .toDouble();
              m_webEngine->page()->setZoomFactor(currentFactor);
            }
          });

  connect(m_settingsWidget, &SettingsWidget::notificationPopupTimeOutChanged,
          m_settingsWidget, [=]() {
            setNotificationPresenter(m_webEngine->page()->profile());
          });

  connect(m_settingsWidget, &SettingsWidget::notify, m_settingsWidget,
          [=](QString message) { showNotification("", message); });

  connect(m_settingsWidget, &SettingsWidget::updateFullWidthView,
          m_settingsWidget, [=](bool checked) {
            if (m_webEngine && m_webEngine->page()) {
              WebEnginePage *wp =
                  qobject_cast<WebEnginePage *>(m_webEngine->page());
              wp->injectClassChangeObserver();
              if (checked)
                m_webEngine->page()->runJavaScript(
                    "document.querySelector('body').classList.add('whatsie-"
                    "full-view');");
              else
                m_webEngine->page()->runJavaScript(
                    "document.querySelector('body').classList.remove('"
                    "whatsie-full-view');");
            }
          });

  m_settingsWidget->appLockSetChecked(SettingsManager::instance()
                                          .settings()
                                          .value("lockscreen", false)
                                          .toBool());
  m_settingsWidget->loadDictionaries(m_dictionaries);
}

void MainWindow::showSettings(bool isAskedByCLI) {
  if (m_lockWidget && m_lockWidget->getIsLocked()) {
    QString error = tr("Unlock to access Settings.");
    if (isAskedByCLI)
      showNotification(QApplication::applicationName() + "| Error", error);
    else
      QMessageBox::critical(this, QApplication::applicationName() + "| Error",
                            error);
    show();
    return;
  }
  if (m_webEngine == nullptr) {
    QMessageBox::critical(
        this, QApplication::applicationName() + "| Error",
        "Unable to initialize settings module.\nWebengine is not initialized.");
    return;
  }
  if (!m_settingsWidget->isVisible()) {
    updateSettingsUserAgentWidget();
    m_settingsWidget->refresh();
    QRect screenRect = screen()->geometry();
    if (!screenRect.contains(m_settingsWidget->pos()))
      m_settingsWidget->move(screenRect.center() -
                             m_settingsWidget->rect().center());
    m_settingsWidget->show();
  }
}

void MainWindow::updateSettingsUserAgentWidget() {
  m_settingsWidget->updateDefaultUAButton(
      m_webEngine->page()->profile()->httpUserAgent());
}

void MainWindow::askToReloadPage() {
  QMessageBox msgBox;
  msgBox.setWindowTitle(QApplication::applicationName() + " | Action required");
  msgBox.setInformativeText("Page needs to be reloaded to continue.");
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.exec();
  doAppReload();
}

// ── Notifications ─────────────────────────────────────────────────────────────

void MainWindow::showNotification(QString title, QString message) {
  if (SettingsManager::instance()
          .settings()
          .value("disableNotificationPopups", false)
          .toBool())
    return;

  if (title.isEmpty())
    title = QApplication::applicationName();

  if (SettingsManager::instance()
              .settings()
              .value("notificationCombo", 1)
              .toInt() == 0 &&
      m_systemTrayIcon != nullptr) {
    auto timeout = SettingsManager::instance()
                       .settings()
                       .value("notificationTimeOut", 9000)
                       .toInt();
    if (userDesktopEnvironment.contains("gnome", Qt::CaseInsensitive)) {
      m_systemTrayIcon->showMessage(title, message, QSystemTrayIcon::Critical,
                                    0);
    } else {
      m_systemTrayIcon->showMessage(
          title, message, QIcon(":/icons/app/notification/whatsie-notify.png"),
          timeout);
    }
  } else {
    auto popup = new NotificationPopup(m_webEngine);
    connect(popup, &NotificationPopup::notification_clicked, this,
            [=]() { notificationClicked(); });
    popup->style()->polish(qApp);
    popup->setMinimumWidth(300);
    popup->adjustSize();
    QScreen *scr = QGuiApplication::primaryScreen();
    if (scr) {
      popup->present(scr, title, message,
                     QPixmap(":/icons/app/notification/whatsie-notify.png"));
    } else {
      qWarning() << "showNotification: unable to get primary screen";
    }
  }
}

void MainWindow::notificationClicked() {
  show();
  QCoreApplication::processEvents();
  if (windowState().testFlag(Qt::WindowMinimized))
    setWindowState(windowState() & ~Qt::WindowMinimized);
  raise();
  activateWindow();
}

// ── Lifecycle events ──────────────────────────────────────────────────────────

void MainWindow::closeEvent(QCloseEvent *event) {
  SettingsManager::instance().settings().setValue("geometry", saveGeometry());
  getPageTheme();
  QTimer::singleShot(500, m_settingsWidget,
                     [=]() { m_settingsWidget->refresh(); });

  if (QSystemTrayIcon::isSystemTrayAvailable() &&
      SettingsManager::instance()
              .settings()
              .value("closeButtonActionCombo", 0)
              .toInt() == 0) {
    hide();
    event->ignore();
    if (SettingsManager::instance()
            .settings()
            .value("firstrun_tray", true)
            .toBool()) {
      showNotification(QApplication::applicationName(),
                       "Minimized to system tray.");
      SettingsManager::instance().settings().setValue("firstrun_tray", false);
    }
    return;
  }
  event->accept();
  quitApp();
  QMainWindow::closeEvent(event);
}

void MainWindow::quitApp() {
  SettingsManager::instance().settings().setValue("geometry", saveGeometry());
  getPageTheme();
  QTimer::singleShot(500, this, [=]() {
    SettingsManager::instance().settings().setValue("firstrun_tray", true);
    qApp->quit();
  });
}

void MainWindow::runMinimized() {
  m_minimizeAction->trigger();
  showNotification("Whatsie",
                   "Whatsie started minimized in system tray. Click to Open.");
}

void MainWindow::alreadyRunning(bool notify) {
  if (notify)
    showNotification(QApplication::applicationName(),
                     "Restored an already running instance.");
  setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
  show();
}

// ── Dialogs ───────────────────────────────────────────────────────────────────

void MainWindow::showAbout() {
  About *about = new About(this);
  about->setWindowFlag(Qt::Dialog);
  about->setMinimumSize(about->sizeHint());
  about->adjustSize();
  about->setAttribute(Qt::WA_DeleteOnClose, true);
  about->show();
}

void MainWindow::toggleTheme() {
  if (m_settingsWidget != nullptr)
    m_settingsWidget->toggleTheme();
}

// ── Chat / URL helpers ────────────────────────────────────────────────────────

void MainWindow::loadSchemaUrl(const QString &arg) {
  if (arg.contains("send?") || arg.contains("send/?")) {
    QString newArg = arg;
    newArg = newArg.replace("?", "&");
    QUrlQuery query(newArg);
    triggerNewChat(query.queryItemValue("phone"),
                   query.queryItemValue("text"));
  }
}

void MainWindow::newChat() {
  bool ok;
  QString phoneNumber = QInputDialog::getText(
      this, tr("New Chat"),
      tr("Enter a valid WhatsApp number with country code (ex- +91XXXXXXXXXX)"),
      QLineEdit::Normal, "", &ok);
  if (ok)
    triggerNewChat(phoneNumber, "");
}

void MainWindow::triggerNewChat(const QString &phone, const QString &text) {
  static QString phoneStr, textStr;
  m_webEngine->page()->runJavaScript(
      "openNewChatWhatsieDefined()",
      [this, phone, text](const QVariant &result) {
        if (result.toString().contains("true")) {
          m_webEngine->page()->runJavaScript(
              QString("openNewChatWhatsie(\"%1\",\"%2\")").arg(phone, text));
        } else {
          phoneStr = phone.isEmpty() ? "" : "phone=" + phone;
          textStr = text.isEmpty() ? "" : "text=" + text;
          m_webEngine->page()->load(
              QUrl("https://web.whatsapp.com/send?" + phoneStr + "&" +
                   textStr));
        }
        alreadyRunning();
      });
}

// ── Rate widget ───────────────────────────────────────────────────────────────

void MainWindow::initRateWidget() {
  RateApp *rateApp = new RateApp(this, "snap://whatsie", 5, 5, 1000 * 30);
  rateApp->setWindowTitle(QApplication::applicationName() + " | " +
                          tr("Rate Application"));
  rateApp->setVisible(false);
  rateApp->setWindowFlags(Qt::Dialog);
  rateApp->setAttribute(Qt::WA_DeleteOnClose, true);
  QPoint centerPos = geometry().center() - rateApp->geometry().center();
  connect(rateApp, &RateApp::showRateDialog, rateApp, [=]() {
    if (windowState() != Qt::WindowMinimized && isVisible() &&
        isActiveWindow()) {
      rateApp->move(centerPos);
      rateApp->show();
    } else {
      rateApp->delayShowEvent();
    }
  });
}
