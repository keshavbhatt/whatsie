#include "mainwindow.h"

#include <QInputDialog>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStyleHints>
#include <QUrlQuery>
#include <QWebEngineNotification>

extern QString defaultUserAgentStr;
extern double defaultZoomFactorMaximized;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), notificationsTitleRegExp("^\\([1-9]\\d*\\).*"),
      trayIconRead(":/icons/app/icon-32.png"),
      trayIconUnread(":/icons/app/whatsapp-message-32.png") {

  setObjectName("MainWindow");
  setWindowTitle(QApplication::applicationName());
  setWindowIcon(QIcon(":/icons/app/icon-256.png"));
  setMinimumWidth(750);
  setMinimumHeight(640);
  restoreGeometry(settings.value("geometry").toByteArray());
  restoreState(settings.value("windowState").toByteArray());
  initThemes();
  createActions();
  createTrayIcon();
  createWebEngine();
  init_settingWidget();
  initRateWidget();
  tryLock();
  updateWindowTheme();
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

void MainWindow::runMinimized() { this->minimizeAction->trigger(); }

MainWindow::~MainWindow() { webEngine->deleteLater(); }

void MainWindow::loadAppWithArgument(const QString &arg) {
  // https://faq.whatsapp.com/iphone/how-to-link-to-whatsapp-from-a-different-app/?lang=en

  // The WhatsApp Messenger application
  if (arg.contains("://app")) {
    this->show(); // restore app
    return;
  }

  // PASSED SCHEME whatsapp://send?text=Hello%2C%20World!&phone=919568388397"
  // CONVERTED URI
  // https://web.whatsapp.com/send?phone=919568388397&text=Hello%2C%20World New
  // chat composer
  if (arg.contains("send?") || arg.contains("send/?")) {
    QString newArg = arg;
    qWarning() << "New chat composer";
    newArg = newArg.replace("?", "&");
    QUrlQuery query(newArg);
    qWarning() << query.hasQueryItem("phone");
    QString phone, phoneStr, text, textStr, urlStr;
    // create send url equivalent
    phone = query.queryItemValue("phone");
    text = query.queryItemValue("text");
    phoneStr = phone.isEmpty() ? "" : "phone=" + phone;
    textStr = text.isEmpty() ? "" : "text=" + text;
    urlStr = "https://web.whatsapp.com/send?" + phoneStr + "&" + textStr;
    qWarning() << "Loading" << urlStr;
    this->webEngine->page()->load(QUrl(urlStr));
    return;
  }
}

void MainWindow::updatePageTheme() {
  QString webPageTheme = "web"; // implies light
  QString windowTheme = settings.value("windowTheme", "light").toString();
  if (windowTheme == "dark") {
    webPageTheme = "web dark";
  }
  if (webEngine && webEngine->page()) {
    webEngine->page()->runJavaScript(
        "document.querySelector('body').className='" + webPageTheme + "';",
        [](const QVariant &result) {
          qDebug() << "Value is: " << result.toString() << Qt::endl;
        });
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

  foreach (QWidget *w, widgets) { w->setPalette(qApp->palette()); }

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

bool MainWindow::isLoggedIn(){
    static bool loggedIn = false;
    if (webEngine && webEngine->page()) {
        webEngine->page()->runJavaScript(
          "window.localStorage.getItem('WAToken2')",
          [=](const QVariant &result) { qDebug() <<Q_FUNC_INFO << result;
            if(result.isValid() && result.toString().isEmpty() == false){
                loggedIn = true;
            }
        });
        qDebug() << "isLoggedIn" <<loggedIn;
        return loggedIn;
    }else{
        qDebug() << "isLoggedIn" <<loggedIn;
        return loggedIn;
    }
}

void MainWindow::tryLogOut() {
  if (webEngine && webEngine->page()) {
    webEngine->page()->runJavaScript(
        "document.querySelector(\"span[data-testid|='menu']\").click();"
        "document.querySelector(\"#side > header > div > div > span > div > "
        "span > div > ul > li:nth-child(5) > div\").click()",
        [=](const QVariant &result) { qDebug() << Q_FUNC_INFO << result; });
  }
}

void MainWindow::init_settingWidget() {
  if (settingsWidget == nullptr) {
    settingsWidget = new SettingsWidget(
        this, webEngine->page()->profile()->cachePath(),
        webEngine->page()->profile()->persistentStoragePath());
    settingsWidget->setWindowTitle(QApplication::applicationName() +
                                   " | Settings");
    settingsWidget->setWindowFlags(Qt::Dialog);

    connect(settingsWidget, SIGNAL(init_lock()), this, SLOT(init_lock()));
    connect(settingsWidget, SIGNAL(change_lock_password()), this,
            SLOT(change_lock_password()));

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
  if (windowState() == Qt::WindowMaximized ||
      windowState() == Qt::WindowFullScreen) {
    double currentFactor =
        settings.value("zoomFactorMaximized", defaultZoomFactorMaximized)
            .toDouble();
    webEngine->page()->setZoomFactor(currentFactor);
  } else if (windowState() == Qt::WindowNoState) {
    double currentFactor = settings.value("zoomFactor", 1.0).toDouble();
    webEngine->page()->setZoomFactor(currentFactor);
  }
}

void MainWindow::lockApp() {
  if (lockWidget != nullptr && lockWidget->isLocked)
    return;

  if (settings.value("asdfg").isValid()) {
    init_lock();
    lockWidget->lock_app();
  } else {
    QMessageBox msgBox;
    msgBox.setText("App lock is not configured.");
    msgBox.setIconPixmap(
        QPixmap(":/icons/information-line.png")
            .scaled(42, 42, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.setInformativeText("Do you want to setup App lock now ?");
    msgBox.setStandardButtons(QMessageBox::Cancel);
    QPushButton *setAppLock =
        new QPushButton(this->style()->standardIcon(QStyle::SP_DialogYesButton),
                        "Yes", nullptr);
    msgBox.addButton(setAppLock, QMessageBox::NoRole);
    connect(setAppLock, &QPushButton::clicked, setAppLock,
            [=]() { init_lock(); });
    msgBox.exec();
  }
}

void MainWindow::showSettings() {
  if (lockWidget && lockWidget->isLocked) {
    QMessageBox::critical(this, QApplication::applicationName() + "| Error",
                          "UnLock Application to access Settings.");
    this->show();
    return;
  }

  if (webEngine == nullptr) {
    QMessageBox::critical(
        this, QApplication::applicationName() + "| Error",
        "Unable to initialize settings module.\nIs webengine initialized?");
    return;
  }
  if (!settingsWidget->isVisible()) {
    this->updateSettingsUserAgentWidget();
    settingsWidget->refresh();
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
  settings.setValue("windowState", saveState());
  getPageTheme();
  QTimer::singleShot(500, settingsWidget, [=]() { settingsWidget->refresh(); });

  if (QSystemTrayIcon::isSystemTrayAvailable() &&
      settings.value("closeButtonActionCombo", 0).toInt() == 0) {
    this->hide();
    event->ignore();
    if (settings.value("firstrun_tray", true).toBool()) {
      notify(QApplication::applicationName(),
             "Application is minimized to system tray.");
      settings.setValue("firstrun_tray", false);
    }
    return;
  }
  event->accept();
  qApp->quit();
  settings.setValue("firstrun_tray", true);
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
    trayIcon->showMessage(title, message, QIcon(":/icons/app/icon-64.png"),
                          settings.value("notificationTimeOut", 9000).toInt());
    trayIcon->disconnect(trayIcon, SIGNAL(messageClicked()));
    connect(trayIcon, &QSystemTrayIcon::messageClicked, trayIcon, [=]() {
      if (windowState() == Qt::WindowMinimized ||
          windowState() != Qt::WindowActive) {
        activateWindow();
        raise();
        showNormal();
      }
    });
  } else {
    auto popup = new NotificationPopup(webEngine);
    connect(popup, &NotificationPopup::notification_clicked, popup, [=]() {
      if (windowState() == Qt::WindowMinimized ||
          windowState() != Qt::WindowActive) {
        activateWindow();
        raise();
        showNormal();
      }
    });
    popup->style()->polish(qApp);
    popup->setMinimumWidth(300);
    popup->adjustSize();
    popup->present(title, message, QPixmap(":/icons/app/icon-64.png"));
  }
}

void MainWindow::createActions() {

  openUrlAction = new QAction("New Chat", this);
  this->addAction(openUrlAction);
  openUrlAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_N));
  connect(openUrlAction, &QAction::triggered, this, &MainWindow::newChat);

  fullscreenAction = new QAction(tr("Fullscreen"), this);
  fullscreenAction->setShortcut(Qt::Key_F11);
  connect(fullscreenAction, &QAction::triggered, fullscreenAction,
          [=]() { setWindowState(windowState() ^ Qt::WindowFullScreen); });
  this->addAction(fullscreenAction);

  minimizeAction = new QAction(tr("Mi&nimize to tray"), this);
  minimizeAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_H));
  connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);
  addAction(minimizeAction);
  this->addAction(minimizeAction);

  restoreAction = new QAction(tr("&Restore"), this);
  connect(restoreAction, &QAction::triggered, this, &QWidget::show);
  addAction(restoreAction);

  reloadAction = new QAction(tr("Re&load"), this);
  reloadAction->setShortcut(Qt::Key_F5);
  connect(reloadAction, &QAction::triggered, this, &MainWindow::doReload);
  addAction(reloadAction);

  lockAction = new QAction(tr("Loc&k"), this);
  lockAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_L));
  connect(lockAction, &QAction::triggered, this, &MainWindow::lockApp);
  addAction(lockAction);
  this->addAction(lockAction);

  settingsAction = new QAction(tr("&Settings"), this);
  connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);

  aboutAction = new QAction(tr("&About"), this);
  connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

  quitAction = new QAction(tr("&Quit"), this);
  quitAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_Q));
  connect(quitAction, &QAction::triggered, this, &MainWindow::quitApp);
  addAction(quitAction);
  this->addAction(quitAction);
}

