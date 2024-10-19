#include <QApplication>
#include <QDebug>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QtWebEngine>
#include <QtWidgets>

#include "common.h"
#include "def.h"
#include "mainwindow.h"
#include "settingsmanager.h"
#include <singleapplication.h>

int main(int argc, char *argv[]) {

  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

#ifdef QT_DEBUG
  qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
          "--remote-debugging-port=9421 --ignore-gpu-blocklist --no-sandbox "
          "--single-process --disable-extensions");
#else
  qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
          "--disable-logging --ignore-gpu-blocklist --no-sandbox "
          "--single-process --disable-extensions");
#endif

  SingleApplication instance(argc, argv, true);
  instance.setQuitOnLastWindowClosed(false);
  instance.setWindowIcon(QIcon(":/icons/app/icon-64.png"));
  QApplication::setApplicationName("WhatSie");
  QApplication::setDesktopFileName("com.ktechpit.whatsie");
  QApplication::setOrganizationDomain("com.ktechpit");
  QApplication::setOrganizationName("org.keshavnrj.ubuntu");
  QApplication::setApplicationVersion(VERSIONSTR);

  qputenv("QTWEBENGINE_DICTIONARIES_PATH",
          Dictionaries::GetDictionaryPath().toUtf8().constData());

  QCommandLineParser parser;
  parser.setApplicationDescription(
      QObject::tr("Feature rich WhatsApp web client based on Qt WebEngine"));

  QList<QCommandLineOption> secondaryInstanceCLIOptions;

  QCommandLineOption showCLIHelpOption(
      QStringList() << "h"
                    << "help",
      QObject::tr("Displays help on commandline options"));

  QCommandLineOption openSettingsOption(
      QStringList() << "s"
                    << "open-settings",
      QObject::tr("Opens Settings dialog in a running instance of ") +
          QApplication::applicationName());

  QCommandLineOption lockAppOption(QStringList() << "l"
                                                 << "lock-app",
                                   QObject::tr("Locks a running instance of ") +
                                       QApplication::applicationName());

  QCommandLineOption openAboutOption(
      QStringList() << "i"
                    << "open-about",
      QObject::tr("Opens About dialog in a running instance of ") +
          QApplication::applicationName());

  QCommandLineOption toggleThemeOption(
      QStringList() << "t"
                    << "toggle-theme",
      QObject::tr(
          "Toggle between dark & light theme in a running instance of ") +
          QApplication::applicationName());

  QCommandLineOption reloadAppOption(
      QStringList() << "r"
                    << "reload-app",
      QObject::tr("Reload the app in a running instance of ") +
          QApplication::applicationName());

  QCommandLineOption newChatOption(
      QStringList() << "n"
                    << "new-chat",
      QObject::tr("Open new chat prompt in a running instance of ") +
          QApplication::applicationName());

  QCommandLineOption buildInfoOption(QStringList() << "b"
                                                   << "build-info",
                                     "Shows detailed current build infomation");

  QCommandLineOption showAppWindowOption(
      QStringList() << "w"
                    << "show-window",
      QObject::tr("Show main window of running instance of ") +
          QApplication::applicationName());

  parser.addOption(showCLIHelpOption);
  parser.addVersionOption();
  parser.addOption(buildInfoOption);
  parser.addOption(showAppWindowOption);
  parser.addOption(openSettingsOption);
  parser.addOption(lockAppOption);
  parser.addOption(openAboutOption);
  parser.addOption(toggleThemeOption);
  parser.addOption(reloadAppOption);
  parser.addOption(newChatOption);

  secondaryInstanceCLIOptions << showAppWindowOption << openSettingsOption
                              << lockAppOption << openAboutOption
                              << toggleThemeOption << reloadAppOption
                              << newChatOption;

  parser.process(instance);

  if (parser.isSet(showCLIHelpOption)) {
    parser.showHelp();
  }

  if (parser.isSet(buildInfoOption)) {

    qInfo().noquote()
        << parser.applicationDescription() << "\n"
        << QStringLiteral("version: %1, branch: %2, commit: %3, built_at: %4")
               .arg(VERSIONSTR, GIT_BRANCH, GIT_HASH, BUILD_TIMESTAMP);
    return 0;
  }

  // if secondary instance is invoked
  if (instance.isSecondary()) {
    instance.sendMessage(instance.arguments().join(' ').toUtf8());
    qInfo().noquote() << QApplication::applicationName() +
                             " is already running with PID: " +
                             QString::number(instance.primaryPid()) +
                             " by USER:"
                      << instance.primaryUser();
    return 0;
  }

  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::DnsPrefetchEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::FullScreenSupportEnabled, true);
  QWebEngineSettings::defaultSettings()->setAttribute(
      QWebEngineSettings::JavascriptCanAccessClipboard, true);

  MainWindow whatsie;

  // else
  QObject::connect(
      &instance, &SingleApplication::receivedMessage, &whatsie,
      [&whatsie, &secondaryInstanceCLIOptions](int instanceId,
                                               QByteArray message) {
        qInfo().noquote() << "Another instance with PID: " +
                                 QString::number(instanceId) +
                                 ", sent argument: " + message;
        QString messageStr = QTextCodec::codecForMib(106)->toUnicode(message);

        QCommandLineParser p;
        p.addOptions(secondaryInstanceCLIOptions);
        p.parse(QStringList(messageStr.split(" ")));

        if (p.isSet("s")) {
          qInfo() << "cmd:"
                  << "OpenAppSettings";
          whatsie.alreadyRunning();
          whatsie.showSettings(true);
          return;
        }

        if (p.isSet("l")) {
          qInfo() << "cmd:"
                  << "LockApp";
          whatsie.alreadyRunning();
          if (!SettingsManager::instance()
                   .settings()
                   .value("asdfg")
                   .isValid()) {
            whatsie.showNotification(
                QApplication::applicationName(),
                QObject::tr("App lock is not configured, \n"
                            "Please setup the password in the Settings "
                            "first."));
          } else {
            whatsie.lockApp();
          }
          return;
        }

        if (p.isSet("i")) {
          qInfo() << "cmd:"
                  << "OpenAppAbout";
          whatsie.alreadyRunning();
          whatsie.showAbout();
          return;
        }

        if (p.isSet("t")) {
          qInfo() << "cmd:"
                  << "ToggleAppTheme";
          whatsie.alreadyRunning();
          whatsie.toggleTheme();
          return;
        }

        if (p.isSet("r")) {
          qInfo() << "cmd:"
                  << "ReloadApp";
          whatsie.alreadyRunning();
          whatsie.doReload(false, true);
          return;
        }

        if (p.isSet("n")) {
          qInfo() << "cmd:"
                  << "OpenNewChatPrompt";
          whatsie.alreadyRunning();
          whatsie.newChat(); // TODO: invetigate the crash
          return;
        }

        if (p.isSet("w")) {
          qInfo() << "cmd:"
                  << "ShowAppWindow";
          whatsie.alreadyRunning();
          whatsie.show();
          return;
        }

        if (messageStr.contains("whatsapp://", Qt::CaseInsensitive)) {
          QString urlStr =
              "whatsapp://" + messageStr.split("whatsapp://").last();
          qInfo() << "cmd:"
                  << "x-schema-handler";
          whatsie.loadSchemaUrl(urlStr);
        } else {
          whatsie.alreadyRunning(true);
        }
      });

  foreach (QString argStr, instance.arguments()) {
    if (argStr.contains("whatsapp://")) {
      qInfo() << "cmd:"
              << "x-schema-handler";
      whatsie.loadSchemaUrl(argStr);
    }
  }

  if (QSystemTrayIcon::isSystemTrayAvailable() &&
      SettingsManager::instance()
          .settings()
          .value("startMinimized", false)
          .toBool()) {
    whatsie.runMinimized();
  } else {
    whatsie.show();
  }

  return instance.exec();
}
