#include "mainwindow.h"

#include <QInputDialog>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QShortcut>
#include <QStyleHints>
#include <QUrlQuery>
#include <QWebEngineNotification>

extern QString defaultUserAgentStr;
extern double defaultZoomFactorMaximized;
extern int defaultAppAutoLockDuration;
extern bool defaultAppAutoLock;

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
  tryLock();
  updateWindowTheme();
  initAutoLock();
}

void MainWindow::restoreMainWindow() {
  if (SettingsManager::instance().settings().value("geometry").isValid()) {
    restoreGeometry(
        SettingsManager::instance().settings().value("geometry").toByteArray());
    QPoint pos = QCursor::pos();
    auto localScreens = QGuiApplication::screens();
    for (auto screen : qAsConst(localScreens)) {
      QRect screenRect = screen->geometry();
      if (screenRect.contains(pos)) {
        this->move(screenRect.center() - this->rect().center());
      }
    }
  } else {
    this->resize(800, 684);
  }
}

void MainWindow::initAutoLock() {
  m_autoLockEventFilter = new AutoLockEventFilter(
      SettingsManager::instance()
          .settings()
          .value("autoLockDuration", defaultAppAutoLockDuration)
          .toInt() *
      1000);
  connect(m_autoLockEventFilter, &AutoLockEventFilter::autoLockTimerTimeout,
          this, [=]() {
            if ((m_settingsWidget && !m_settingsWidget->isVisible()) &&
                SettingsManager::instance()
                    .settings()
                    .value("appAutoLocking", defaultAppAutoLock)
                    .toBool()) {
              this->lockApp();
            }
          });
  if (SettingsManager::instance()
          .settings()
          .value("appAutoLocking", defaultAppAutoLock)
          .toBool()) {
    qApp->installEventFilter(m_autoLockEventFilter);
  }
}

void MainWindow::initRateWidget() {
  RateApp *rateApp = new RateApp(this, "snap://whatsie", 5, 5, 1000 * 30);
  rateApp->setWindowTitle(QApplication::applicationName() + " | " +
                          tr("Rate Application"));
  rateApp->setVisible(false);
  rateApp->setWindowFlags(Qt::Dialog);
  rateApp->setAttribute(Qt::WA_DeleteOnClose, true);
  QPoint centerPos = this->geometry().center() - rateApp->geometry().center();
  connect(rateApp, &RateApp::showRateDialog, rateApp, [=]() {
    if (this->windowState() != Qt::WindowMinimized && this->isVisible() &&
        isActiveWindow()) {
      rateApp->move(centerPos);
      rateApp->show();
    } else {
      rateApp->delayShowEvent();
    }
  });
}

void MainWindow::runMinimized() {
  this->m_minimizeAction->trigger();
  notify("Whatsie", "Whatsie started minimized in tray. Click to Open.");
}

MainWindow::~MainWindow() { m_webEngine->deleteLater(); }

void MainWindow::loadSchemaUrl(const QString &arg) {
  // https://faq.whatsapp.com/iphone/how-to-link-to-whatsapp-from-a-different-app/?lang=en

  // PASSED SCHEME whatsapp://send?text=Hello%2C%20World!&phone=919568388397"
  // CONVERTED URI
  // https://web.whatsapp.com/send?phone=919568388397&text=Hello%2C%20World New
  // chat composer
  if (arg.contains("send?") || arg.contains("send/?")) {
    QString newArg = arg;
    newArg = newArg.replace("?", "&");
    QUrlQuery query(newArg);

    QString phone, text;
    phone = query.queryItemValue("phone");
    text = query.queryItemValue("text");
    triggerNewChat(phone, text);
  }
}

void MainWindow::updatePageTheme() {
  if (m_webEngine && m_webEngine->page()) {

    QString windowTheme = SettingsManager::instance()
                              .settings()
                              .value("windowTheme", "light")
                              .toString();

    if (windowTheme == "dark") {
      m_webEngine->page()->runJavaScript(
          "localStorage['system-theme-mode']='false'; "
          "localStorage.theme='\"dark\"'; ");

      m_webEngine->page()->runJavaScript(
          "document.querySelector('body').classList.add('" + windowTheme +
          "');");
    } else {
      m_webEngine->page()->runJavaScript(
          "localStorage['system-theme-mode']='false'; "
          "localStorage.theme='\"light\"'; ");

      m_webEngine->page()->runJavaScript(
          "document.querySelector('body').classList.remove('dark');");
    }
  }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  if (m_lockWidget != nullptr) {
    m_lockWidget->resize(event->size());
  }
}

