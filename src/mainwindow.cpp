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
    : QMainWindow(parent), notificationsTitleRegExp("^\\([1-9]\\d*\\).*"),
      trayIconNormal(":/icons/app/notification/whatsie-notify.png"),
      unreadMessageCountRegExp("\\([^\\d]*(\\d+)[^\\d]*\\)") {

  setObjectName("MainWindow");
  setWindowTitle(QApplication::applicationName());
  setWindowIcon(QIcon(":/icons/app/icon-64.png"));
  setMinimumWidth(525);
  setMinimumHeight(448);
  restoreMainWindow();
  initThemes();
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
  if (settings.value("geometry").isValid()) {
    restoreGeometry(settings.value("geometry").toByteArray());
    QPoint pos = QCursor::pos();
    for (QScreen *screen : QGuiApplication::screens()) {
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
  autoLockEventFilter = new AutoLockEventFilter(
      settings.value("autoLockDuration", defaultAppAutoLockDuration).toInt() *
      1000);
  connect(autoLockEventFilter, &AutoLockEventFilter::autoLockTimerTimeout, this,
          [=]() {
            if ((settingsWidget && !settingsWidget->isVisible()) &&
                settings.value("appAutoLocking", defaultAppAutoLock).toBool()) {
              this->lockApp();
            }
          });
  if (settings.value("appAutoLocking", defaultAppAutoLock).toBool()) {
    qApp->installEventFilter(autoLockEventFilter);
  }
}

void MainWindow::initThemes() {
  // Light
  lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
  lightPalette.setColor(QPalette::WindowText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
  lightPalette.setColor(QPalette::Light, QColor(180, 180, 180));
  lightPalette.setColor(QPalette::Midlight, QColor(200, 200, 200));
  lightPalette.setColor(QPalette::Dark, QColor(225, 225, 225));
  lightPalette.setColor(QPalette::Text, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::BrightText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::ButtonText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::Base, QColor(237, 237, 237));
  lightPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
  lightPalette.setColor(QPalette::Highlight, QColor(76, 163, 224));
  lightPalette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::Link, QColor(0, 162, 232));
  lightPalette.setColor(QPalette::AlternateBase, QColor(225, 225, 225));
  lightPalette.setColor(QPalette::ToolTipBase, QColor(240, 240, 240));
  lightPalette.setColor(QPalette::ToolTipText, QColor(0, 0, 0));
  lightPalette.setColor(QPalette::LinkVisited, QColor(222, 222, 222));
  lightPalette.setColor(QPalette::Disabled, QPalette::WindowText,
                        QColor(115, 115, 115));
  lightPalette.setColor(QPalette::Disabled, QPalette::Text,
                        QColor(115, 115, 115));
  lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                        QColor(115, 115, 115));
  lightPalette.setColor(QPalette::Disabled, QPalette::Highlight,
                        QColor(190, 190, 190));
  lightPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                        QColor(115, 115, 115));

  // Dark
  darkPalette.setColor(QPalette::Window, QColor(17, 27, 33));
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(32, 44, 51));
  darkPalette.setColor(QPalette::AlternateBase, QColor(95, 108, 115));
  darkPalette.setColor(QPalette::ToolTipBase, QColor(66, 66, 66));
  darkPalette.setColor(QPalette::ToolTipText, QColor(192, 192, 192));
  darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
  darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
  darkPalette.setColor(QPalette::Button, QColor(17, 27, 33));
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::Highlight, QColor(38, 140, 196));
  darkPalette.setColor(QPalette::HighlightedText, Qt::white);
  darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText,
                       QColor(127, 127, 127));
  darkPalette.setColor(QPalette::Disabled, QPalette::Window,
                       QColor(65, 65, 67));
  darkPalette.setColor(QPalette::Disabled, QPalette::Highlight,
                       QColor(80, 80, 80));
  darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText,
                       QColor(127, 127, 127));
  darkPalette.setColor(QPalette::Disabled, QPalette::Text,
                       QColor(127, 127, 127));
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
  this->minimizeAction->trigger();
  notify("Whatsie", "Whatsie started minimized in tray. Click to Open.");
}

