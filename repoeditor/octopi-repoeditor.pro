#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T18:57:08
#
#-------------------------------------------------

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += qt console warn_on debug

TEMPLATE = app
DESTDIR += bin
OBJECTS_DIR += build
MOC_DIR += build
UI_DIR += build

HEADERS += addrepo.h \
            checkboxdelegate.h \
            optionsdelegate.h \
            repoconf.h \
            repoeditor.h \
            repoentry.h \
            ../src/QtSolutions/qtsingleapplication.h \
            ../src/QtSolutions/qtlocalpeer.h \
            ../src/QtSolutions/qtlockedfile.h \
            ../src/QtSolutions/qtsinglecoreapplication.h

SOURCES += addrepo.cpp \
           checkboxdelegate.cpp \
           main.cpp \
           optionsdelegate.cpp \
           repoconf.cpp \
           repoeditor.cpp \
           repoentry.cpp \
           ../src/QtSolutions/qtsingleapplication.cpp \
           ../src/QtSolutions/qtlocalpeer.cpp \
           ../src/QtSolutions/qtlockedfile.cpp \
           ../src/QtSolutions/qtsinglecoreapplication.cpp

FORMS += addrepo.ui \
         repoeditor.ui

RESOURCES += resources.qrc
    
TRANSLATIONS += resources/translations/octopi_repoeditor_en.ts \
                resources/translations/octopi_repoeditor_it.ts \
                resources/translations/octopi_repoeditor_sk.ts \
                resources/translations/octopi_repoeditor_sv.ts