void MainWindow::updateWindowTheme() {
  qApp->setStyle(QStyleFactory::create(SettingsManager::instance()
                                           .settings()
                                           .value("widgetStyle", "Fusion")
                                           .toString()));
  if (SettingsManager::instance()
          .settings()
          .value("windowTheme", "light")
          .toString() == "dark") {
    qApp->setPalette(Theme::getDarkPalette());
    m_webEngine->setStyleSheet(
        "QWebEngineView{background:rgb(17, 27, 33);}"); // whatsapp dark color
  } else {
    qApp->setPalette(Theme::getLightPalette());
    m_webEngine->setStyleSheet(
        "QWebEngineView{background:#F0F0F0;}"); // whatsapp light color
  }

  QList<QWidget *> widgets = this->findChildren<QWidget *>();
  foreach (QWidget *w, widgets) {
    w->setPalette(qApp->palette());
  }
  setNotificationPresenter(m_webEngine->page()->profile());

  if (m_lockWidget != nullptr) {
    m_lockWidget->setStyleSheet(
        "QWidget#login{background-color:palette(window)};"
        "QWidget#signup{background-color:palette(window)};");
    m_lockWidget->applyThemeQuirks();
  }
  this->update();
}

void MainWindow::handleCookieAdded(const QNetworkCookie &cookie) {
  qDebug() << cookie.toRawForm() << "\n\n\n";
}

void MainWindow::forceLogOut() {
  if (m_webEngine && m_webEngine->page()) {
    m_webEngine->page()->runJavaScript(
        "window.localStorage.clear();",
        [=](const QVariant &result) { qDebug() << result; });
  }
}

bool MainWindow::isLoggedIn() {
  static bool loggedIn = false;
  if (m_webEngine && m_webEngine->page()) {
    m_webEngine->page()->runJavaScript(
        "window.localStorage.getItem('last-wid-md')",
        [=](const QVariant &result) {
          qDebug() << Q_FUNC_INFO << result;
          if (result.isValid() && result.toString().isEmpty() == false) {
            loggedIn = true;
          }
        });
    qDebug() << "isLoggedIn" << loggedIn;
    return loggedIn;
  } else {
    qDebug() << "isLoggedIn" << loggedIn;
    return loggedIn;
  }
}

void MainWindow::tryLogOut() {
  if (m_webEngine && m_webEngine->page()) {
    m_webEngine->page()->runJavaScript(
        "document.querySelector(\"span[data-testid|='menu']\").click();"
        "document.querySelector(\"#side > header > div > div > span > div > "
        "span > div > ul > li:nth-child(4) > div\").click();"
        "var dialogEle,dialogEleLastElem;"
        "function logoutC(){"
        "  dialogEle=document.activeElement.querySelectorAll(\":last-child\");"
        "  dialogEleLastElem=dialogEle[dialogEle.length-1];"
        "  dialogEleLastElem.click();"
        "}"
        "setTimeout(logoutC, 600);"
        "",
        [=](const QVariant &result) { qDebug() << Q_FUNC_INFO << result; });
  }
}

