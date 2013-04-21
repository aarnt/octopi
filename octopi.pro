#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T18:57:08
#
#-------------------------------------------------

QT += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt console warn_on debug

TEMPLATE = app

DESTDIR += bin
OBJECTS_DIR += build
MOC_DIR += build
UI_DIR += build

HEADERS  += src/QtSolutions/qtsingleapplication.h \
        src/QtSolutions/qtlocalpeer.h \
        src/mainwindow.h \
        src/strconstants.h \
        src/searchlineedit.h \
        src/argumentlist.h \
        src/settingsmanager.h \
        src/uihelper.h \
        src/package.h \
        src/packagecontroller.h \
        src/unixcommand.h \
        src/wmhelper.h \
    src/QtSolutions/qtsinglecoreapplication.h \
    src/QtSolutions/QtSingleApplication \
    src/QtSolutions/qtlockedfile.h \
    src/QtSolutions/QtLockedFile \
    src/treeviewpackagesitemdelegate.h \
    src/searchbar.h \
    src/transactiondialog.h

SOURCES += src/QtSolutions/qtsingleapplication.cpp \
        src/QtSolutions/qtlocalpeer.cpp \
        src/main.cpp\
        src/mainwindow.cpp \
        src/searchlineedit.cpp \
        src/argumentlist.cpp \
        src/settingsmanager.cpp \
        src/package.cpp \
        src/packagecontroller.cpp \
        src/unixcommand.cpp \
        src/wmhelper.cpp \
    src/QtSolutions/qtsinglecoreapplication.cpp \
    src/QtSolutions/qtlockedfile.cpp \
    src/QtSolutions/qtlockedfile_win.cpp \
    src/QtSolutions/qtlockedfile_unix.cpp \
    src/treeviewpackagesitemdelegate.cpp \
    src/mainwindow_init.cpp \
    src/mainwindow_transaction.cpp \
    src/mainwindow_events.cpp \
    src/mainwindow_help.cpp \
    src/searchbar.cpp \
    src/mainwindow_searchbar.cpp \
    src/transactiondialog.cpp \
    src/mainwindow_news.cpp \
    src/mainwindow_refresh.cpp

FORMS    += ui/mainwindow.ui \
    ui/transactiondialog.ui

RESOURCES += \
    resources.qrc

OTHER_FILES += \
    src/QtSolutions/qtsinglecoreapplication.pri \
    src/QtSolutions/qtsingleapplication.pri
