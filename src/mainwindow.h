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

#include "lock.h"
#include "notificationpopup.h"
#include "requestinterceptor.h"
#include "settingswidget.h"
#include "webenginepage.h"

#include "about.h"
#include "dictionaries.h"
#include "downloadmanagerwidget.h"
#include "rateapp.h"
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
  void loadAppWithArgument(const QString &arg);
  void runMinimized();
protected slots:
  void closeEvent(QCloseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  QPalette lightPalette;
  void createActions();
  void createTrayIcon();
  void createWebEngine();

  QSettings settings;
  QRegExp notificationsTitleRegExp;
  QIcon trayIconRead;
  QIcon trayIconUnread;

  QAction *reloadAction;
  QAction *minimizeAction;
  QAction *restoreAction;
  QAction *aboutAction;
  QAction *settingsAction;
  QAction *quitAction;
  QAction *lockAction;
  QAction *fullscreenAction;
  QAction *openUrlAction;

  QMenu *trayIconMenu;
  QSystemTrayIcon *trayIcon;
  QWebEngineView *webEngine;
  SettingsWidget *settingsWidget = nullptr;
  DownloadManagerWidget m_downloadManagerWidget;
  QScopedPointer<QWebEngineProfile> m_otrProfile;
  Lock *lockWidget = nullptr;
  int correctlyLoaderRetries = 4;
  QStringList m_dictionaries;

private slots:

  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void messageClicked();
  void doReload();
  void showAbout();
  void notify(QString title, QString message);
  void showSettings();
  void handleCookieAdded(const QNetworkCookie &cookie);

  QString getPageTheme();
  void toggleMute(const bool &checked);
  void doAppReload();
  void askToReloadPage();
  void updateSettingsUserAgentWidget();
  void fullScreenRequested(QWebEngineFullScreenRequest request);

  void createWebPage(bool offTheRecord = false);
  void init_settingWidget();
  void init_globalWebProfile();
  void check_window_state();
  void init_lock();
  void lockApp();

  void checkLoadedCorrectly();
  void loadingQuirk(QString test);
  void setNotificationPresenter(QWebEngineProfile *profile);
  void newChat();
  bool isPhoneNumber(const QString &phoneNumber);
  void quitApp();
};

#endif // MAINWINDOW_H