void MainWindow::initSettingWidget() {
  int screenNumber = qApp->desktop()->screenNumber(this);
  if (m_settingsWidget == nullptr) {
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
                this->updateSettingsUserAgentWidget();
                this->askToReloadPage();
              }
            });
    connect(m_settingsWidget, &SettingsWidget::autoPlayMediaToggled,
            m_settingsWidget, [=](bool checked) {
              QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
              auto *webSettings = profile->settings();
              webSettings->setAttribute(
                  QWebEngineSettings::PlaybackRequiresUserGesture, checked);

              m_webEngine->page()->profile()->settings()->setAttribute(
                  QWebEngineSettings::PlaybackRequiresUserGesture, checked);
            });

    connect(m_settingsWidget, &SettingsWidget::dictChanged, m_settingsWidget,
            [=](QString dictName) {
              if (m_webEngine && m_webEngine->page()) {
                m_webEngine->page()->profile()->setSpellCheckLanguages(
                    QStringList() << dictName);
              }
            });

    connect(m_settingsWidget, &SettingsWidget::spellCheckChanged,
            m_settingsWidget, [=](bool checked) {
              if (m_webEngine && m_webEngine->page()) {
                m_webEngine->page()->profile()->setSpellCheckEnabled(checked);
              }
            });

    connect(m_settingsWidget, &SettingsWidget::zoomChanged, m_settingsWidget,
            [=]() {
              if (windowState() == Qt::WindowNoState) {
                double currentFactor = SettingsManager::instance()
                                           .settings()
                                           .value("zoomFactor", 1.0)
                                           .toDouble();
                m_webEngine->page()->setZoomFactor(currentFactor);
              }
            });

    connect(m_settingsWidget, &SettingsWidget::zoomMaximizedChanged,
            m_settingsWidget, [=]() {
              if (windowState() == Qt::WindowMaximized ||
                  windowState() == Qt::WindowFullScreen) {
                double currentFactor = SettingsManager::instance()
                                           .settings()
                                           .value("zoomFactorMaximized",
                                                  defaultZoomFactorMaximized)
                                           .toDouble();
                m_webEngine->page()->setZoomFactor(currentFactor);
              }
            });

    connect(m_settingsWidget, &SettingsWidget::notificationPopupTimeOutChanged,
            m_settingsWidget, [=]() {
              setNotificationPresenter(m_webEngine->page()->profile());
            });

    connect(m_settingsWidget, &SettingsWidget::notify, m_settingsWidget,
            [=](QString message) { notify("", message); });

    connect(m_settingsWidget, &SettingsWidget::updateFullWidthView,
            m_settingsWidget, [=](bool checked) {
              if (m_webEngine && m_webEngine->page()) {
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
}

void MainWindow::changeEvent(QEvent *e) {
  if (e->type() == QEvent::WindowStateChange) {
    handleZoomOnWindowStateChange(static_cast<QWindowStateChangeEvent *>(e));
  }
  QMainWindow::changeEvent(e);
}

void MainWindow::handleZoomOnWindowStateChange(
    const QWindowStateChangeEvent *ev) {
  if (m_settingsWidget != nullptr) {
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

void MainWindow::lockApp() {
  if (m_lockWidget != nullptr && m_lockWidget->getIsLocked())
    return;

  if (SettingsManager::instance().settings().value("asdfg").isValid()) {
    initLock();
    m_lockWidget->lock_app();
  } else {
    int ret = QMessageBox::information(
        this, tr(QApplication::applicationName().toUtf8()),
        tr("App lock is not configured, \n"
           "Please setup the password in the Settings first.\n\nOpen "
           "Settings now?"),
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret == QMessageBox::Yes) {
      this->showSettings();
    }
  }
}

void MainWindow::toggleTheme() {
  if (m_settingsWidget != nullptr) {
    m_settingsWidget->toggleTheme();
  }
}

void MainWindow::showSettings(bool isAskedByCLI) {
  if (m_lockWidget && m_lockWidget->getIsLocked()) {
    QString error = tr("Unlock to access Settings.");
    if (isAskedByCLI) {
      this->notify(QApplication::applicationName() + "| Error", error);
    } else {
      QMessageBox::critical(this, QApplication::applicationName() + "| Error",
                            error);
    }
    this->show();
    return;
  }

  if (m_webEngine == nullptr) {
    QMessageBox::critical(
        this, QApplication::applicationName() + "| Error",
        "Unable to initialize settings module.\nWebengine is not initialized.");
    return;
  }
  if (!m_settingsWidget->isVisible()) {
    this->updateSettingsUserAgentWidget();
    m_settingsWidget->refresh();
    int screenNumber = qApp->desktop()->screenNumber(this);
    QRect screenRect = QGuiApplication::screens().at(screenNumber)->geometry();
    if (!screenRect.contains(m_settingsWidget->pos())) {
      m_settingsWidget->move(screenRect.center() -
                             m_settingsWidget->rect().center());
    }
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
  this->doAppReload();
}

void MainWindow::showAbout() {
  About *about = new About(this);
  about->setWindowFlag(Qt::Dialog);
  about->setMinimumSize(about->sizeHint());
  about->adjustSize();
  about->setAttribute(Qt::WA_DeleteOnClose, true);
  about->show();
}

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
    this->hide();
    event->ignore();
    if (SettingsManager::instance()
            .settings()
            .value("firstrun_tray", true)
            .toBool()) {
      notify(QApplication::applicationName(), "Minimized to system tray.");
      SettingsManager::instance().settings().setValue("firstrun_tray", false);
    }
    return;
  }
  event->accept();
  quitApp();
  QMainWindow::closeEvent(event);
}

void MainWindow::notify(QString title, QString message) {

  if (SettingsManager::instance()
          .settings()
          .value("disableNotificationPopups", false)
          .toBool() == true) {
    return;
  }

  if (title.isEmpty())
    title = QApplication::applicationName();

  if (SettingsManager::instance()
              .settings()
              .value("notificationCombo", 1)
              .toInt() == 0 &&
      m_systemTrayIcon != nullptr) {
    m_systemTrayIcon->showMessage(
        title, message, QIcon(":/icons/app/notification/whatsie-notify.png"),
        SettingsManager::instance()
            .settings()
            .value("notificationTimeOut", 9000)
            .toInt());
    m_systemTrayIcon->disconnect(m_systemTrayIcon, SIGNAL(messageClicked()));
    connect(m_systemTrayIcon, &QSystemTrayIcon::messageClicked,
            m_systemTrayIcon, [=]() {
              if (windowState().testFlag(Qt::WindowMinimized) ||
                  !windowState().testFlag(Qt::WindowActive)) {
                activateWindow();
                this->show();
              }
            });
  } else {
    auto popup = new NotificationPopup(m_webEngine);
    connect(popup, &NotificationPopup::notification_clicked, popup, [=]() {
      if (windowState().testFlag(Qt::WindowMinimized) ||
          !windowState().testFlag(Qt::WindowActive) || this->isHidden()) {
        this->show();
        setWindowState((windowState() & ~Qt::WindowMinimized) |
                       Qt::WindowActive);
      }
    });
    popup->style()->polish(qApp);
    popup->setMinimumWidth(300);
    popup->adjustSize();
    int screenNumber = qApp->desktop()->screenNumber(this);
    popup->present(screenNumber < 0 ? 0 : screenNumber, title, message,
                   QPixmap(":/icons/app/notification/whatsie-notify.png"));
  }
}

void MainWindow::createActions() {

  m_openUrlAction = new QAction("New Chat", this);
  m_openUrlAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_N));
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
      QKeySequence(Qt::Modifier::CTRL + Qt::Key_W), this, SLOT(hide()));
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
  m_lockAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_L));
  connect(m_lockAction, &QAction::triggered, this, &MainWindow::lockApp);
  addAction(m_lockAction);

  m_settingsAction = new QAction(tr("&Settings"), this);
  m_settingsAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_P));
  connect(m_settingsAction, &QAction::triggered, this,
          &MainWindow::showSettings);
  addAction(m_settingsAction);

  m_toggleThemeAction = new QAction(tr("&Toggle theme"), this);
  m_toggleThemeAction->setShortcut(
      QKeySequence(Qt::Modifier::CTRL + Qt::Key_T));
  connect(m_toggleThemeAction, &QAction::triggered, this,
          &MainWindow::toggleTheme);
  addAction(m_toggleThemeAction);

  m_aboutAction = new QAction(tr("&About"), this);
  connect(m_aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

  m_quitAction = new QAction(tr("&Quit"), this);
  m_quitAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_Q));
  connect(m_quitAction, &QAction::triggered, this, &MainWindow::quitApp);
  addAction(m_quitAction);
}

