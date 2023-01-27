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
#include <QRegExp>
#include <QSettings>
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
#include "webenginepage.h"
#include "webview.h"

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void updateWindowTheme();
  void updatePageTheme();
  void handleWebViewTitleChanged(QString title);
  void handleLoadFinished(bool loaded);
  void handleDownloadRequested(QWebEngineDownloadItem *download);
  void loadSchemaUrl(const QString &arg);
  void showSettings(bool isAskedByCLI = false);
  void showAbout();
  void lockApp();
  void runMinimized();
  void alreadyRunning(bool notify = false);
  void notify(QString title, QString message);
  void toggleTheme();
  void doReload(bool byPassCache = false, bool isAskedByCLI = false,
                bool byLoadingQuirk = false);
  void newChat();
protected slots:
  void closeEvent(QCloseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void changeEvent(QEvent *e) override;

private:
  const QIcon getTrayIcon(const int &notificationCount) const;
  QPalette lightPalette, darkPalette;
  void createActions();
  void createTrayIcon();
  void createWebEngine();

  QSettings settings;
  QRegExp notificationsTitleRegExp;
  QIcon trayIconNormal;
  QRegExp unreadMessageCountRegExp;

  QAction *reloadAction;
  QAction *minimizeAction;
  QAction *restoreAction;
  QAction *aboutAction;
  QAction *settingsAction;
  QAction *toggleThemeAction;
  QAction *quitAction;
  QAction *lockAction;
  QAction *fullscreenAction;
  QAction *openUrlAction;

  QMenu *trayIconMenu;
  QSystemTrayIcon *trayIcon;
  QWebEngineView *webEngine;
  SettingsWidget *settingsWidget = nullptr;
  DownloadManagerWidget downloadManagerWidget;
  QScopedPointer<QWebEngineProfile> otrProfile;
  Lock *lockWidget = nullptr;
  int correctlyLoaderRetries = 4;
  QStringList dictionaries;
  AutoLockEventFilter *autoLockEventFilter = nullptr;

private slots:

  QString getPageTheme();
  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void messageClicked();
  void handleCookieAdded(const QNetworkCookie &cookie);
  void toggleMute(const bool &checked);
  void doAppReload();
  void askToReloadPage();
  void updateSettingsUserAgentWidget();
  void fullScreenRequested(QWebEngineFullScreenRequest request);
  void createWebPage(bool offTheRecord = false);
  void initSettingWidget();
  void initGlobalWebProfile();
  void checkWindowState();
  void initLock();
  void tryLock();
  void checkLoadedCorrectly();
  void loadingQuirk(QString test);
  void setNotificationPresenter(QWebEngineProfile *profile);
  void quitApp();
  void initRateWidget();
  void initThemes();
  void handleZoomOnWindowStateChange(const QWindowStateChangeEvent *ev);
  void handleZoom();
  void changeLockPassword();
  void forceLogOut();
  void tryLogOut();
  bool isLoggedIn();
  void initAutoLock();
  void appAutoLockChanged();
  void triggerNewChat(QString phone, QString text);
  void restoreMainWindow();
};

#endif // MAINWINDOW_H
