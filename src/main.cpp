#include <QApplication>
#include <QtWidgets>
#include <QtWebEngine>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QSettings>
#include <QDebug>

#include <mainwindow.h>
#include "common.h"

extern QString defaultUserAgentStr;

int main(int argc, char *argv[])
{

    //argv[argc++] = "--single-process";

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/app/icon-256.png"));

    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    static const char ENV_VAR_QT_DEVICE_PIXEL_RATIO[] = "QT_DEVICE_PIXEL_RATIO";
    if (!qEnvironmentVariableIsSet(ENV_VAR_QT_DEVICE_PIXEL_RATIO)
            && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
            && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
            && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }

    QApplication::setApplicationName("WhatSie");
    QApplication::setOrganizationName("org.keshavnrj.ubuntu");
    QApplication::setApplicationVersion(VERSIONSTR);

    QtWebEngine::initialize();

    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    MainWindow window;
    window.show();

    return app.exec();
}
