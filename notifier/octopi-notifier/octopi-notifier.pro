#-------------------------------------------------
#
# Project created by QtCreator 2013-07-20T16:15:14
#
#-------------------------------------------------

QT       += core gui network dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt console warn_on debug

TARGET = octopi-notifier
TEMPLATE = app

DESTDIR += ../bin
OBJECTS_DIR += ../build-octopi-notifier
MOC_DIR += ../build-octopi-notifier
UI_DIR += ../build-octopi-notifier

SOURCES += main.cpp \
    mainwindow.cpp \
    ../../src/unixcommand.cpp \
    ../../src/package.cpp \
    ../../src/wmhelper.cpp \
    ../../src/settingsmanager.cpp \
    ../../src/pacmanhelperclient.cpp \
    ../../src/utils/processwrapper.cpp

HEADERS  += \
    mainwindow.h \
    ../../src/uihelper.h \
    ../../src/unixcommand.h \
    ../../src/wmhelper.h \
    ../../src/strconstants.h \
    ../../src/package.h \
    ../../src/pacmanhelperclient.h \
    ../../src/utils/processwrapper.h

RESOURCES += \
    ../../resources.qrc
