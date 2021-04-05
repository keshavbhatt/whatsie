#include "mainwindow.h"

#include <QRadioButton>

extern QString defaultUserAgentStr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      notificationsTitleRegExp("^\\([1-9]\\d*\\).*"),
      trayIconRead(":/icons/app/whatsapp.svg"),
      trayIconUnread(":/icons/app/whatsapp-message.svg")
{
    qApp->setQuitOnLastWindowClosed(false);

    setWindowTitle(QApplication::applicationName());
    setWindowIcon(QIcon(":/icons/app/icon-256.png"));
    setMinimumWidth(800);
    setMinimumHeight(600);


    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    createActions();
    createStatusBar();
    createTrayIcon();
    createWebEngine();


    if(settings.value("lockscreen",false).toBool())
    {
        init_lock();
    }
    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer,&QTimer::timeout,[=](){
        if(settings.value("asdfg").isValid()){
            if(lockWidget && lockWidget->isLocked==false){
                timer->stop();
                //init_accountWidget();
            }
        }
    });
    timer->start();

    init_settingWidget();

    lightPalette = qApp->palette();
    updateWindowTheme();

    // quit application if the download manager window is the only remaining window
    m_downloadManagerWidget.setAttribute(Qt::WA_QuitOnClose, false);
}

void MainWindow::updatePageTheme()
{
    QString webPageTheme = "web"; //implies light
    QString windowTheme  = settings.value("windowTheme","light").toString();
    if(windowTheme == "dark"){
        webPageTheme = "web dark";
    }
    if(webEngine && webEngine->page()){
        webEngine->page()->runJavaScript(
            "document.querySelector('body').className='"+webPageTheme+"';",
            [](const QVariant &result){
                qDebug() << "Value is: " << result.toString() << endl;
            }
        );
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if(lockWidget != nullptr){
        lockWidget->resize(event->size());
    }
}

void MainWindow::updateWindowTheme()
{
    if(settings.value("windowTheme","light").toString() == "dark")
    {

        qApp->setStyle(QStyleFactory::create("fusion"));
        QPalette palette;
        palette.setColor(QPalette::Window,QColor("#131C21")); //whatsapp dark color
        palette.setColor(QPalette::WindowText,Qt::white);
        palette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
        //palette.setColor(QPalette::Base,QColor(42,42,42));
        palette.setColor(QPalette::Base,QColor(84,84,84));

        palette.setColor(QPalette::AlternateBase,QColor(66,66,66));
        palette.setColor(QPalette::ToolTipBase,Qt::white);
        palette.setColor(QPalette::ToolTipText,QColor(53,53,53));
        palette.setColor(QPalette::Text,Qt::white);
        palette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
        palette.setColor(QPalette::Dark,QColor(35,35,35));
        palette.setColor(QPalette::Shadow,QColor(20,20,20));
        palette.setColor(QPalette::Button,QColor(53,53,53));
        palette.setColor(QPalette::ButtonText,Qt::white);
        palette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
        palette.setColor(QPalette::BrightText,Qt::red);
        palette.setColor(QPalette::Link,QColor("skyblue"));
        palette.setColor(QPalette::Highlight,QColor(49,106,150));
        palette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
        palette.setColor(QPalette::HighlightedText,Qt::white);
        palette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));
        qApp->setPalette(palette);
        this->webEngine->setStyleSheet("QWebEngineView{background:#131C21;}"); //whatsapp dark color
    }
    else{
        qApp->setPalette(lightPalette);
        this->update();
    }

    if(lockWidget!=nullptr){
        lockWidget->setStyleSheet("QWidget#login{background-color:palette(window)};"
                                  "QWidget#signup{background-color:palette(window)};"
                                  );
        lockWidget->applyThemeQuirks();
    }
}

void MainWindow::handleCookieAdded(const QNetworkCookie &cookie)
{
  qDebug() << cookie.toRawForm() << "\n\n\n";
}


