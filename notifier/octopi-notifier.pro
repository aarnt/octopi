#-------------------------------------------------
#
# Project created by QtCreator 2013-07-20T16:15:14
#
#-------------------------------------------------

QT += core xml gui network

DEFINES += OCTOPI_EXTENSIONS ALPM_BACKEND

# Disable automatic string conversions
DEFINES += QT_USE_QSTRINGBUILDER \
           QT_NO_CAST_FROM_ASCII \
           QT_NO_CAST_TO_ASCII \
           QT_NO_URL_CAST_FROM_STRING \
           QT_NO_CAST_FROM_BYTEARRAY

CONFIG += qt warn_on debug link_pkgconfig ALPM_BACKEND USE_QTERMWIDGET6

ALPM_BACKEND {
  QMAKE_CXXFLAGS += -std=c++17
  PKGCONFIG += glib-2.0 libalpm
  LIBS += -lalpm_octopi_utils
} else {
  QMAKE_CXXFLAGS += -std=c++17
}

USE_QTERMWIDGET6 {
  LIBS += -lqtermwidget6
  QT += core5compat
} else {
  LIBS += -lqtermwidget5
}

#contains(DEFINES, KSTATUS){
#  QT += dbus
#}

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt console warn_on debug
QMAKE_CXXFLAGS += -std=c++17
TARGET = octopi-notifier
TEMPLATE = app
DESTDIR += ./bin
OBJECTS_DIR += ./build
MOC_DIR += ./build
UI_DIR += ./build

HEADERS  += \
    mainwindow.h \
    outputdialog.h \
    ../src/QtSolutions/qtsingleapplication.h \
    ../src/QtSolutions/qtlocalpeer.h \
    ../src/uihelper.h \
    ../src/terminal.h \
    ../src/unixcommand.h \
    ../src/wmhelper.h \
    ../src/strconstants.h \
    ../src/package.h \
    ../src/utils.h \
    ../src/transactiondialog.h \
    ../src/argumentlist.h \
    ../src/pacmanexec.h \
    ../src/searchlineedit.h \
    ../src/searchbar.h \
    ../src/optionsdialog.h \
    ../src/termwidget.h \
    ../src/aurvote.h \
    ../src/qaesencryption.h

ALPM_BACKEND{
  HEADERS += ../src/alpmbackend.h
}

SOURCES += main.cpp \
    mainwindow.cpp \
    outputdialog.cpp \
    ../src/QtSolutions/qtsingleapplication.cpp \
    ../src/QtSolutions/qtlocalpeer.cpp \
    ../src/terminal.cpp \
    ../src/unixcommand.cpp \
    ../src/package.cpp \
    ../src/wmhelper.cpp \
    ../src/strconstants.cpp \
    ../src/settingsmanager.cpp \
    ../src/utils.cpp \
    ../src/transactiondialog.cpp \
    ../src/argumentlist.cpp \
    ../src/pacmanexec.cpp \
    ../src/searchlineedit.cpp \
    ../src/searchbar.cpp \
    ../src/optionsdialog.cpp \
    ../src/termwidget.cpp \
    ../src/aurvote.cpp \
    ../src/qaesencryption.cpp

ALPM_BACKEND{
  SOURCES += ../src/alpmbackend.cpp
}

FORMS += ../ui/transactiondialog.ui \
    ../ui/optionsdialog.ui

RESOURCES += \
    ../resources.qrc
    
# install
isEmpty(PREFIX) {
    PREFIX = /usr
}
 
isEmpty(BINDIR) {
    BINDIR = $$PREFIX/bin
}

isEmpty(DATADIR) {
    DATADIR = $$PREFIX/share
}

isEmpty(ETCDIR) {
    ETCDIR = /etc
}
    
target.path = $$BINDIR
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro 
sources.path = .

autostart.path = $$ETCDIR/xdg/autostart
autostart.files += octopi-notifier.desktop

desktop.path = $$DATADIR/applications
desktop.files += octopi-notifier.desktop

INSTALLS += target autostart desktop
