#-------------------------------------------------
#
# Project created by QtCreator Wed Apr  7 02:25:15 IST 2021
#
#-------------------------------------------------

# Common security flags for all architectures
QMAKE_CFLAGS += -fstack-protector-strong -fstack-clash-protection -D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS
QMAKE_CXXFLAGS = $$QMAKE_CFLAGS

# Optimization flags
QMAKE_CFLAGS_RELEASE += -O2
QMAKE_CXXFLAGS_RELEASE += -O2

QMAKE_CFLAGS_DEBUG += -O1
QMAKE_CXXFLAGS_DEBUG += -O1

# Architecture-specific flags
equals(QMAKE_HOST.arch, aarch64) {
    QMAKE_CFLAGS += -mbranch-protection=standard
}

# Uncomment if you need specific linker flags as well
#QMAKE_LFLAGS += $$QMAKE_LDFLAGS

QT += core gui webengine webenginewidgets positioning

CONFIG += c++17

#CONFIG += sanitizer
#CONFIG += sanitize_address
#CONFIG += sanitize_memory

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

include(singleapplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

include(widgets/MoreApps/MoreApps.pri)

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# No debug output in release mode
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Print if this is a debug or release build
CONFIG(debug, debug|release) {
    message("This is a debug build")
} else {
    message("This is a release build")
}

# Define git info
GIT_HASH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD)\\\""
GIT_BRANCH="\\\"$$system(git -C \""$$_PRO_FILE_PWD_"\" rev-parse --abbrev-ref HEAD)\\\""
BUILD_TIMESTAMP="\"\\\"$${_DATE_}\\\"\""
DEFINES += GIT_HASH=$$GIT_HASH GIT_BRANCH=$$GIT_BRANCH BUILD_TIMESTAMP=$$BUILD_TIMESTAMP

# Set program version
VERSION = 4.16.3
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        about.cpp \
        automatictheme.cpp \
        dictionaries.cpp \
        downloadmanagerwidget.cpp \
        downloadwidget.cpp \
        lock.cpp \
        main.cpp \
        mainwindow.cpp \
        permissiondialog.cpp \
        rateapp.cpp \
        settingswidget.cpp \
        sunclock.cpp \
        theme.cpp \
        utils.cpp \
        webenginepage.cpp \
        webview.cpp \
        widgets/elidedlabel/elidedlabel.cpp \
        widgets/scrolltext/scrolltext.cpp
RESOURCES += \
        icons.qrc

HEADERS += \
    about.h \
    autolockeventfilter.h \
    automatictheme.h \
    common.h \
    def.h \
    dictionaries.h \
    downloadmanagerwidget.h \
    downloadwidget.h \
    lock.h \
    mainwindow.h \
    notificationpopup.h \
    permissiondialog.h \
    rateapp.h \
    requestinterceptor.h \
    settingsmanager.h \
    settingswidget.h \
    sunclock.hpp \
    theme.h \
    utils.h \
    webenginepage.h \
    webview.h \
    widgets/elidedlabel/elidedlabel.h \
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

TRANSLATIONS += \
    i18n/it_IT.ts


# Default rules for deployment
isEmpty(PREFIX){
 PREFIX = /usr
}

message("Installation prefix: $$PREFIX")

BINDIR  = $$PREFIX/bin
DATADIR = $$PREFIX/share

target.path = $$BINDIR

CONFIG(FLATPAK){
    message("This is a flatpak build, assuming dicts are not required.")
}else{
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
    
    dictionaries.files = $${DICTIONARIES_DIR}/
    dictionaries.path  = $$DATADIR/org.keshavnrj.ubuntu/WhatSie/
    
    unix:INSTALLS += dictionaries
}


icon16.path = $$PREFIX/share/icons/hicolor/16x16/apps/
icon16.files = ../dist/linux/hicolor/16x16/apps/com.ktechpit.whatsie.png
icon32.path = $$PREFIX/share/icons/hicolor/32x32/apps/
icon32.files = ../dist/linux/hicolor/32x32/apps/com.ktechpit.whatsie.png
icon64.path = $$PREFIX/share/icons/hicolor/64x64/apps/
icon64.files = ../dist/linux/hicolor/64x64/apps/com.ktechpit.whatsie.png
icon128.path = $$PREFIX/share/icons/hicolor/128x128/apps/
icon128.files = ../dist/linux/hicolor/128x128/apps/com.ktechpit.whatsie.png
icon256.path = $$PREFIX/share/icons/hicolor/256x256/apps/
icon256.files = ../dist/linux/hicolor/256x256/apps/com.ktechpit.whatsie.png

iconscalable.path = $$PREFIX/share/icons/hicolor/scalable/apps/
iconscalable.files = ../dist/linux/hicolor/scalable/apps/com.ktechpit.whatsie.svg

iconsymbolic.path = $$PREFIX/share/icons/hicolor/symbolic/apps/
iconsymbolic.files = ../dist/linux/hicolor/symbolic/apps/com.ktechpit.whatsie-symbolic.svg

license.path = $$PREFIX/share/licenses/whatsie/
license.files = ../LICENSE

appstream.path = $$PREFIX/share/metainfo/
appstream.files = ../dist/linux/com.ktechpit.whatsie.appdata.xml

desktop.path  = $$DATADIR/applications/
desktop.files = ../dist/linux/com.ktechpit.whatsie.desktop

unix:INSTALLS += target icon16 icon32 icon64 icon128 icon256
unix:INSTALLS += iconscalable iconsymbolic license appstream desktop
