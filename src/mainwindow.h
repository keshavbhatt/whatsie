#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QRadioButton>
#include <QStatusBar>
#include <QStyle>
#include <QStyleFactory>
#include <QSystemTrayIcon>
#include <QWebEngineContextMenuData>
#include <QWebEngineCookieStore>
#include <QWebEngineFullScreenRequest>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>

#include "about.h"
#include "autolockeventfilter.h"
#include "dictionaries.h"
#include "downloadmanagerwidget.h"
#include "lock.h"
#include "notificationpopup.h"
#include "rateapp.h"
#include "requestinterceptor.h"
#include "settingswidget.h"
#include "theme.h"
#include "webenginepage.h"
#include "webview.h"

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void loadSchemaUrl(const QString &arg);
  void alreadyRunning(bool notify = false);
  void runMinimized();
  void showNotification(QString title, QString message);
  void doReload(bool byPassCache = false, bool isAskedByCLI = false,
                bool byLoadingQuirk = false);

public slots:
  void updateWindowTheme();
  void updatePageTheme();
  void handleWebViewTitleChanged(const QString &title);
  void handleLoadFinished(bool loaded);
  void handleDownloadRequested(QWebEngineDownloadItem *download);
  void showSettings(bool isAskedByCLI = false);
  void showAbout();
  void lockApp();
  void toggleTheme();
  void newChat();

protected slots:
  void closeEvent(QCloseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void changeEvent(QEvent *e) override;

private:
  const QIcon getTrayIcon(const int &notificationCount) const;
  void createActions();
  void createTrayIcon();
  void createWebEngine();
  QString getPageTheme() const;
  void handleCookieAdded(const QNetworkCookie &cookie);
  void doAppReload();
  void askToReloadPage();
  void updateSettingsUserAgentWidget();
  void createWebPage(bool offTheRecord = false);
  void initSettingWidget();
  void initGlobalWebProfile();
  void tryLock();
  void checkLoadedCorrectly();
  void loadingQuirk(const QString &test);
  void setNotificationPresenter(QWebEngineProfile *profile);
  void initRateWidget();
  void handleZoomOnWindowStateChange(const QWindowStateChangeEvent *ev);
  void handleZoom();
  void forceLogOut();
  bool isLoggedIn();
  void tryLogOut();
  void initAutoLock();
  void triggerNewChat(const QString &phone, const QString &text);
  void restoreMainWindow();

  QIcon m_trayIconNormal;
  QRegularExpression m_notificationsTitleRegExp;
  QRegularExpression m_unreadMessageCountRegExp;
  DownloadManagerWidget m_downloadManagerWidget;
  QScopedPointer<QWebEngineProfile> m_otrProfile;
  int m_correctlyLoadedRetries = 4;
  QStringList m_dictionaries;

  QAction *m_reloadAction = nullptr;
  QAction *m_minimizeAction = nullptr;
  QAction *m_restoreAction = nullptr;
  QAction *m_aboutAction = nullptr;
  QAction *m_settingsAction = nullptr;
  QAction *m_toggleThemeAction = nullptr;
  QAction *m_quitAction = nullptr;
  QAction *m_lockAction = nullptr;
  QAction *m_fullscreenAction = nullptr;
  QAction *m_openUrlAction = nullptr;

  QMenu *m_trayIconMenu = nullptr;
  QSystemTrayIcon *m_systemTrayIcon = nullptr;
  QWebEngineView *m_webEngine = nullptr;
  SettingsWidget *m_settingsWidget = nullptr;
  Lock *m_lockWidget = nullptr;
  AutoLockEventFilter *m_autoLockEventFilter = nullptr;
  Qt::WindowStates windowStateBeforeFullScreen;

  QString userDesktopEnvironment = Utils::detectDesktopEnvironment();

  void notificationClicked();
  NotificationPopup *m_webengine_notifier_popup = nullptr;
private slots:
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void toggleMute(const bool &checked);
  void fullScreenRequested(QWebEngineFullScreenRequest request);
  void checkWindowState();
  void initLock();
  void quitApp();
  void changeLockPassword();
  void appAutoLockChanged();
};

#endif // MAINWINDOW_H
