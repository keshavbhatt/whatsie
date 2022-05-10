QT += core network
CONFIG += c++11

HEADERS += $$PWD/SingleApplication \
    $$PWD/singleapplication.h \
    $$PWD/singleapplication_p.h
SOURCES += $$PWD/singleapplication.cpp \
    $$PWD/singleapplication_p.cpp

INCLUDEPATH += $$PWD
