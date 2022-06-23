#-------------------------------------------------
#
# Project created by QtCreator Wed Apr  7 02:25:15 IST 2021
#
#-------------------------------------------------

QT += core gui webengine webenginewidgets xml positioning

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

!qtConfig(webengine-spellchecker) {
    error("Qt WebEngine compiled without spellchecker support, this example will not work.")
}

qtConfig(webengine-native-spellchecker) {
    error("Spellcheck example can not be built when using native OS dictionaries.")
}

TARGET = whatsie
TEMPLATE = app
LIBS += -L/usr/X11/lib -lX11

win32{
    LIBS += User32.Lib
}

include(singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# No debug output in release mode
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Define git info
GIT_HASH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD)\\\""
GIT_BRANCH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --abbrev-ref HEAD)\\\""
BUILD_TIMESTAMP="\\\"$$system(date -u +\""%Y-%m-%dT%H:%M:%SUTC\"")\\\""
DEFINES += GIT_HASH=$$GIT_HASH GIT_BRANCH=$$GIT_BRANCH BUILD_TIMESTAMP=$$BUILD_TIMESTAMP

# Set program version
VERSION = 4.4
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        SunClock.cpp \
        about.cpp \
        automatictheme.cpp \
        dictionaries.cpp \
        downloadmanagerwidget.cpp \
        downloadwidget.cpp \
        elidedlabel.cpp \
        lock.cpp \
        main.cpp \
        mainwindow.cpp \
        permissiondialog.cpp \
        rateapp.cpp \
        settingswidget.cpp \
        utils.cpp \
        webenginepage.cpp \
        webview.cpp \
        widgets/scrolltext/scrolltext.cpp

RESOURCES += \
        icons.qrc

HEADERS += \
    SunClock.hpp \
    about.h \
    autolockeventfilter.h \
    automatictheme.h \
    common.h \
    dictionaries.h \
    downloadmanagerwidget.h \
    downloadwidget.h \
    elidedlabel.h \
    lock.h \
    mainwindow.h \
    notificationpopup.h \
    permissiondialog.h \
    rateapp.h \
    requestinterceptor.h \
    settingswidget.h \
    utils.h \
    webenginepage.h \
    webview.h \
    widgets/scrolltext/scrolltext.h


FORMS += \
    about.ui \
    automatictheme.ui \
    certificateerrordialog.ui \
    downloadmanagerwidget.ui \
    downloadwidget.ui \
    lock.ui \
    passworddialog.ui \
    permissiondialog.ui \
    rateapp.ui \
    settingswidget.ui

qtPrepareTool(CONVERT_TOOL, qwebengine_convert_dict)

DICTIONARIES_DIR = qtwebengine_dictionaries

dict.files = $$files($$PWD/dictionaries/*.dic, true)

dictoolbuild.input = dict.files
dictoolbuild.output = $${DICTIONARIES_DIR}/${QMAKE_FILE_BASE}.bdic
dictoolbuild.depends = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.aff
dictoolbuild.commands = $${CONVERT_TOOL} ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
dictoolbuild.name = Build ${QMAKE_FILE_IN_BASE}
dictoolbuild.CONFIG = no_link target_predeps
QMAKE_EXTRA_COMPILERS += dictoolbuild


# Default rules for deployment.
isEmpty(PREFIX){
 PREFIX = /usr
}

BINDIR  = $$PREFIX/bin
DATADIR = $$PREFIX/share

target.path = $$BINDIR

dicts.files = $${DICTIONARIES_DIR}/
dicts.path  = $$DATADIR/org.keshavnrj.ubuntu/WhatSie/

icon.files = icons/whatsie.png
icon.path  = $$DATADIR/icons/hicolor/512x512/apps/

desktop.files = whatsie.desktop
desktop.path  = $$DATADIR/applications/

INSTALLS += target dicts icon desktop

