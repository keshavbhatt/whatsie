#include "mainwindow.h"


extern QString defaultUserAgentStr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      notificationsTitleRegExp("^\\([1-9]\\d*\\).*"),
      trayIconRead(":/icons/app/whatsapp.svg"),
      trayIconUnread(":/icons/app/whatsapp-message.svg")
{
    setWindowTitle(QApplication::applicationName());
    setWindowIcon(QIcon(":/icons/app/icon-256.png"));
    setMinimumWidth(800);
    setMinimumHeight(600);
    readSettings();

    createActions();
    createStatusBar();
    createTrayIcon();
    createWebEngine();

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

void MainWindow::updateWindowTheme()
{
    if(settings.value("windowTheme","light").toString() == "dark")
    {
        //TODO make dark palette match whatsapp dark theme
        qApp->setStyle(QStyleFactory::create("fusion"));
        QPalette palette;
        palette.setColor(QPalette::Window,QColor("#131C21")); //whatsapp dark color
        palette.setColor(QPalette::WindowText,Qt::white);
        palette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
        palette.setColor(QPalette::Base,QColor(42,42,42));
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
}

void MainWindow::handleCookieAdded(const QNetworkCookie &cookie)
{
  qDebug() << cookie.toRawForm() << "\n\n\n";
}

void MainWindow::readSettings()
{
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::init_settingWidget()
{
    if(settingsWidget == nullptr)
    {
        settingsWidget = new SettingsWidget(this,webEngine->page()->profile()->cachePath()
                                            ,webEngine->page()->profile()->persistentStoragePath());
        settingsWidget->setWindowTitle(QApplication::applicationName()+" | Settings");
        settingsWidget->setWindowFlags(Qt::Dialog);

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
    }
}

void MainWindow::showSettings()
{
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
    QDialog *aboutDialog = new QDialog(this,Qt::Dialog);
    aboutDialog->setWindowModality(Qt::WindowModal);
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *message = new QLabel(aboutDialog);
    layout->addWidget(message);
    connect(message,&QLabel::linkActivated,[=](const QString linkStr){
        if(linkStr.contains("about_qt")){
            qApp->aboutQt();
        }else{
            QDesktopServices::openUrl(QUrl(linkStr));
        }
    });
    aboutDialog->setLayout(layout);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose,true);
    aboutDialog->show();

    QString mes =
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><img src=':/icons/app/icon-64.png' /></p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Designed and Developed</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>by <span style=' font-weight:600;'>Keshav Bhatt</span> &lt;keshavnrj@gmail.com&gt;</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Website: https://ktechpit.com</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Runtime: <a href='http://about_qt'>Qt Toolkit</a></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Version: "+QApplication::applicationVersion()+"</p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><a href='https://snapcraft.io/search?q=keshavnrj'>More Apps</p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>";

    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    message->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(1000);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InCurve);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    message->setText(mes);
    message->show();
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
    connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);
    addAction(restoreAction);

    reloadAction = new QAction(tr("Re&load"), this);
    reloadAction->setShortcut(Qt::Key_F5);
    connect(reloadAction, &QAction::triggered, this, &MainWindow::doReload);
    addAction(reloadAction);

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
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(reloadAction);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(trayIconRead, this);
    trayIcon->setContextMenu(trayIconMenu);

    trayIcon->show();

    connect(trayIcon, &QSystemTrayIcon::messageClicked,
            this, &MainWindow::messageClicked);
    connect(trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::iconActivated);
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
        page->setBackgroundColor(QColor("#F0F0F0")); //whatsapp bg color
    }
    webEngine->setPage(page);
    //page should be set parent of profile to prevent
    //Release of profile requested but WebEnginePage still not deleted. Expect troubles !
    profile->setParent(page);
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
    if(loaded){
        updatePageTheme();
    }
}

void MainWindow::handleLoadProgress(int progress)
{
    if (progress >= 100)
    {
        statusBar->hide();
    }
    else
    {
        statusBar->showMessage("Loading "+QString::number(progress)+"%");
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
