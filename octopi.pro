#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T18:57:08
#
#-------------------------------------------------

QT += core gui network xml dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets quick quickwidgets

DEFINES += OCTOPI_EXTENSIONS
CONFIG += qt warn_on debug
QMAKE_CXXFLAGS += -std=c++11
TEMPLATE = app
DESTDIR += bin
OBJECTS_DIR += build
MOC_DIR += build
UI_DIR += build

HEADERS += src/QtSolutions/qtsingleapplication.h \
        src/QtSolutions/qtlocalpeer.h \
        repoeditor/repoentry.h \
        src/repoconf.h \
        src/mainwindow.h \
        src/strconstants.h \
        src/searchlineedit.h \
        src/argumentlist.h \
        src/settingsmanager.h \
        src/uihelper.h \
        src/package.h \
        src/unixcommand.h \
        src/wmhelper.h \
        src/treeviewpackagesitemdelegate.h \
        src/searchbar.h \
        src/transactiondialog.h \
        src/globals.h \
        src/multiselectiondialog.h \
        src/packagerepository.h \
        src/model/packagemodel.h \
        src/ui/octopitabinfo.h \
        src/utils.h \
        src/terminal.h

greaterThan(QT_VERSION, 5.3): HEADERS += src/terminalselectordialog.h

SOURCES += src/QtSolutions/qtsingleapplication.cpp \
        src/QtSolutions/qtlocalpeer.cpp \
        repoeditor/repoentry.cpp \
        src/repoconf.cpp \
        src/main.cpp\
        src/mainwindow.cpp \
        src/searchlineedit.cpp \
        src/argumentlist.cpp \
        src/settingsmanager.cpp \
        src/package.cpp \
        src/unixcommand.cpp \
        src/wmhelper.cpp \
        src/treeviewpackagesitemdelegate.cpp \
        src/mainwindow_init.cpp \
        src/mainwindow_transaction.cpp \
        src/mainwindow_events.cpp \
        src/mainwindow_help.cpp \
        src/searchbar.cpp \
        src/mainwindow_searchbar.cpp \
        src/transactiondialog.cpp \
        src/mainwindow_news.cpp \
        src/mainwindow_refresh.cpp \
        src/globals.cpp \
        src/multiselectiondialog.cpp \
        src/packagerepository.cpp \
        src/model/packagemodel.cpp \
        src/ui/octopitabinfo.cpp \
        src/utils.cpp \
        src/terminal.cpp

greaterThan(QT_VERSION, 5.3): SOURCES += src/terminalselectordialog.cpp

FORMS   += ui/mainwindow.ui \
        ui/transactiondialog.ui \
        ui/multiselectiondialog.ui

RESOURCES += resources.qrc

TRANSLATIONS += resources/translations/octopi_pt_BR.ts \
    resources/translations/octopi_fr_FR.ts \
    resources/translations/octopi_es_ES.ts \
    resources/translations/octopi_es_419.ts \
    resources/translations/octopi_es_AR.ts \
    resources/translations/octopi_el.ts \
    resources/translations/octopi_de.ts \
    resources/translations/octopi_it.ts \
    resources/translations/octopi_ro.ts \
    resources/translations/octopi_tr_TR.ts \
    resources/translations/octopi_ar.ts \
    resources/translations/octopi_ca.ts \
    resources/translations/octopi_bg.ts \
    resources/translations/octopi_pl.ts \
    resources/translations/octopi_id.ts \
    resources/translations/octopi_da.ts \
    resources/translations/octopi_cs.ts \
    resources/translations/octopi_he.ts \
    resources/translations/octopi_lt.ts \
    resources/translations/octopi_nb.ts \
    resources/translations/octopi_sr@latin.ts \
    resources/translations/octopi_sr_RS.ts \
    resources/translations/octopi_nl.ts \
    resources/translations/octopi_ru.ts \
    resources/translations/octopi_uz.ts \
    resources/translations/octopi_fi.ts \
    resources/translations/octopi_vi.ts \
    resources/translations/octopi_en_GB.ts \
    resources/translations/octopi_pt_PT.ts \
    resources/translations/octopi_sv.ts \
    resources/translations/octopi_hu.ts \
    resources/translations/octopi_zh_TW.ts \
    resources/translations/octopi_ms_MY.ts \
    resources/translations/octopi_uk.ts \
    resources/translations/octopi_sk.ts \
    resources/translations/octopi_ja.ts \
    resources/translations/octopi_eu.ts

greaterThan(QT_MAJOR_VERSION, 4){
    TRANSLATIONS += resources/translations/octopi_ast.ts
}