void MainWindow::quitApp() {
  SettingsManager::instance().settings().setValue("geometry", saveGeometry());
  getPageTheme();
  QTimer::singleShot(500, this, [=]() {
    SettingsManager::instance().settings().setValue("firstrun_tray", true);
    qApp->quit();
  });
}

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

  m_systemTrayIcon->show();

  connect(m_systemTrayIcon, &QSystemTrayIcon::messageClicked, this,
          &MainWindow::messageClicked);
  connect(m_systemTrayIcon, &QSystemTrayIcon::activated, this,
          &MainWindow::iconActivated);

  // enable show shortcuts in menu
  if (qApp->styleHints()->showShortcutsInContextMenus()) {
    foreach (QAction *action, m_trayIconMenu->actions()) {
      action->setShortcutVisibleInContextMenu(true);
    }
  }
}

void MainWindow::initLock() {

  if (m_lockWidget == nullptr) {
    m_lockWidget = new Lock(this);
    m_lockWidget->setObjectName("lockWidget");

    m_lockWidget->setWindowFlags(Qt::Widget);
    m_lockWidget->setStyleSheet(
        "QWidget#login{background-color:palette(window)};"
        "QWidget#signup{background-color:palette(window)}");
    m_lockWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(m_lockWidget, &Lock::passwordNotSet, m_settingsWidget, [=]() {
      SettingsManager::instance().settings().setValue("lockscreen", false);
      m_settingsWidget->appLockSetChecked(false);
    });

    connect(m_lockWidget, &Lock::unLocked, [=]() {
      // unlock event
    });

    connect(m_lockWidget, &Lock::passwordSet, m_settingsWidget, [=]() {
      if (SettingsManager::instance().settings().value("asdfg").isValid()) {
        m_settingsWidget->setCurrentPasswordText(
            QByteArray::fromBase64(SettingsManager::instance()
                                       .settings()
                                       .value("asdfg")
                                       .toString()
                                       .toUtf8()));
      } else {
        m_settingsWidget->setCurrentPasswordText("Require setup");
      }
      m_settingsWidget->appLockSetChecked(SettingsManager::instance()
                                              .settings()
                                              .value("lockscreen", false)
                                              .toBool());
    });
    m_lockWidget->applyThemeQuirks();
  }

  m_lockWidget->setGeometry(this->rect());

  if (SettingsManager::instance().settings().value("lockscreen").toBool()) {
    if (SettingsManager::instance().settings().value("asdfg").isValid()) {
      m_lockWidget->lock_app();
    } else {
      m_lockWidget->signUp();
    }
    m_lockWidget->show();
  } else {
    m_lockWidget->hide();
  }
  updateWindowTheme();
}

