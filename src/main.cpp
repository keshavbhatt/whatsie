#include <QApplication>
#include <QtWidgets>
#include <QtWebEngine>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QSettings>
#include <QDebug>

#include "mainwindow.h"

#include "rungaurd.h"
#include "common.h"


int main(int argc, char *argv[])
{
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

    QString appname = QApplication::applicationName();

    //allow multiple instances in debug builds
    #ifndef QT_DEBUG
        RunGuard guard("org.keshavnrj.ubuntu."+appname);
        if ( !guard.tryToRun() ){
            QMessageBox::critical(0, appname,"An instance of "+appname+" is already running.");
            return 0;
        }
    #endif


    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, true);
    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

    MainWindow window;

    QStringList argsList = app.arguments();
    qWarning()<<"Launching with argument"<<argsList;
    foreach (QString argStr, argsList) {
        if(argStr.contains("http")){
            qWarning()<<"Link passed as argument";
            window.loadAppWithArgument(argsList.last());
        }
    }
    window.show();

    return app.exec();
}
