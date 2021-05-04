#-------------------------------------------------
#
# Project created by QtCreator Wed Apr  7 02:25:15 IST 2021
#
#-------------------------------------------------

QT += core gui webengine webenginewidgets webchannel xml positioning

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

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# No debug output in release mode
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



# Set program version
VERSION = 1.0
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

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
        rungaurd.cpp \
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
    rungaurd.h \
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

DISTFILES += \
    dict/de/de-DE.aff \
    dict/de/de-DE.dic \
    dict/en/en-US.aff \
    dict/en/en-US.dic \
    dict/es/es.aff \
    dict/es/es.dic \
    dict/fr/fr.aff \
    dict/fr/fr.dic \
    dict/gb/en-GB.aff \
    dict/gb/en-GB.dic

qtPrepareTool(CONVERT_TOOL, qwebengine_convert_dict)

debug_and_release {
    CONFIG(debug, debug|release): DICTIONARIES_DIR = debug/qtwebengine_dictionaries
    else: DICTIONARIES_DIR = release/qtwebengine_dictionaries
} else {
    DICTIONARIES_DIR = qtwebengine_dictionaries
}

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

icon.files = icons/whatsie.png
icon.path = $$DATADIR/icons/hicolor/512x512/apps/

desktop.files = whatsie.desktop
desktop.path = $$DATADIR/applications/

INSTALLS += target icon desktop