void MainWindow::changeLockPassword() {
  SettingsManager::instance().settings().remove("asdfg");
  m_settingsWidget->appLockSetChecked(false);
  m_settingsWidget->autoAppLockSetChecked(false);
  m_settingsWidget->updateAppLockPasswordViewer();
  tryLogOut();
  QTimer::singleShot(1000, this, [=]() {
    if (isLoggedIn()) {
      forceLogOut();
      doAppReload();
    }
    appAutoLockChanged();
    initLock();
  });
}

void MainWindow::appAutoLockChanged() {
  bool enabled = SettingsManager::instance()
                     .settings()
                     .value("appAutoLocking", defaultAppAutoLock)
                     .toBool();
  if (enabled) {
    m_autoLockEventFilter->setTimeoutmillis(
        SettingsManager::instance()
            .settings()
            .value("autoLockDuration", defaultAppAutoLockDuration)
            .toInt() *
        1000);
    qApp->installEventFilter(m_autoLockEventFilter);
    m_autoLockEventFilter->resetTimer();
  } else {
    m_autoLockEventFilter->stopTimer();
    qApp->removeEventFilter(m_autoLockEventFilter);
  }
}

// check window state and set tray menus
void MainWindow::checkWindowState() {
  QObject *tray_icon_menu = this->findChild<QObject *>("trayIconMenu");
  if (tray_icon_menu != nullptr) {
    QMenu *menu = qobject_cast<QMenu *>(tray_icon_menu);
    if (this->isVisible()) {
      menu->actions().at(0)->setDisabled(false);
      menu->actions().at(1)->setDisabled(true);
    } else {
      menu->actions().at(0)->setDisabled(true);
      menu->actions().at(1)->setDisabled(false);
    }
    if (m_lockWidget && m_lockWidget->getIsLocked()) {
      menu->actions().at(4)->setDisabled(true);
    } else {
      menu->actions().at(4)->setDisabled(false);
    }
  }
}

void MainWindow::initGlobalWebProfile() {

  QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
  profile->setHttpUserAgent(SettingsManager::instance()
                                .settings()
                                .value("useragent", defaultUserAgentStr)
                                .toString());

  QStringList dict_names;
  dict_names.append(SettingsManager::instance()
                        .settings()
                        .value("sc_dict", "en-US")
                        .toString());

  profile->setSpellCheckEnabled(SettingsManager::instance()
                                    .settings()
                                    .value("sc_enabled", true)
                                    .toBool());
  profile->setSpellCheckLanguages(dict_names);

  auto *webSettings = profile->settings();
  webSettings->setAttribute(QWebEngineSettings::AutoLoadImages, true);
  webSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
  webSettings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
  webSettings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
  webSettings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls,
                            true);
  webSettings->setAttribute(QWebEngineSettings::XSSAuditingEnabled, true);
  webSettings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls,
                            true);
  webSettings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, false);
  webSettings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
  webSettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
  webSettings->setAttribute(QWebEngineSettings::LinksIncludedInFocusChain,
                            false);
  webSettings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled,
                            false);
  webSettings->setAttribute(QWebEngineSettings::SpatialNavigationEnabled, true);
  webSettings->setAttribute(QWebEngineSettings::JavascriptCanPaste, true);
  webSettings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard,
                            true);
  webSettings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture,
                            SettingsManager::instance()
                                .settings()
                                .value("autoPlayMedia", false)
                                .toBool());
}