void MainWindow::init_settingWidget()
{
    if(settingsWidget == nullptr)
    {
        settingsWidget = new SettingsWidget(this,webEngine->page()->profile()->cachePath()
                                            ,webEngine->page()->profile()->persistentStoragePath());
        settingsWidget->setWindowTitle(QApplication::applicationName()+" | Settings");
        settingsWidget->setWindowFlags(Qt::Dialog);

        connect(settingsWidget,SIGNAL(init_lock()),this,SLOT(init_lock()));
        connect(settingsWidget,SIGNAL(updateWindowTheme()),this,SLOT(updateWindowTheme()));
        connect(settingsWidget,SIGNAL(updatePageTheme()),this,SLOT(updatePageTheme()));
        connect(settingsWidget,&SettingsWidget::muteToggled,[=](const bool checked)
        {
            this->toggleMute(checked);
        });
        connect(settingsWidget,&SettingsWidget::userAgentChanged,[=](QString userAgentStr)
        {
            if(webEngine->page()->profile()->httpUserAgent() != userAgentStr)
            {
                settings.setValue("useragent",userAgentStr);
                this->updateSettingsUserAgentWidget();
                this->askToReloadPage();
            }
        });
        connect(settingsWidget,&SettingsWidget::autoPlayMediaToggled,[=](bool checked)
        {   QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();

            auto* webSettings = profile->settings();
            webSettings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture,checked);

            this->webEngine->page()->profile()->settings()->setAttribute(
                        QWebEngineSettings::PlaybackRequiresUserGesture,
                                   checked);
        });

        settingsWidget->appLockSetChecked(settings.value("lockscreen",false).toBool());
        settingsWidget->resize(settingsWidget->sizeHint().width(),settingsWidget->minimumSizeHint().height());
    }
}

void MainWindow::lockApp()
{
    if(settings.value("asdfg").isValid() && settings.value("lockscreen").toBool()==false){
        QMessageBox::critical(this,QApplication::applicationName()+"| Error",
                              "Unable to lock App, Enable AppLock in settings First.");
        this->show();
        return;
    }

    if(settings.value("asdfg").isValid()){
        init_lock();
    }else{
        if(settings.value("asdfg").isValid() ==false){
            QMessageBox msgBox;
              msgBox.setText("App lock is not configured.");
              msgBox.setIconPixmap(QPixmap(":/icons/information-line.png").scaled(42,42,Qt::KeepAspectRatio,Qt::SmoothTransformation));
              msgBox.setInformativeText("Do you want to setup App lock now ?");
              msgBox.setStandardButtons(QMessageBox::Cancel );
              QPushButton *setAppLock = new QPushButton("Yes",nullptr);
              msgBox.addButton(setAppLock,QMessageBox::NoRole);
              connect(setAppLock,&QPushButton::clicked,[=](){
                    init_lock();
              });
              msgBox.exec();
        }
    }
}

void MainWindow::showSettings()
{
    if(lockWidget && lockWidget->isLocked){
        QMessageBox::critical(this,QApplication::applicationName()+"| Error",
                              "UnLock Application to access Settings.");
        this->show();
        return;
    }

    if(webEngine == nullptr){
        QMessageBox::critical(this,QApplication::applicationName()+"| Error",
                              "Unable to initialize settings module.\nIs webengine initialized?");
        return;
    }
    if(!settingsWidget->isVisible())
    {
     this->updateSettingsUserAgentWidget();
     settingsWidget->refresh();
     settingsWidget->showNormal();
    }
}

void MainWindow::updateSettingsUserAgentWidget()
{
    settingsWidget->updateDefaultUAButton(this->webEngine->page()->profile()->httpUserAgent());
}

void MainWindow::askToReloadPage()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(QApplication::applicationName()+" | Action required");
    msgBox.setInformativeText("Page needs to be reloaded to continue.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    this->doAppReload();
}

void MainWindow::showAbout()
{
    About *about = new About(this);
    about->setWindowFlag(Qt::Dialog);
    about->adjustSize();
    about->setFixedSize(about->sizeHint());
    about->setAttribute(Qt::WA_DeleteOnClose);
    about->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());

    if(QSystemTrayIcon::isSystemTrayAvailable() && settings.value("closeButtonActionCombo",0).toInt() == 0){
        this->hide();
        event->ignore();
        notify(QApplication::applicationName(),"Application is minimized to system tray.");
        return;
    }
    event->accept();
    qApp->quit();
    QMainWindow::closeEvent(event);
}