MainWindow::~MainWindow() { webEngine->deleteLater(); }

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
  if (webEngine && webEngine->page()) {

    QString windowTheme = settings.value("windowTheme", "light").toString();

    if (windowTheme == "dark") {
      webEngine->page()->runJavaScript(
          "localStorage['system-theme-mode']='false'; "
          "localStorage.theme='\"dark\"'; ");

      webEngine->page()->runJavaScript(
          "document.querySelector('body').classList.add('" + windowTheme +
          "');");
    } else {
      webEngine->page()->runJavaScript(
          "localStorage['system-theme-mode']='false'; "
          "localStorage.theme='\"light\"'; ");

      webEngine->page()->runJavaScript(
          "document.querySelector('body').classList.remove('dark');");
    }
  }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  if (lockWidget != nullptr) {
    lockWidget->resize(event->size());
  }
}

void MainWindow::updateWindowTheme() {
  qApp->setStyle(QStyleFactory::create(
      settings.value("widgetStyle", "Fusion").toString()));
  if (settings.value("windowTheme", "light").toString() == "dark") {
    qApp->setPalette(darkPalette);
    this->webEngine->setStyleSheet(
        "QWebEngineView{background:rgb(17, 27, 33);}"); // whatsapp dark color
  } else {
    qApp->setPalette(lightPalette);
    this->webEngine->setStyleSheet(
        "QWebEngineView{background:#F0F0F0;}"); // whatsapp light color
  }

  QList<QWidget *> widgets = this->findChildren<QWidget *>();
  foreach (QWidget *w, widgets) {
    w->setPalette(qApp->palette());
  }
  setNotificationPresenter(webEngine->page()->profile());

  if (lockWidget != nullptr) {
    lockWidget->setStyleSheet(
        "QWidget#login{background-color:palette(window)};"
        "QWidget#signup{background-color:palette(window)};");
    lockWidget->applyThemeQuirks();
  }
  this->update();
}

void MainWindow::handleCookieAdded(const QNetworkCookie &cookie) {
  qDebug() << cookie.toRawForm() << "\n\n\n";
}

void MainWindow::forceLogOut() {
  if (webEngine && webEngine->page()) {
    webEngine->page()->runJavaScript(
        "window.localStorage.clear();",
        [=](const QVariant &result) { qDebug() << result; });
  }
}