void MainWindow::createWebEngine() {
  initGlobalWebProfile();

  QSizePolicy widgetSize;
  widgetSize.setHorizontalPolicy(QSizePolicy::Expanding);
  widgetSize.setVerticalPolicy(QSizePolicy::Expanding);
  widgetSize.setHorizontalStretch(1);
  widgetSize.setVerticalStretch(1);

  m_dictionaries = Dictionaries::GetDictionaries();

  WebView *webEngineView = new WebView(this, m_dictionaries);
  setCentralWidget(webEngineView);
  webEngineView->setSizePolicy(widgetSize);
  webEngineView->show();

  this->m_webEngine = webEngineView;

  webEngineView->addAction(m_minimizeAction);
  webEngineView->addAction(m_lockAction);
  webEngineView->addAction(m_quitAction);

  createWebPage(false);
}

const QIcon MainWindow::getTrayIcon(const int &notificationCount) const {
  if (notificationCount == 0) {
    return QIcon(":/icons/app/notification/whatsie-notify.png");
  } else if (notificationCount >= 10) {
    return QIcon(":/icons/app/notification/whatsie-notify-10.png");
  } else {
    return QIcon(":/icons/app/notification/whatsie-notify-" +
                 QString::number(notificationCount) + ".png");
  }
}

void MainWindow::createWebPage(bool offTheRecord) {
  if (offTheRecord && !m_otrProfile) {
    m_otrProfile.reset(new QWebEngineProfile);
  }
  auto profile =
      offTheRecord ? m_otrProfile.get() : QWebEngineProfile::defaultProfile();

  QStringList dict_names;
  dict_names.append(SettingsManager::instance()
                        .settings()
                        .value("sc_dict", "en-US")
                        .toString());

  profile->setSpellCheckEnabled(SettingsManager::instance()
                                    .settings()
                                    .value("sc_enabled", true)
                                    .toBool());
  profile->setSpellCheckLanguages(dict_names);
  profile->setHttpUserAgent(SettingsManager::instance()
                                .settings()
                                .value("useragent", defaultUserAgentStr)
                                .toString());

  setNotificationPresenter(profile);

  QWebEnginePage *page = new WebEnginePage(profile, m_webEngine);
  if (SettingsManager::instance()
          .settings()
          .value("windowTheme", "light")
          .toString() == "dark") {
    page->setBackgroundColor(QColor(17, 27, 33)); // whatsapp dark bg color
  } else {
    page->setBackgroundColor(QColor(240, 240, 240)); // whatsapp light bg color
  }
  m_webEngine->setPage(page);
  // page should be set parent of profile to prevent
  // Release of profile requested but WebEnginePage still not deleted. Expect
  // troubles !
  profile->setParent(page);
  auto randomValue = QRandomGenerator::global()->generateDouble() * 300.0;
  page->setUrl(
      QUrl("https://web.whatsapp.com?v=" + QString::number(randomValue)));

  connect(profile, &QWebEngineProfile::downloadRequested,
          &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested);

  connect(page, &QWebEnginePage::fullScreenRequested, this,
          &MainWindow::fullScreenRequested);

  double currentFactor = SettingsManager::instance()
                             .settings()
                             .value("zoomFactor", 1.0)
                             .toDouble();
  m_webEngine->page()->setZoomFactor(currentFactor);
}

