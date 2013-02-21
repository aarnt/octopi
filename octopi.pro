#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T18:57:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

DESTDIR += bin
OBJECTS_DIR += build
MOC_DIR += build
UI_DIR += build

HEADERS  += src/mainwindow.h

SOURCES += src/main.cpp\
        src/mainwindow.cpp

FORMS    += ui/mainwindow.ui