bool MainWindow::isLoggedIn() {
  static bool loggedIn = false;
  if (webEngine && webEngine->page()) {
    webEngine->page()->runJavaScript(
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
  if (webEngine && webEngine->page()) {
    webEngine->page()->runJavaScript(
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
  if (settingsWidget == nullptr) {
    settingsWidget = new SettingsWidget(
        this, screenNumber, webEngine->page()->profile()->cachePath(),
        webEngine->page()->profile()->persistentStoragePath());
    settingsWidget->setWindowTitle(QApplication::applicationName() +
                                   " | Settings");
    settingsWidget->setWindowFlags(Qt::Dialog);

    connect(settingsWidget, SIGNAL(initLock()), this, SLOT(initLock()));
    connect(settingsWidget, SIGNAL(changeLockPassword()), this,
            SLOT(changeLockPassword()));
    connect(settingsWidget, SIGNAL(appAutoLockChanged()), this,
            SLOT(appAutoLockChanged()));

    connect(settingsWidget, SIGNAL(updateWindowTheme()), this,
            SLOT(updateWindowTheme()));
    connect(settingsWidget, SIGNAL(updatePageTheme()), this,
            SLOT(updatePageTheme()));

    connect(settingsWidget, &SettingsWidget::muteToggled, settingsWidget,
            [=](const bool checked) { this->toggleMute(checked); });
    connect(settingsWidget, &SettingsWidget::userAgentChanged, settingsWidget,
            [=](QString userAgentStr) {
              if (webEngine->page()->profile()->httpUserAgent() !=
                  userAgentStr) {
                settings.setValue("useragent", userAgentStr);
                this->updateSettingsUserAgentWidget();
                this->askToReloadPage();
              }
            });
    connect(settingsWidget, &SettingsWidget::autoPlayMediaToggled,
            settingsWidget, [=](bool checked) {
              QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
              auto *webSettings = profile->settings();
              webSettings->setAttribute(
                  QWebEngineSettings::PlaybackRequiresUserGesture, checked);

              this->webEngine->page()->profile()->settings()->setAttribute(
                  QWebEngineSettings::PlaybackRequiresUserGesture, checked);
            });

    connect(settingsWidget, &SettingsWidget::dictChanged, settingsWidget,
            [=](QString dictName) {
              if (webEngine && webEngine->page()) {
                webEngine->page()->profile()->setSpellCheckLanguages(
                    QStringList() << dictName);
              }
            });

    connect(settingsWidget, &SettingsWidget::spellCheckChanged, settingsWidget,
            [=](bool checked) {
              if (webEngine && webEngine->page()) {
                webEngine->page()->profile()->setSpellCheckEnabled(checked);
              }
            });

    connect(
        settingsWidget, &SettingsWidget::zoomChanged, settingsWidget, [=]() {
          if (windowState() == Qt::WindowNoState) {
            double currentFactor = settings.value("zoomFactor", 1.0).toDouble();
            webEngine->page()->setZoomFactor(currentFactor);
          }
        });

    connect(settingsWidget, &SettingsWidget::zoomMaximizedChanged,
            settingsWidget, [=]() {
              if (windowState() == Qt::WindowMaximized ||
                  windowState() == Qt::WindowFullScreen) {
                double currentFactor = settings
                                           .value("zoomFactorMaximized",
                                                  defaultZoomFactorMaximized)
                                           .toDouble();
                webEngine->page()->setZoomFactor(currentFactor);
              }
            });

    connect(settingsWidget, &SettingsWidget::notificationPopupTimeOutChanged,
            settingsWidget, [=]() {
              setNotificationPresenter(this->webEngine->page()->profile());
            });

    connect(settingsWidget, &SettingsWidget::notify, settingsWidget,
            [=](QString message) { notify("", message); });

    connect(settingsWidget, &SettingsWidget::updateFullWidthView,
            settingsWidget, [=](bool checked) {
              if (webEngine && webEngine->page()) {
                if (checked)
                  webEngine->page()->runJavaScript(
                      "document.querySelector('body').classList.add('whatsie-"
                      "full-view');");
                else
                  webEngine->page()->runJavaScript(
                      "document.querySelector('body').classList.remove('"
                      "whatsie-full-view');");
              }
            });

    settingsWidget->appLockSetChecked(
        settings.value("lockscreen", false).toBool());

    // spell checker
    settingsWidget->loadDictionaries(m_dictionaries);
  }
}

void MainWindow::changeEvent(QEvent *e) {
  if (e->type() == QEvent::WindowStateChange) {
    handleZoomOnWindowStateChange(static_cast<QWindowStateChangeEvent *>(e));
  }
  QMainWindow::changeEvent(e);
}

void MainWindow::handleZoomOnWindowStateChange(QWindowStateChangeEvent *ev) {
  if (settingsWidget != nullptr) {
    if (ev->oldState().testFlag(Qt::WindowMaximized) &&
        windowState().testFlag(Qt::WindowNoState)) {
      emit settingsWidget->zoomChanged();
    } else if ((!ev->oldState().testFlag(Qt::WindowMaximized) &&
                windowState().testFlag(Qt::WindowMaximized)) ||
               (!ev->oldState().testFlag(Qt::WindowMaximized) &&
                windowState().testFlag(Qt::WindowFullScreen))) {
      emit settingsWidget->zoomMaximizedChanged();
    }
  }
}

void MainWindow::handleZoom() {
  if (windowState().testFlag(Qt::WindowMaximized) ||
      windowState().testFlag(Qt::WindowFullScreen)) {
    double currentFactor =
        settings.value("zoomFactorMaximized", defaultZoomFactorMaximized)
            .toDouble();
    webEngine->page()->setZoomFactor(currentFactor);
  } else if (windowState().testFlag(Qt::WindowNoState)) {
    double currentFactor = settings.value("zoomFactor", 1.0).toDouble();
    webEngine->page()->setZoomFactor(currentFactor);
  }
}

void MainWindow::lockApp() {
  if (lockWidget != nullptr && lockWidget->getIsLocked())
    return;

  if (settings.value("asdfg").isValid()) {
    initLock();
    lockWidget->lock_app();
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
  if (settingsWidget != nullptr) {
    settingsWidget->toggleTheme();
  }
}

void MainWindow::showSettings(bool isAskedByCLI) {
  if (lockWidget && lockWidget->getIsLocked()) {
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

  if (webEngine == nullptr) {
    QMessageBox::critical(
        this, QApplication::applicationName() + "| Error",
        "Unable to initialize settings module.\nWebengine is not initialized.");
    return;
  }
  if (!settingsWidget->isVisible()) {
    this->updateSettingsUserAgentWidget();
    settingsWidget->refresh();
    int screenNumber = qApp->desktop()->screenNumber(this);
    QRect screenRect = QGuiApplication::screens().at(screenNumber)->geometry();
    if (!screenRect.contains(settingsWidget->pos())) {
      settingsWidget->move(screenRect.center() -
                           settingsWidget->rect().center());
    }
    settingsWidget->show();
  }
}

void MainWindow::updateSettingsUserAgentWidget() {
  settingsWidget->updateDefaultUAButton(
      this->webEngine->page()->profile()->httpUserAgent());
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
  about->setAttribute(Qt::WA_DeleteOnClose);
  about->show();
}

void MainWindow::closeEvent(QCloseEvent *event) {
  settings.setValue("geometry", saveGeometry());
  getPageTheme();
  QTimer::singleShot(500, settingsWidget, [=]() { settingsWidget->refresh(); });

  if (QSystemTrayIcon::isSystemTrayAvailable() &&
      settings.value("closeButtonActionCombo", 0).toInt() == 0) {
    this->hide();
    event->ignore();
    if (settings.value("firstrun_tray", true).toBool()) {
      notify(QApplication::applicationName(), "Minimized to system tray.");
      settings.setValue("firstrun_tray", false);
    }
    return;
  }
  event->accept();
  quitApp();
  QMainWindow::closeEvent(event);
}

void MainWindow::notify(QString title, QString message) {

  if (settings.value("disableNotificationPopups", false).toBool() == true) {
    return;
  }

  if (title.isEmpty())
    title = QApplication::applicationName();

  if (settings.value("notificationCombo", 1).toInt() == 0 &&
      trayIcon != nullptr) {
    trayIcon->showMessage(title, message,
                          QIcon(":/icons/app/notification/whatsie-notify.png"),
                          settings.value("notificationTimeOut", 9000).toInt());
    trayIcon->disconnect(trayIcon, SIGNAL(messageClicked()));
    connect(trayIcon, &QSystemTrayIcon::messageClicked, trayIcon, [=]() {
      if (windowState().testFlag(Qt::WindowMinimized) ||
          !windowState().testFlag(Qt::WindowActive)) {
        activateWindow();
        this->show();
      }
    });
  } else {
    auto popup = new NotificationPopup(webEngine);
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

  openUrlAction = new QAction("New Chat", this);
  openUrlAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_N));
  connect(openUrlAction, &QAction::triggered, this, &MainWindow::newChat);
  addAction(openUrlAction);

  fullscreenAction = new QAction(tr("Fullscreen"), this);
  fullscreenAction->setShortcut(Qt::Key_F11);
  connect(fullscreenAction, &QAction::triggered, fullscreenAction,
          [=]() { setWindowState(windowState() ^ Qt::WindowFullScreen); });
  addAction(fullscreenAction);

  minimizeAction = new QAction(tr("Mi&nimize to tray"), this);
  connect(minimizeAction, &QAction::triggered, this, &QMainWindow::hide);
  addAction(minimizeAction);

  QShortcut *minimizeShortcut = new QShortcut(
      QKeySequence(Qt::Modifier::CTRL + Qt::Key_W), this, SLOT(hide()));
  minimizeShortcut->setAutoRepeat(false);

  restoreAction = new QAction(tr("&Restore"), this);
  connect(restoreAction, &QAction::triggered, this, &QMainWindow::show);
  addAction(restoreAction);

  reloadAction = new QAction(tr("Re&load"), this);
  reloadAction->setShortcut(Qt::Key_F5);
  connect(reloadAction, &QAction::triggered, this, [=] { this->doReload(); });
  addAction(reloadAction);

  lockAction = new QAction(tr("Loc&k"), this);
  lockAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_L));
  connect(lockAction, &QAction::triggered, this, &MainWindow::lockApp);
  addAction(lockAction);

  settingsAction = new QAction(tr("&Settings"), this);
  settingsAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_P));
  connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);
  addAction(settingsAction);

  toggleThemeAction = new QAction(tr("&Toggle theme"), this);
  toggleThemeAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_T));
  connect(toggleThemeAction, &QAction::triggered, this,
          &MainWindow::toggleTheme);
  addAction(toggleThemeAction);

  aboutAction = new QAction(tr("&About"), this);
  connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

  quitAction = new QAction(tr("&Quit"), this);
  quitAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_Q));
  connect(quitAction, &QAction::triggered, this, &MainWindow::quitApp);
  addAction(quitAction);
}