void MainWindow::setNotificationPresenter(QWebEngineProfile *profile) {
  auto *op = m_webEngine->findChild<NotificationPopup *>("engineNotifier");
  if (op != nullptr) {
    op->close();
    op->deleteLater();
  }

  auto popup = new NotificationPopup(m_webEngine);
  popup->setObjectName("engineNotifier");
  connect(popup, &NotificationPopup::notification_clicked, popup, [=]() {
    if (windowState().testFlag(Qt::WindowMinimized) ||
        !windowState().testFlag(Qt::WindowActive) || this->isHidden()) {
      this->show();
      setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    }
  });

  profile->setNotificationPresenter(
      [=](std::unique_ptr<QWebEngineNotification> notification) {
        if (SettingsManager::instance()
                .settings()
                .value("disableNotificationPopups", false)
                .toBool() == true) {
          return;
        }
        if (SettingsManager::instance()
                    .settings()
                    .value("notificationCombo", 1)
                    .toInt() == 0 &&
            m_systemTrayIcon != nullptr) {
          QIcon icon(QPixmap::fromImage(notification->icon()));
          m_systemTrayIcon->showMessage(notification->title(),
                                        notification->message(), icon,
                                        SettingsManager::instance()
                                            .settings()
                                            .value("notificationTimeOut", 9000)
                                            .toInt());
          m_systemTrayIcon->disconnect(m_systemTrayIcon,
                                       SIGNAL(messageClicked()));
          connect(m_systemTrayIcon, &QSystemTrayIcon::messageClicked,
                  m_systemTrayIcon, [=]() {
                    if (windowState().testFlag(Qt::WindowMinimized) ||
                        !windowState().testFlag(Qt::WindowActive) ||
                        this->isHidden()) {
                      this->show();
                      setWindowState((windowState() & ~Qt::WindowMinimized) |
                                     Qt::WindowActive);
                    }
                  });

        } else {
          popup->setMinimumWidth(300);
          int screenNumber = qApp->desktop()->screenNumber(this);
          popup->present(screenNumber, notification);
        }
      });
}

void MainWindow::fullScreenRequested(QWebEngineFullScreenRequest request) {
  if (request.toggleOn()) {
    m_webEngine->showFullScreen();
    this->showFullScreen();
    request.accept();
  } else {
    m_webEngine->showNormal();
    this->show();
    request.accept();
  }
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

    setWindowIcon(m_trayIconNormal);
  }
}

void MainWindow::handleLoadFinished(bool loaded) {
  if (loaded) {
    qDebug() << "Loaded";
    checkLoadedCorrectly();
    updatePageTheme();
    handleZoom();
    if (m_settingsWidget != nullptr) {
      m_settingsWidget->refresh();
    }
  }
}

void MainWindow::checkLoadedCorrectly() {
  if (m_webEngine && m_webEngine->page()) {
    // test 1 based on the class name of body tag of the page
    m_webEngine->page()->runJavaScript(
        "document.querySelector('body').className",
        [this](const QVariant &result) {
          if (result.toString().contains("page-version", Qt::CaseInsensitive)) {
            qDebug() << "Test 1 found" << result.toString();
            m_webEngine->page()->runJavaScript(
                "document.getElementsByTagName('body')[0].innerText = ''");
            loadingQuirk("test1");
          } else if (m_webEngine->title().contains("Error",
                                                   Qt::CaseInsensitive)) {
            Utils::delete_cache(m_webEngine->page()->profile()->cachePath());
            Utils::delete_cache(
                m_webEngine->page()->profile()->persistentStoragePath());
            SettingsManager::instance().settings().setValue(
                "useragent", defaultUserAgentStr);
            Utils::DisplayExceptionErrorDialog(
                "test1 handleWebViewTitleChanged(title) title: Error, "
                "Resetting UA, Quiting!\nUA: " +
                SettingsManager::instance()
                    .settings()
                    .value("useragent", "DefaultUA")
                    .toString());
            m_quitAction->trigger();
          } else {
            qDebug() << "Test 1 loaded correctly, value:" << result.toString();
          }
        });
  }
}

void MainWindow::loadingQuirk(const QString &test) {
  // contains ug message apply quirk
  if (m_correctlyLoadedRetries > -1) {
    qWarning() << test << "checkLoadedCorrectly()/loadingQuirk()/doReload()"
               << m_correctlyLoadedRetries;
    doReload(false, false, true);
    m_correctlyLoadedRetries--;
  } else {
    Utils::delete_cache(m_webEngine->page()->profile()->cachePath());
    Utils::delete_cache(
        m_webEngine->page()->profile()->persistentStoragePath());
    SettingsManager::instance().settings().setValue("useragent",
                                                    defaultUserAgentStr);
    Utils::DisplayExceptionErrorDialog(
        test +
        " checkLoadedCorrectly()/loadingQuirk() reload retries 0, Resetting "
        "UA, Quiting!\nUA: " +
        SettingsManager::instance()
            .settings()
            .value("useragent", "DefaultUA")
            .toString());

    m_quitAction->trigger();
  }
}