void MainWindow::quitApp() {
  getPageTheme();
  QTimer::singleShot(500, &settings, [=]() {
    qWarning() << "THEME" << settings.value("windowTheme").toString();
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
  trayIconMenu->addAction(settingsAction);
  trayIconMenu->addAction(aboutAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);

  trayIcon = new QSystemTrayIcon(trayIconRead, this);
  trayIcon->setContextMenu(trayIconMenu);
  connect(trayIconMenu, SIGNAL(aboutToShow()), this,
          SLOT(check_window_state()));

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

void MainWindow::init_lock() {

  if (lockWidget == nullptr) {
    lockWidget = new Lock(this);
    lockWidget->setObjectName("lockWidget");
  }

  lockWidget->setWindowFlags(Qt::Widget);
  lockWidget->setStyleSheet("QWidget#login{background-color:palette(window)};"
                            "QWidget#signup{background-color:palette(window)}");
  lockWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  lockWidget->setGeometry(this->rect());
  // lockWidget->disconnect();

  connect(lockWidget, &Lock::passwordNotSet, settingsWidget, [=]() {
    settings.setValue("lockscreen", false);
    settingsWidget->appLockSetChecked(false);
  });

  connect(lockWidget, &Lock::unLocked, [=]() {
    // unlock event
  });

  connect(lockWidget, &Lock::passwordSet, settingsWidget, [=]() {
    if (settings.value("asdfg").isValid()) {
      settingsWidget->setCurrentPasswordText(
          QByteArray::fromBase64(settings.value("asdfg").toString().toUtf8()));
    } else {
      settingsWidget->setCurrentPasswordText("Require setup");
    }
    settingsWidget->appLockSetChecked(
        settings.value("lockscreen", false).toBool());
  });

  lockWidget->applyThemeQuirks();
  lockWidget->show();
  if (settings.value("asdfg").isValid() &&
      settings.value("lockscreen").toBool()) {
    lockWidget->lock_app();
  } else if (settings.value("lockscreen").toBool() &&
             !settings.value("asdfg").isValid()) {
    lockWidget->signUp();
  } else {
    lockWidget->hide();
  }
  updateWindowTheme();
}

void MainWindow::change_lock_password() {
  settings.remove("asdfg");
  settingsWidget->appLockSetChecked(false);

  tryLogOut();
  QTimer::singleShot(2000, this, [=]() {
      if(isLoggedIn()){
        forceLogOut();
        doAppReload();
      }
      init_lock();
  });


}

// check window state and set tray menus
void MainWindow::check_window_state() {
  QObject *tray_icon_menu = this->findChild<QObject *>("trayIconMenu");
  if (tray_icon_menu != nullptr) {
    if (this->isVisible()) {
      ((QMenu *)(tray_icon_menu))->actions().at(0)->setDisabled(false);
      ((QMenu *)(tray_icon_menu))->actions().at(1)->setDisabled(true);
    } else {
      ((QMenu *)(tray_icon_menu))->actions().at(0)->setDisabled(true);
      ((QMenu *)(tray_icon_menu))->actions().at(1)->setDisabled(false);
    }
    if (lockWidget && lockWidget->isLocked) {
      ((QMenu *)(tray_icon_menu))->actions().at(4)->setDisabled(true);
    } else {
      ((QMenu *)(tray_icon_menu))->actions().at(4)->setDisabled(false);
    }
  }
}

void MainWindow::init_globalWebProfile() {

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
  init_globalWebProfile();

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
  // RequestInterceptor *interceptor = new RequestInterceptor(profile);
  // profile->setUrlRequestInterceptor(interceptor);
  auto randomValue = QRandomGenerator::global()->generateDouble() * 300;
  page->setUrl(
      QUrl("https://web.whatsapp.com?v=" + QString::number(randomValue)));
  connect(profile, &QWebEngineProfile::downloadRequested,
          &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested);

  connect(webEngine->page(),
          SIGNAL(fullScreenRequested(QWebEngineFullScreenRequest)), this,
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
    if (windowState() == Qt::WindowMinimized ||
        windowState() != Qt::WindowActive) {
      activateWindow();
      raise();
      showNormal();
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
            if (windowState() == Qt::WindowMinimized ||
                windowState() != Qt::WindowActive) {
              activateWindow();
              raise();
              showNormal();
            }
          });

        } else {
          popup->setMinimumWidth(300);
          popup->present(notification);
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
    this->showNormal();
    request.accept();
  }
}

void MainWindow::handleWebViewTitleChanged(QString title) {
  setWindowTitle(QApplication::applicationName() + ": " + title);

  if (notificationsTitleRegExp.exactMatch(title)) {
    if (notificationsTitleRegExp.isEmpty() == false &&
        notificationsTitleRegExp.capturedTexts().isEmpty() == false) {
      QString capturedTitle =
          notificationsTitleRegExp.capturedTexts().constFirst();
      QRegExp rgex("\\([^\\d]*(\\d+)[^\\d]*\\)");
      rgex.setMinimal(true);
      if (rgex.indexIn(capturedTitle) != -1) {
        QString unreadMessageCount = rgex.capturedTexts().constLast();
        QString suffix =
            unreadMessageCount.toInt() > 1 ? tr("messages") : tr("message");
        restoreAction->setText(tr("Restore") + " | " + unreadMessageCount +
                               " " + suffix);
      }
    }
    trayIcon->setIcon(trayIconUnread);
    setWindowIcon(trayIconUnread);
  } else {
    trayIcon->setIcon(trayIconRead);
    setWindowIcon(trayIconRead);
  }
}

void MainWindow::handleLoadFinished(bool loaded) {
  if (loaded) {
    checkLoadedCorrectly();
    updatePageTheme();
    handleZoom();
    settingsWidget->refresh();
  }
}

void MainWindow::checkLoadedCorrectly() {
  if (webEngine && webEngine->page()) {
    // test 1 based on the class name of body of the page
    webEngine->page()->runJavaScript(
        "document.querySelector('body').className",
        [this](const QVariant &result) {
          if (result.toString().contains("page-version", Qt::CaseInsensitive)) {
            qWarning() << "Test 1 found" << result.toString();
            webEngine->page()->runJavaScript(
                "document.getElementsByTagName('body')[0].innerText = ''");
            loadingQuirk("test1");
          } else if (webEngine->title().contains("Error",
                                                 Qt::CaseInsensitive)) {
            utils::delete_cache(webEngine->page()->profile()->cachePath());
            utils::delete_cache(
                webEngine->page()->profile()->persistentStoragePath());
            settings.setValue("useragent", defaultUserAgentStr);
            utils *util = new utils(this);
            util->DisplayExceptionErrorDialog(
                "test1 handleWebViewTitleChanged(title) title: Error, "
                "Resetting UA, Quiting!\nUA: " +
                settings.value("useragent", "DefaultUA").toString());

            quitAction->trigger();
          } else {
            qWarning() << "Test 1 loaded correctly value:" << result.toString();
          }
        });
  }
}

void MainWindow::loadingQuirk(QString test) {
  // contains ug message apply quirk
  if (correctlyLoaderRetries > -1) {
    qWarning() << test << "checkLoadedCorrectly()/loadingQuirk()/doReload()"
               << correctlyLoaderRetries;
    doReload();
    correctlyLoaderRetries--;
  } else {
    utils::delete_cache(webEngine->page()->profile()->cachePath());
    utils::delete_cache(webEngine->page()->profile()->persistentStoragePath());
    settings.setValue("useragent", defaultUserAgentStr);
    utils *util = new utils(this);
    util->DisplayExceptionErrorDialog(
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
  if (settings.value("minimizeOnTrayIconClick", false).toBool() == false)
    return;
  if (isVisible()) {
    hide();
  } else {
    showNormal();
  }
}

void MainWindow::messageClicked() {
  if (isVisible()) {
    hide();
  } else {
    showNormal();
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
  QString text = QInputDialog::getText(
      this, tr("New Chat"),
      tr("Enter a valid WhatsApp number with country code (ex- +91XXXXXXXXXX)"),
      QLineEdit::Normal, "", &ok);
  if (ok) {
    if (isPhoneNumber(text))
      this->webEngine->page()->load(
          QUrl("https://web.whatsapp.com/send?phone=" + text));
    else
      QMessageBox::information(this,
                               QApplication::applicationName() + "| Error",
                               "Invalid Phone Number");
  }
}

bool MainWindow::isPhoneNumber(const QString &phoneNumber) {
  const QString phone = "^\\+(((\\d{2}))\\s?)?((\\d{2})|(\\((\\d{2})\\))\\s?)?("
                        "\\d{3,15})(\\-(\\d{3,15}))?$";
  static QRegularExpression reg(phone);
  return reg.match(phoneNumber).hasMatch();
}

void MainWindow::doReload(bool byPassCache) {
  this->webEngine->triggerPageAction(QWebEnginePage::ReloadAndBypassCache,
                                     byPassCache);
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
  if (settings.value("asdfg").isValid() &&
      settings.value("lockscreen", false).toBool()) {
    init_lock();
  }
  if (settings.value("asdfg").isValid() == false) {
    settings.setValue("lockscreen", false);
    settingsWidget->appLockSetChecked(false);
    init_lock();
  }
}