void MainWindow::quitApp() {
  settings.setValue("geometry", saveGeometry());
  getPageTheme();
  QTimer::singleShot(500, &settings, [=]() {
    settings.setValue("firstrun_tray", true);
    qApp->quit();
  });
}

void MainWindow::createTrayIcon() {
  trayIconMenu = new QMenu(this);
  trayIconMenu->setObjectName("trayIconMenu");
  trayIconMenu->addAction(minimizeAction);
  trayIconMenu->addAction(restoreAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(reloadAction);
  trayIconMenu->addAction(lockAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(openUrlAction);
  trayIconMenu->addAction(toggleThemeAction);
  trayIconMenu->addAction(settingsAction);
  trayIconMenu->addAction(aboutAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);

  trayIcon = new QSystemTrayIcon(trayIconNormal, this);
  trayIcon->setContextMenu(trayIconMenu);
  connect(trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(checkWindowState()));

  trayIcon->show();

  connect(trayIcon, &QSystemTrayIcon::messageClicked, this,
          &MainWindow::messageClicked);
  connect(trayIcon, &QSystemTrayIcon::activated, this,
          &MainWindow::iconActivated);

  // enable show shortcuts in menu
  if (qApp->styleHints()->showShortcutsInContextMenus()) {
    foreach (QAction *action, trayIconMenu->actions()) {
      action->setShortcutVisibleInContextMenu(true);
    }
  }
}

void MainWindow::initLock() {

  if (lockWidget == nullptr) {
    lockWidget = new Lock(this);
    lockWidget->setObjectName("lockWidget");

    lockWidget->setWindowFlags(Qt::Widget);
    lockWidget->setStyleSheet(
        "QWidget#login{background-color:palette(window)};"
        "QWidget#signup{background-color:palette(window)}");
    lockWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(lockWidget, &Lock::passwordNotSet, settingsWidget, [=]() {
      settings.setValue("lockscreen", false);
      settingsWidget->appLockSetChecked(false);
    });

    connect(lockWidget, &Lock::unLocked, [=]() {
      // unlock event
    });

    connect(lockWidget, &Lock::passwordSet, settingsWidget, [=]() {
      if (settings.value("asdfg").isValid()) {
        settingsWidget->setCurrentPasswordText(QByteArray::fromBase64(
            settings.value("asdfg").toString().toUtf8()));
      } else {
        settingsWidget->setCurrentPasswordText("Require setup");
      }
      settingsWidget->appLockSetChecked(
          settings.value("lockscreen", false).toBool());
    });
    lockWidget->applyThemeQuirks();
  }

  lockWidget->setGeometry(this->rect());

  if (settings.value("lockscreen").toBool()) {
    if (settings.value("asdfg").isValid()) {
      lockWidget->lock_app();
    } else {
      lockWidget->signUp();
    }
    lockWidget->show();
  } else {
    lockWidget->hide();
  }
  updateWindowTheme();
}

void MainWindow::changeLockPassword() {
  settings.remove("asdfg");
  settingsWidget->appLockSetChecked(false);
  settingsWidget->autoAppLockSetChecked(false);
  settingsWidget->updateAppLockPasswordViewer();
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
  bool enabled = settings.value("appAutoLocking", defaultAppAutoLock).toBool();
  if (enabled) {
    autoLockEventFilter->setTimeoutmillis(
        settings.value("autoLockDuration", defaultAppAutoLockDuration).toInt() *
        1000);
    qApp->installEventFilter(autoLockEventFilter);
    autoLockEventFilter->resetTimer();
  } else {
    autoLockEventFilter->stopTimer();
    qApp->removeEventFilter(autoLockEventFilter);
  }
}

// check window state and set tray menus
void MainWindow::checkWindowState() {
  QObject *tray_icon_menu = this->findChild<QObject *>("trayIconMenu");
  if (tray_icon_menu != nullptr) {
    if (this->isVisible()) {
      ((QMenu *)(tray_icon_menu))->actions().at(0)->setDisabled(false);
      ((QMenu *)(tray_icon_menu))->actions().at(1)->setDisabled(true);
    } else {
      ((QMenu *)(tray_icon_menu))->actions().at(0)->setDisabled(true);
      ((QMenu *)(tray_icon_menu))->actions().at(1)->setDisabled(false);
    }
    if (lockWidget && lockWidget->getIsLocked()) {
      ((QMenu *)(tray_icon_menu))->actions().at(4)->setDisabled(true);
    } else {
      ((QMenu *)(tray_icon_menu))->actions().at(4)->setDisabled(false);
    }
  }
}

void MainWindow::initGlobalWebProfile() {

  QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
  profile->setHttpUserAgent(
      settings.value("useragent", defaultUserAgentStr).toString());

  QStringList dict_names;
  dict_names.append(settings.value("sc_dict", "en-US").toString());

  profile->setSpellCheckEnabled(settings.value("sc_enabled", true).toBool());
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
                            settings.value("autoPlayMedia", false).toBool());
}

void MainWindow::createWebEngine() {
  initGlobalWebProfile();

  QSizePolicy widgetSize;
  widgetSize.setHorizontalPolicy(QSizePolicy::Expanding);
  widgetSize.setVerticalPolicy(QSizePolicy::Expanding);
  widgetSize.setHorizontalStretch(1);
  widgetSize.setVerticalStretch(1);

  m_dictionaries = Dictionaries::GetDictionaries();

  WebView *webEngine = new WebView(this, m_dictionaries);
  setCentralWidget(webEngine);
  webEngine->setSizePolicy(widgetSize);
  webEngine->show();

  this->webEngine = webEngine;

  webEngine->addAction(minimizeAction);
  webEngine->addAction(lockAction);
  webEngine->addAction(quitAction);

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
  dict_names.append(settings.value("sc_dict", "en-US").toString());

  profile->setSpellCheckEnabled(settings.value("sc_enabled", true).toBool());
  profile->setSpellCheckLanguages(dict_names);
  profile->setHttpUserAgent(
      settings.value("useragent", defaultUserAgentStr).toString());

  setNotificationPresenter(profile);

  QWebEnginePage *page = new WebEnginePage(profile, webEngine);
  if (settings.value("windowTheme", "light").toString() == "dark") {
    page->setBackgroundColor(QColor(17, 27, 33)); // whatsapp dark bg color
  } else {
    page->setBackgroundColor(QColor(240, 240, 240)); // whatsapp light bg color
  }
  webEngine->setPage(page);
  // page should be set parent of profile to prevent
  // Release of profile requested but WebEnginePage still not deleted. Expect
  // troubles !
  profile->setParent(page);
  auto randomValue = QRandomGenerator::global()->generateDouble() * 300;
  page->setUrl(
      QUrl("https://web.whatsapp.com?v=" + QString::number(randomValue)));

  connect(profile, &QWebEngineProfile::downloadRequested,
          &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested);

  connect(page, SIGNAL(fullScreenRequested(QWebEngineFullScreenRequest)), this,
          SLOT(fullScreenRequested(QWebEngineFullScreenRequest)));

  double currentFactor = settings.value("zoomFactor", 1.0).toDouble();
  webEngine->page()->setZoomFactor(currentFactor);
}

void MainWindow::setNotificationPresenter(QWebEngineProfile *profile) {
  auto *op = webEngine->findChild<NotificationPopup *>("engineNotifier");
  if (op != nullptr) {
    op->close();
    op->deleteLater();
  }

  auto popup = new NotificationPopup(webEngine);
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
        if (settings.value("disableNotificationPopups", false).toBool() ==
            true) {
          return;
        }
        if (settings.value("notificationCombo", 1).toInt() == 0 &&
            trayIcon != nullptr) {
          QIcon icon(QPixmap::fromImage(notification->icon()));
          trayIcon->showMessage(
              notification->title(), notification->message(), icon,
              settings.value("notificationTimeOut", 9000).toInt());
          trayIcon->disconnect(trayIcon, SIGNAL(messageClicked()));
          connect(trayIcon, &QSystemTrayIcon::messageClicked, trayIcon, [=]() {
            if (windowState().testFlag(Qt::WindowMinimized) ||
                !windowState().testFlag(Qt::WindowActive) || this->isHidden()) {
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
    webEngine->showFullScreen();
    this->showFullScreen();
    request.accept();
  } else {
    webEngine->showNormal();
    this->show();
    request.accept();
  }
}

void MainWindow::handleWebViewTitleChanged(QString title) {
  setWindowTitle(QApplication::applicationName() + ": " + title);

  if (notificationsTitleRegExp.exactMatch(title)) {
    if (notificationsTitleRegExp.capturedTexts().isEmpty() == false) {
      QString capturedTitle =
          notificationsTitleRegExp.capturedTexts().constFirst();
      unreadMessageCountRegExp.setMinimal(true);
      if (unreadMessageCountRegExp.indexIn(capturedTitle) != -1) {
        QString unreadMessageCountStr =
            unreadMessageCountRegExp.capturedTexts().constLast();
        int unreadMessageCount = unreadMessageCountStr.toInt();

        restoreAction->setText(
            tr("Restore") + " | " + unreadMessageCountStr + " " +
            (unreadMessageCount > 1 ? tr("messages") : tr("message")));
        trayIcon->setIcon(getTrayIcon(unreadMessageCount));
        setWindowIcon(getTrayIcon(unreadMessageCount));
      }
    }
  } else {
    trayIcon->setIcon(trayIconNormal);
    setWindowIcon(trayIconNormal);
  }
}

void MainWindow::handleLoadFinished(bool loaded) {
  if (loaded) {
    qDebug() << "Loaded";
    checkLoadedCorrectly();
    updatePageTheme();
    handleZoom();
    injectMutationObserver();
    injectFullWidthJavaScript();
    injectClassChangeObserver();
    injectNewChatJavaScript();
    if (settingsWidget != nullptr) {
      settingsWidget->refresh();
    }
  }
}

void MainWindow::injectClassChangeObserver() {
  QString js = R"(
            const observer = new MutationObserver(() => {
                var haveFullView = document.body.classList.contains('whatsie-full-view');
                var container = document.querySelector('#app > .app-wrapper-web > div');
                if(container){
                    if(haveFullView){
                        container.style.width = '100%';
                        container.style.height = '100%';
                        container.style.top = '0';
                        container.style.maxWidth = 'unset';
                    }else{
                        container.style.width = null;
                        container.style.height = null;
                        container.style.top = null;
                        container.style.maxWidth = null;
                    }
                }
            });
            observer.observe(document.body, {
            attributes: true,
            attributeFilter: ['class'],
            childList: false,
            characterData: false
        });)";
  webEngine->page()->runJavaScript(js);
}

void MainWindow::injectMutationObserver() {
  QString js =
      R"(function waitForElement(selector) {
                return new Promise(resolve => {
                    if (document.querySelector(selector)) {
                        return resolve(document.querySelector(selector));
                    }
                    const observer = new MutationObserver(mutations => {
                        if (document.querySelector(selector)) {
                            resolve(document.querySelector(selector));
                            observer.disconnect();
                        }
                    });
                    observer.observe(document.body, {
                        childList: true,
                        subtree: true
                    });
                });
            };)";
  webEngine->page()->runJavaScript(js);
}

void MainWindow::injectFullWidthJavaScript() {
  if (!settings.value("fullWidthView", true).toBool())
    return;
  QString js =
      R"(waitForElement('#pane-side').then( () => {
            var container = document.querySelector('#app > .app-wrapper-web > div');
            container.style.width = '100%';
            container.style.height = '100%';
            container.style.top = '0';
            container.style.maxWidth = 'unset';
         });
        )";
  webEngine->page()->runJavaScript(js);
}

