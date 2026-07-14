#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>

#include "autolockeventfilter.h"
#include "downloadmanagerwidget.h"
#include "lock.h"
#include "notificationpopup.h"
#include "settingswidget.h"
#include "webenginepage.h"
#ifdef Q_OS_LINUX
#include <libnotify-qt.h>
#endif

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
  void doAppReload();
  void askToReloadPage();
  void updateSettingsUserAgentWidget();
  void createWebPage(bool offTheRecord = false);
  void initSettingWidget();
  void tryLock();
  void checkLoadedCorrectly();
  void loadingQuirk(const QString &test);
  void setNotificationPresenter(QWebEngineProfile *profile);
#ifdef Q_OS_LINUX
  Notification::EventPtr notify(const QString& title, const QString& body, qint32 timeout);
#endif
  void initRateWidget();
  void handleZoomOnWindowStateChange(const QWindowStateChangeEvent *ev);
  void handleZoom();
  void forceLogOut();
  bool isLoggedIn();
  void tryLogOut();
  void initAutoLock();
  void triggerNewChat(const QString &phone, const QString &text);
  void restoreMainWindow();

#ifdef Q_OS_LINUX
  Notification::Manager m_notifier;
#else
  // Routes QSystemTrayIcon::messageClicked to the most recent notification
  QMetaObject::Connection m_trayNotificationClickConnection;
#endif
  QIcon m_trayIconNormal;
  QRegularExpression m_notificationsTitleRegExp;
  QRegularExpression m_unreadMessageCountRegExp;
  DownloadManagerWidget m_downloadManagerWidget;
  QScopedPointer<QWebEngineProfile> m_otrProfile;
  int m_correctlyLoadedRetries = 4;
  // Set while quitApp() runs so closeEvent() does not turn an intentional
  // quit into minimize-to-tray (Qt 6.3+ quit() closes windows first and a
  // vetoed close cancels the quit).
  bool m_isQuitting = false;

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