void MainWindow::notify(QString title,QString message)
{
    if(settings.value("disableNotificationPopups",false).toBool() == true){
        return;
    }
    auto popup = new NotificationPopup(webEngine);
    connect(popup,&NotificationPopup::notification_clicked,[=](){
        if(windowState()==Qt::WindowMinimized || windowState()!=Qt::WindowActive){
            activateWindow();
            raise();
            show();
        }
    });
    popup->adjustSize();
    popup->present(title,message,QPixmap(":/icons/app/icon-64.png"));
}

void MainWindow::createActions()
{

    minimizeAction = new QAction(tr("Mi&nimize to tray"), this);
    connect(minimizeAction, &QAction::triggered, this, &QWidget::hide);
    addAction(minimizeAction);

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &QWidget::show);
    addAction(restoreAction);

    reloadAction = new QAction(tr("Re&load"), this);
    reloadAction->setShortcut(Qt::Key_F5);
    connect(reloadAction, &QAction::triggered, this, &MainWindow::doReload);
    addAction(reloadAction);

    lockAction = new QAction(tr("Lock"), this);
    lockAction->setShortcut(QKeySequence(Qt::Key_Control,Qt::Key_L));
    connect(lockAction, &QAction::triggered, this, &MainWindow::lockApp);
    addAction(lockAction);

    settingsAction = new QAction(tr("Settings"), this);
    connect(settingsAction, &QAction::triggered, this, &MainWindow::showSettings);


    aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAbout);

    quitAction = new QAction(tr("&Quit"), this);
    quitAction->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_Q));
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    addAction(quitAction);
}

void MainWindow::createStatusBar()
{
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->hide();
    this->statusBar = statusBar;
}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->setObjectName("trayIconMenu");
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(reloadAction);
    trayIconMenu->addAction(lockAction);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(trayIconRead, this);
    trayIcon->setContextMenu(trayIconMenu);
    connect(trayIconMenu,SIGNAL(aboutToShow()),this,SLOT(check_window_state()));

    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::messageClicked,
            this, &MainWindow::messageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::iconActivated);
}


void MainWindow::init_lock()
{
    if(lockWidget==nullptr){
        lockWidget = new Lock(this);
        lockWidget->setObjectName("lockWidget");
    }
        lockWidget->setWindowFlags(Qt::Widget);
        lockWidget->setStyleSheet("QWidget#login{background-color:palette(window)};"
                                  "QWidget#signup{background-color:palette(window)}");
        lockWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
        lockWidget->setGeometry(this->rect());

        connect(lockWidget,&Lock::passwordNotSet,[=]()
        {
            settings.setValue("lockscreen",false);
            settingsWidget->appLockSetChecked(false);
        });

        connect(lockWidget,&Lock::unLocked,[=]()
        {
            //unlock event
        });

        connect(lockWidget,&Lock::passwordSet,[=](){
            //enable disable lock screen
            if(settings.value("asdfg").isValid()){
                settingsWidget->setCurrentPasswordText("Current Password: <i>"
                        +QByteArray::fromBase64(settings.value("asdfg").toString().toUtf8())+"</i>");
            }else{
               settingsWidget->setCurrentPasswordText("Current Password: <i>Require setup</i>");
            }
            settingsWidget->appLockSetChecked(settings.value("lockscreen",false).toBool());
        });
        lockWidget->applyThemeQuirks();
        lockWidget->show();
        if(settings.value("asdfg").isValid() && settings.value("lockscreen").toBool()==true){
            lockWidget->lock_app();
        }
}

