#-------------------------------------------------
#
# Project created by QtCreator 2013-07-20T16:15:14
#
#-------------------------------------------------

QT += core xml gui network dbus

# This controls whether octopi-notifier uses KStatusNotifier lib
# You SHOULD REALLY enable KSTATUS define in plasma 5 desktops!
DEFINES += ALPM_BACKEND #KSTATUS
CONFIG += qt warn_on debug link_pkgconfig ALPM_BACKEND

ALPM_BACKEND {
  QMAKE_CXXFLAGS += -std=c++11
  PKGCONFIG += glib-2.0
  LIBS += -lglib-2.0 -lalpm -lalpm_octopi_utils
} else {
  QMAKE_CXXFLAGS += -std=c++11
}

contains(DEFINES, KSTATUS){
  QT += KNotifications
}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt console warn_on debug
QMAKE_CXXFLAGS += -std=c++11
TARGET = octopi-notifier
TEMPLATE = app
DESTDIR += ../bin
OBJECTS_DIR += ../build-octopi-notifier
MOC_DIR += ../build-octopi-notifier
UI_DIR += ../build-octopi-notifier

HEADERS  += \
    mainwindow.h \
    setupdialog.h \
    outputdialog.h \
    ../../src/uihelper.h \
    ../../src/terminal.h \
    ../../src/unixcommand.h \
    ../../src/wmhelper.h \
    ../../src/strconstants.h \
    ../../src/package.h \
    ../pacmanhelper/pacmanhelperclient.h \
    ../../src/utils.h \
    ../../src/transactiondialog.h \
    ../../src/argumentlist.h \
    ../../src/pacmanexec.h \
    ../../src/searchlineedit.h \
    ../../src/searchbar.h

ALPM_BACKEND{
  HEADERS += ../../src/alpmbackend.h
}

SOURCES += main.cpp \
    mainwindow.cpp \
    setupdialog.cpp \
    outputdialog.cpp \
    ../../src/terminal.cpp \
    ../../src/unixcommand.cpp \
    ../../src/package.cpp \
    ../../src/wmhelper.cpp \
    ../../src/settingsmanager.cpp \
    ../pacmanhelper/pacmanhelperclient.cpp \
    ../../src/utils.cpp \
    ../../src/transactiondialog.cpp \
    ../../src/argumentlist.cpp \
    ../../src/pacmanexec.cpp \
    ../../src/searchlineedit.cpp \
    ../../src/searchbar.cpp

ALPM_BACKEND{
  SOURCES += ../../src/alpmbackend.cpp
}

FORMS += ../../ui/transactiondialog.ui \
    ui/setupdialog.ui

RESOURCES += \
    ../../resources.qrc
    
# install
isEmpty(PREFIX) {
    PREFIX = /usr
}
 
isEmpty(BINDIR) {
    BINDIR = $$PREFIX/bin
}
    
target.path = $$BINDIR
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro 
sources.path = .

INSTALLS += target