// unused direct method to download file without having entry in download
// manager
void MainWindow::handleDownloadRequested(QWebEngineDownloadItem *download) {
  QFileDialog dialog(this);
  bool usenativeFileDialog = SettingsManager::instance()
                                 .settings()
                                 .value("useNativeFileDialog", false)
                                 .toBool();

  if (usenativeFileDialog == false) {
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
  }

  dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
  dialog.setFileMode(QFileDialog::FileMode::AnyFile);
  QString suggestedFileName = QUrl(download->downloadDirectory()).fileName();
  dialog.selectFile(suggestedFileName);

  if (dialog.exec() && dialog.selectedFiles().size() > 0) {
    download->setDownloadDirectory(dialog.selectedFiles().at(0));
    download->accept();
  }
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
    this->show();
  }
}

void MainWindow::messageClicked() {
  if (isVisible()) {
    hide();
  } else {
    this->show();
  }
}

void MainWindow::doAppReload() {

  if (m_webEngine->page()) {
    m_webEngine->page()->disconnect();
  }
  createWebPage(false);
}

void MainWindow::newChat() {
  bool ok;
  QString phoneNumber = QInputDialog::getText(
      this, tr("New Chat"),
      tr("Enter a valid WhatsApp number with country code (ex- +91XXXXXXXXXX)"),
      QLineEdit::Normal, "", &ok);
  if (ok) {
    triggerNewChat(phoneNumber, "");
  }
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
          // create send url equivalent
          phoneStr = phone.isEmpty() ? "" : "phone=" + phone;
          textStr = text.isEmpty() ? "" : "text=" + text;
          QString urlStr =
              "https://web.whatsapp.com/send?" + phoneStr + "&" + textStr;
          m_webEngine->page()->load(QUrl(urlStr));
        }
        this->alreadyRunning();
      });
}

void MainWindow::doReload(bool byPassCache, bool isAskedByCLI,
                          bool byLoadingQuirk) {
  if (byLoadingQuirk) {
    m_webEngine->triggerPageAction(QWebEnginePage::ReloadAndBypassCache,
                                   byPassCache);
  } else {
    if (m_lockWidget && !m_lockWidget->getIsLocked()) {
      this->notify(QApplication::applicationName(),
                   QObject::tr("Reloading..."));
    } else {
      QString error = tr("Unlock to Reload the App.");
      if (isAskedByCLI) {
        this->notify(QApplication::applicationName() + "| Error", error);
      } else {
        QMessageBox::critical(this, QApplication::applicationName() + "| Error",
                              error);
      }
      this->show();
      return;
    }
    m_webEngine->triggerPageAction(QWebEnginePage::ReloadAndBypassCache,
                                   byPassCache);
  }
}

void MainWindow::toggleMute(const bool &checked) {
  m_webEngine->page()->setAudioMuted(checked);
}

// get value of page theme when page is loaded
QString MainWindow::getPageTheme() const {
  static QString theme = "web"; // implies light
  if (m_webEngine && m_webEngine->page()) {
    m_webEngine->page()->runJavaScript(
        "document.querySelector('body').className;",
        [=](const QVariant &result) {
          theme = result.toString();
          theme.contains("dark") ? theme = "dark" : theme = "light";
          SettingsManager::instance().settings().setValue("windowTheme", theme);
        });
  }
  return theme;
}

void MainWindow::tryLock() {
  if (SettingsManager::instance().settings().value("asdfg").isValid()) {
    initLock();
    return;
  }
  if (SettingsManager::instance().settings().value("asdfg").isValid() ==
      false) {
    SettingsManager::instance().settings().setValue("lockscreen", false);
    SettingsManager::instance().settings().setValue("appAutoLocking", false);
    m_settingsWidget->appAutoLockingSetChecked(false);
    m_settingsWidget->appLockSetChecked(false);
    initLock();
  }
}

void MainWindow::alreadyRunning(bool notify) {
  if (notify) {
    QString appname = QApplication::applicationName();
    this->notify(appname, "Restored an already running instance.");
  }
  this->setWindowState((this->windowState() & ~Qt::WindowMinimized) |
                       Qt::WindowActive);
  this->show();
}