void MainWindow::injectNewChatJavaScript() {
  QString js = R"(const openNewChatWhatsie = (phone,text) => {
                    const link = document.createElement('a');
                    link.setAttribute('href',
                    `whatsapp://send/?phone=${phone}&text=${text}`);
                    document.body.append(link);
                    link.click();
                    document.body.removeChild(link);
                };
                function openNewChatWhatsieDefined()
                {
                    return (openNewChatWhatsie != 'undefined');
                })";
  webEngine->page()->runJavaScript(js);
}

void MainWindow::checkLoadedCorrectly() {
  if (webEngine && webEngine->page()) {
    // test 1 based on the class name of body tag of the page
    webEngine->page()->runJavaScript(
        "document.querySelector('body').className",
        [this](const QVariant &result) {
          if (result.toString().contains("page-version", Qt::CaseInsensitive)) {
            qDebug() << "Test 1 found" << result.toString();
            webEngine->page()->runJavaScript(
                "document.getElementsByTagName('body')[0].innerText = ''");
            loadingQuirk("test1");
          } else if (webEngine->title().contains("Error",
                                                 Qt::CaseInsensitive)) {
            utils::delete_cache(webEngine->page()->profile()->cachePath());
            utils::delete_cache(
                webEngine->page()->profile()->persistentStoragePath());
            settings.setValue("useragent", defaultUserAgentStr);
            utils::DisplayExceptionErrorDialog(
                "test1 handleWebViewTitleChanged(title) title: Error, "
                "Resetting UA, Quiting!\nUA: " +
                settings.value("useragent", "DefaultUA").toString());
            quitAction->trigger();
          } else {
            qDebug() << "Test 1 loaded correctly, value:" << result.toString();
          }
        });
  }
}

