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

HEADERS +=  cachecleaner.h \
            packagegroupmodel.h \
            ../src/unixcommand.h \
            ../src/wmhelper.h \
            ../src/terminal.h \
            ../src/settingsmanager.h \
            ../src/utils.h \
            ../src/package.h \
            ../src/QtSolutions/qtsingleapplication.h \
            ../src/QtSolutions/qtlocalpeer.h \
            ../src/QtSolutions/qtlockedfile.h \
            ../src/QtSolutions/qtsinglecoreapplication.h
    

SOURCES +=  main.cpp \
            cachecleaner.cpp \
            packagegroupmodel.cpp \
            ../src/unixcommand.cpp \
            ../src/wmhelper.cpp \
            ../src/terminal.cpp \
            ../src/settingsmanager.cpp \
            ../src/utils.cpp \
            ../src/package.cpp \
            ../src/QtSolutions/qtsingleapplication.cpp \
            ../src/QtSolutions/qtlocalpeer.cpp \
            ../src/QtSolutions/qtlockedfile.cpp \
            ../src/QtSolutions/qtsinglecoreapplication.cpp

FORMS +=  ui/cachecleaner.ui

RESOURCES += resources.qrc
    
TRANSLATIONS += resources/translations/octopi_cachecleaner_en.ts \
                resources/translations/octopi_cachecleaner_fr.ts

