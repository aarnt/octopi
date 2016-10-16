#-------------------------------------------------
#
# Project created by QtCreator 2013-06-01T15:00:00
#
#-------------------------------------------------

QT += core dbus

CONFIG += qt console warn_on

TEMPLATE = app

DESTDIR += ../bin
OBJECTS_DIR += ../build
MOC_DIR += ../build
UI_DIR += ../build

HEADERS += pacmanhelper.h \
    pacmanhelperadaptor.h

SOURCES += main.cpp \
    pacmanhelper.cpp \
    pacmanhelperadaptor.cpp
    
# install
isEmpty(PREFIX) {
    PREFIX = /usr
}
 
isEmpty(LIBDIR) {
    LIBDIR = $$PREFIX/lib
}
    
target.path = $$LIBDIR/octopi
sources.files = $$SOURCES $$HEADERS *.pro 
sources.path = .

INSTALLS += target