void MainWindow::loadingQuirk(QString test) {
  // contains ug message apply quirk
  if (correctlyLoaderRetries > -1) {
    qWarning() << test << "checkLoadedCorrectly()/loadingQuirk()/doReload()"
               << correctlyLoaderRetries;
    doReload(false, false, true);
    correctlyLoaderRetries--;
  } else {
    utils::delete_cache(webEngine->page()->profile()->cachePath());
    utils::delete_cache(webEngine->page()->profile()->persistentStoragePath());
    settings.setValue("useragent", defaultUserAgentStr);
    utils::DisplayExceptionErrorDialog(
        test +
        " checkLoadedCorrectly()/loadingQuirk() reload retries 0, Resetting "
        "UA, Quiting!\nUA: " +
        settings.value("useragent", "DefaultUA").toString());

    quitAction->trigger();
  }
}

// unused direct method to download file without having entry in download
// manager
void MainWindow::handleDownloadRequested(QWebEngineDownloadItem *download) {
  QFileDialog dialog(this);
  bool usenativeFileDialog =
      settings.value("useNativeFileDialog", false).toBool();

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
  Q_UNUSED(reason);
  if (settings.value("minimizeOnTrayIconClick", false).toBool() == false ||
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

  if (this->webEngine->page()) {
    this->webEngine->page()->disconnect();
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

void MainWindow::triggerNewChat(QString phone, QString text) {
  static QString phoneStr, textStr;
  webEngine->page()->runJavaScript(
      "openNewChatWhatsieDefined()",
      [this, phone, text](const QVariant &result) {
        if (result.toString().contains("true")) {
          this->webEngine->page()->runJavaScript(
              QString("openNewChatWhatsie(\"%1\",\"%2\")").arg(phone, text));
        } else {
          // create send url equivalent
          phoneStr = phone.isEmpty() ? "" : "phone=" + phone;
          textStr = text.isEmpty() ? "" : "text=" + text;
          QString urlStr =
              "https://web.whatsapp.com/send?" + phoneStr + "&" + textStr;
          this->webEngine->page()->load(QUrl(urlStr));
        }
        this->alreadyRunning();
      });
}

void MainWindow::doReload(bool byPassCache, bool isAskedByCLI,
                          bool byLoadingQuirk) {
  if (byLoadingQuirk) {
    this->webEngine->triggerPageAction(QWebEnginePage::ReloadAndBypassCache,
                                       byPassCache);
  } else {
    if (lockWidget && !lockWidget->getIsLocked()) {
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
    this->webEngine->triggerPageAction(QWebEnginePage::ReloadAndBypassCache,
                                       byPassCache);
  }
}

void MainWindow::toggleMute(const bool &checked) {
  this->webEngine->page()->setAudioMuted(checked);
}

// get value of page theme when page is loaded
QString MainWindow::getPageTheme() {
  static QString theme = "web"; // implies light
  if (webEngine && webEngine->page()) {
    webEngine->page()->runJavaScript(
        "document.querySelector('body').className;",
        [this](const QVariant &result) {
          theme = result.toString();
          theme.contains("dark") ? theme = "dark" : theme = "light";
          settings.setValue("windowTheme", theme);
        });
  }
  return theme;
}

void MainWindow::tryLock() {
  if (settings.value("asdfg").isValid()) {
    initLock();
    return;
  }
  if (settings.value("asdfg").isValid() == false) {
    settings.setValue("lockscreen", false);
    settings.setValue("appAutoLocking", false);
    settingsWidget->appAutoLockingSetChecked(false);
    settingsWidget->appLockSetChecked(false);
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