//check window state and set tray menus
void MainWindow::check_window_state()
{
    QObject *tray_icon_menu = this->findChild<QObject*>("trayIconMenu");
    if(tray_icon_menu != nullptr){
        if(this->isVisible()){
            ((QMenu*)(tray_icon_menu))->actions().at(0)->setDisabled(false);
            ((QMenu*)(tray_icon_menu))->actions().at(1)->setDisabled(true);
        }else{
            ((QMenu*)(tray_icon_menu))->actions().at(0)->setDisabled(true);
            ((QMenu*)(tray_icon_menu))->actions().at(1)->setDisabled(false);
        }
        if(lockWidget && lockWidget->isLocked){
            ((QMenu*)(tray_icon_menu))->actions().at(4)->setDisabled(true);
        }else{
            ((QMenu*)(tray_icon_menu))->actions().at(4)->setDisabled(false);
        }
    }
}

void MainWindow::init_globalWebProfile()
{

    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    auto* webSettings = profile->settings();
    webSettings->setAttribute(QWebEngineSettings::AutoLoadImages, true);
    webSettings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    webSettings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, true);
    webSettings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    webSettings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    webSettings->setAttribute(QWebEngineSettings::XSSAuditingEnabled, true);
    webSettings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    webSettings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, false);
    webSettings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled,true);
    webSettings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled ,true);
    webSettings->setAttribute(QWebEngineSettings::LinksIncludedInFocusChain, false);
    webSettings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    //webSettings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, true);

    //    QObject::connect(
    //        QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested,
    //        &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested);

    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture,
                                                        true);

}

void MainWindow::createWebEngine()
{
    init_globalWebProfile();

    QSizePolicy widgetSize;
    widgetSize.setHorizontalPolicy(QSizePolicy::Expanding);
    widgetSize.setVerticalPolicy(QSizePolicy::Expanding);
    widgetSize.setHorizontalStretch(1);
    widgetSize.setVerticalStretch(1);

    QWebEngineView *webEngine = new QWebEngineView(this);
    setCentralWidget(webEngine);
    webEngine->setSizePolicy(widgetSize);
    webEngine->show();

    this->webEngine = webEngine;


    connect(webEngine, &QWebEngineView::titleChanged,
            this, &MainWindow::handleWebViewTitleChanged);
    connect(webEngine, &QWebEngineView::loadStarted,
            this, &MainWindow::handleLoadStarted);
    connect(webEngine, &QWebEngineView::loadProgress,
            this, &MainWindow::handleLoadProgress);
    connect(webEngine, &QWebEngineView::loadFinished,
            this, &MainWindow::handleLoadFinished);
    connect(webEngine, &QWebEngineView::renderProcessTerminated,
            [this](QWebEnginePage::RenderProcessTerminationStatus termStatus, int statusCode) {
        QString status;
        switch (termStatus) {
        case QWebEnginePage::NormalTerminationStatus:
            status = tr("Render process normal exit");
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            status = tr("Render process abnormal exit");
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            status = tr("Render process crashed");
            break;
        case QWebEnginePage::KilledTerminationStatus:
            status = tr("Render process killed");
            break;
        }
        QMessageBox::StandardButton btn = QMessageBox::question(window(), status,
                                                   tr("Render process exited with code: %1\n"
                                                      "Do you want to reload the page ?").arg(statusCode));
        if (btn == QMessageBox::Yes)
            QTimer::singleShot(0, [this] { this->webEngine->reload(); });
    });

    QWebEngineCookieStore *browser_cookie_store = webEngine->page()->profile()->cookieStore();
    connect( browser_cookie_store, &QWebEngineCookieStore::cookieAdded, this, &MainWindow::handleCookieAdded );

    createWebPage(false);
}

