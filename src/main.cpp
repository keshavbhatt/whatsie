#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QtWebEngine>
#include <QtWidgets>

#include "mainwindow.h"

#include "common.h"
#include "rungaurd.h"

int main(int argc, char *argv[]) {

  QStringList args;
  for (int i = 0; i < argc; i++)
    args << QString(argv[i]);

  if (args.contains("-v") || args.contains("--version")) {
    qInfo() << QString("version: %1, branch: %2, commit: %3, built_at: %4")
                    .arg(VERSIONSTR, GIT_BRANCH, GIT_HASH, BUILD_TIMESTAMP);
    return 0;
  }

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  static const char ENV_VAR_QT_DEVICE_PIXEL_RATIO[] = "QT_DEVICE_PIXEL_RATIO";
  if (!qEnvironmentVariableIsSet(ENV_VAR_QT_DEVICE_PIXEL_RATIO) &&
      !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR") &&
      !qEnvironmentVariableIsSet("QT_SCALE_FACTOR") &&
      !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  }

#ifdef QT_DEBUG
  qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--remote-debugging-port=9421");
#else
  qputenv("QTWEBENGINE_CHROMIUM_FLAGS", "--disable-logging --single-process");
#endif

  QApplication app(argc, argv);
  app.setQuitOnLastWindowClosed(false);
  app.setWindowIcon(QIcon(":/icons/app/icon-128.png"));
  QApplication::setApplicationName("WhatSie");
  QApplication::setOrganizationName("org.keshavnrj.ubuntu");
  QApplication::setApplicationVersion(VERSIONSTR);
  QString appname = QApplication::applicationName();

  RunGuard guard("org.keshavnrj.ubuntu." + appname);
  if (!guard.tryToRun()) {
    QMessageBox::critical(0, appname,
                          "An instance of " + appname + " is already running.");
    return 0;
  }

  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::PluginsEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::DnsPrefetchEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::FullScreenSupportEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::JavascriptCanAccessClipboard, true);

  MainWindow window;

  QStringList argsList = app.arguments();
  foreach (QString argStr, argsList) {
    if (argStr.contains("whatsapp://")) {
      window.loadAppWithArgument(argStr);
    }
  }
  QSettings settings;
  if (QSystemTrayIcon::isSystemTrayAvailable() &&
      settings.value("startMinimized", false).toBool()) {
      window.runMinimized();
  }else{
      window.show();
  }

  return app.exec();
}
