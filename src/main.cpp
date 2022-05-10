#include <QApplication>
#include <QDebug>
#include <QSettings>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QtWebEngine>
#include <QtWidgets>

#include "common.h"
#include "mainwindow.h"
#include <singleapplication.h>

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

  SingleApplication app(argc, argv, true);
  app.setQuitOnLastWindowClosed(false);
  app.setWindowIcon(QIcon(":/icons/app/icon-128.png"));
  QApplication::setApplicationName("WhatSie");
  QApplication::setOrganizationName("org.keshavnrj.ubuntu");
  QApplication::setApplicationVersion(VERSIONSTR);

  // if secondary instance is invoked
  if (app.isSecondary()) {
    app.sendMessage(app.arguments().join(' ').toUtf8());
    qInfo() << QApplication::applicationName() +
                   " is already running with PID:" +
                   QString::number(app.primaryPid()) + "; by USER: "
            << app.primaryUser();
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

  // else
  QObject::connect(
      &app, &SingleApplication::receivedMessage,
      [&window](int instanceId, QByteArray message) {
        qInfo() << "Another instance with PID: " + QString::number(instanceId) +
                       ", sent argument: " + message;
        QString messageStr = QTextCodec::codecForMib(106)->toUnicode(message);
        if (messageStr.contains("whatsapp://whatsie", Qt::CaseInsensitive)) {
          window.show();
          return;
        } else if (messageStr.contains("whatsapp://", Qt::CaseInsensitive)) {
          QString urlStr =
              "whatsapp://" + messageStr.split("whatsapp://").last();
          window.loadAppWithArgument(urlStr);
        } else {
          window.alreadyRunning(true);
        }
      });

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
  } else {
    window.show();
  }

  return app.exec();
}
