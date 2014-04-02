#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T18:57:08
#
#-------------------------------------------------

TARGET = octopi-repoeditor
QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt console warn_on debug

TEMPLATE = app

DESTDIR += bin
OBJECTS_DIR += build
MOC_DIR += build
UI_DIR += build

HEADERS  += addrepo.h \
            checkboxdelegate.h \
            optionsdelegate.h \
            repoconf.h \
            repoeditor.h \
            repoentry.h

SOURCES += addrepo.cpp \
           checkboxdelegate.cpp \
           main.cpp \
           optionsdelegate.cpp \
           repoconf.cpp \
           repoeditor.cpp \
           repoentry.cpp

FORMS   += addrepo.ui \
           repoeditor.ui

RESOURCES += \
    resources.qrc
    
TRANSLATIONS += resources/translations/octopi_repoeditor_en.ts \
                resources/translations/octopi_repoeditor_it.ts