void MainWindow::createWebPage(bool offTheRecord)
{
    if (offTheRecord && !m_otrProfile) {
        m_otrProfile.reset(new QWebEngineProfile);
      }

    auto profile = offTheRecord ? m_otrProfile.get() : QWebEngineProfile::defaultProfile();

    profile->setHttpUserAgent(settings.value("useragent",defaultUserAgentStr).toString());

        auto popup = new NotificationPopup(webEngine);
        connect(popup,&NotificationPopup::notification_clicked,[=](){
            if(windowState()==Qt::WindowMinimized || windowState()!=Qt::WindowActive){
                activateWindow();
                raise();
                show();
            }
        });
        profile->setNotificationPresenter([=] (std::unique_ptr<QWebEngineNotification> notification)
        {
            if(settings.value("disableNotificationPopups",false).toBool() == true){
                return;
            }
            popup->present(notification);
        });

    QWebEnginePage *page = new WebEnginePage(profile,webEngine);
    if(settings.value("windowTheme","light").toString() == "dark"){
        page->setBackgroundColor(QColor("#131C21")); //whatsapp dark bg color
    }
    webEngine->setPage(page);
    //page should be set parent of profile to prevent
    //Release of profile requested but WebEnginePage still not deleted. Expect troubles !
    profile->setParent(page);
    profile->setSpellCheckEnabled(true);
    //RequestInterceptor *interceptor = new RequestInterceptor(profile);
    //profile->setUrlRequestInterceptor(interceptor);
    page->setUrl(QUrl("https://web.whatsapp.com/"));

    connect(profile, &QWebEngineProfile::downloadRequested,
        &m_downloadManagerWidget, &DownloadManagerWidget::downloadRequested);

    connect(webEngine->page(), SIGNAL(fullScreenRequested(QWebEngineFullScreenRequest)),
                this, SLOT(fullScreenRequested(QWebEngineFullScreenRequest)));
}

void MainWindow::fullScreenRequested(QWebEngineFullScreenRequest request)
{
    if (request.toggleOn())
    {
            webEngine->showFullScreen();
            this->showFullScreen();
            request.accept();
    } else {
            webEngine->showNormal();
            this->showNormal();
            request.accept();
    }
}

void MainWindow::handleWebViewTitleChanged(QString title)
{
    setWindowTitle(title);

    if (notificationsTitleRegExp.exactMatch(title))
    {
        trayIcon->setIcon(trayIconUnread);
        setWindowIcon(trayIconUnread);
    }
    else
    {
        trayIcon->setIcon(trayIconRead);
        setWindowIcon(trayIconRead);
    }
}

void MainWindow::handleLoadStarted()
{
    statusBar->show();
}

void MainWindow::handleLoadFinished(bool loaded)
{
    statusBar->hide();
    if(loaded){
        updatePageTheme();
    }
}

void MainWindow::handleLoadProgress(int progress)
{
    statusBar->showMessage("Loading "+QString::number(progress)+"%");
    if (progress >= 50)
    {
        statusBar->hide();
    }
}

//unused direct method to download file without having entry in download manager
void MainWindow::handleDownloadRequested(QWebEngineDownloadItem *download)
{
    QFileDialog dialog;
    dialog.setParent(this);
    dialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    dialog.setFileMode(QFileDialog::FileMode::AnyFile);
    QString suggestedFileName = QUrl(download->path()).fileName();
    dialog.selectFile(suggestedFileName);

    if (dialog.exec() && dialog.selectedFiles().size() > 0)
    {
        download->setPath(dialog.selectedFiles().at(0));
        download->accept();
    }
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    Q_UNUSED(reason);
    if (isVisible()) {
        hide();
    } else {
        showNormal();
    }
}

void MainWindow::messageClicked()
{
    if (isVisible()) {
        hide();
    } else {
        showNormal();
    }
}

void MainWindow::doAppReload()
{
    if(this->webEngine->page()){
        this->webEngine->page()->disconnect();
    }
    createWebPage(false);
}

void MainWindow::doReload()
{
    this->webEngine->triggerPageAction(QWebEnginePage::ReloadAndBypassCache, false);
}

void MainWindow::toggleMute(const bool checked)
{
    this->webEngine->page()->setAudioMuted(checked);
}

// get value of page theme when page is loaded
QString MainWindow::getPageTheme()
{
    static QString theme = "web"; //implies light
    if(webEngine && webEngine->page())
    {
        webEngine->page()->runJavaScript(
            "document.querySelector('body').className;",
            [](const QVariant &result){
                theme = result.toString();
            }
        );
    }
    return theme;
}
