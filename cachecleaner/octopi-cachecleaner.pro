#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T18:57:08
#
#-------------------------------------------------

QT += core gui network xml widgets

CONFIG += qt console warn_on debug
TEMPLATE = app
DESTDIR += bin
OBJECTS_DIR += build
MOC_DIR += build
UI_DIR += build

HEADERS +=  cachecleaner.h \
            packagegroupmodel.h \
            ../src/strconstants.h \
            ../src/unixcommand.h \
            ../src/wmhelper.h \
            ../src/terminal.h \
            ../src/settingsmanager.h \
            ../src/searchlineedit.h \
            ../src/utils.h \
            ../src/package.h \
            ../src/QtSolutions/qtsingleapplication.h \
            ../src/QtSolutions/qtlocalpeer.h \
            ../src/QtSolutions/qtlockedfile.h \
            ../src/QtSolutions/qtsinglecoreapplication.h
    

SOURCES +=  main.cpp \
            cachecleaner.cpp \
            packagegroupmodel.cpp \
            ../src/strconstants.cpp \
            ../src/unixcommand.cpp \
            ../src/wmhelper.cpp \
            ../src/terminal.cpp \
            ../src/settingsmanager.cpp \
            ../src/searchlineedit.cpp \
            ../src/utils.cpp \
            ../src/package.cpp \
            ../src/QtSolutions/qtsingleapplication.cpp \
            ../src/QtSolutions/qtlocalpeer.cpp \
            ../src/QtSolutions/qtlockedfile.cpp \
            ../src/QtSolutions/qtsinglecoreapplication.cpp

FORMS +=  ui/cachecleaner.ui

RESOURCES += resources.qrc
    
TRANSLATIONS += resources/translations/octopi_cachecleaner_pt_BR.ts \
    resources/translations/octopi_cachecleaner_fr_FR.ts \
    resources/translations/octopi_cachecleaner_fr.ts \
    resources/translations/octopi_cachecleaner_es_ES.ts \
    resources/translations/octopi_cachecleaner_es_419.ts \
    resources/translations/octopi_cachecleaner_es_AR.ts \
    resources/translations/octopi_cachecleaner_el.ts \
    resources/translations/octopi_cachecleaner_de.ts \
    resources/translations/octopi_cachecleaner_it.ts \
    resources/translations/octopi_cachecleaner_ro.ts \
    resources/translations/octopi_cachecleaner_tr_TR.ts \
    resources/translations/octopi_cachecleaner_ar.ts \
    resources/translations/octopi_cachecleaner_ca.ts \
    resources/translations/octopi_cachecleaner_bg.ts \
    resources/translations/octopi_cachecleaner_pl.ts \
    resources/translations/octopi_cachecleaner_id.ts \
    resources/translations/octopi_cachecleaner_da.ts \
    resources/translations/octopi_cachecleaner_cs.ts \
    resources/translations/octopi_cachecleaner_he.ts \
    resources/translations/octopi_cachecleaner_hi.ts \
    resources/translations/octopi_cachecleaner_hi_IN.ts \
    resources/translations/octopi_cachecleaner_lt.ts \
    resources/translations/octopi_cachecleaner_nb.ts \
    resources/translations/octopi_cachecleaner_sr@latin.ts \
    resources/translations/octopi_cachecleaner_sr_RS.ts \
    resources/translations/octopi_cachecleaner_nl.ts \
    resources/translations/octopi_cachecleaner_ru.ts \
    resources/translations/octopi_cachecleaner_uz.ts \
    resources/translations/octopi_cachecleaner_fi.ts \
    resources/translations/octopi_cachecleaner_fi_FI.ts \
    resources/translations/octopi_cachecleaner_vi.ts \
    resources/translations/octopi_cachecleaner_en_GB.ts \
    resources/translations/octopi_cachecleaner_pt_PT.ts \
    resources/translations/octopi_cachecleaner_sv.ts \
    resources/translations/octopi_cachecleaner_sl.ts \
    resources/translations/octopi_cachecleaner_hu.ts \
    resources/translations/octopi_cachecleaner_zh_TW.ts \
    resources/translations/octopi_cachecleaner_ms_MY.ts \
    resources/translations/octopi_cachecleaner_uk.ts \
    resources/translations/octopi_cachecleaner_sk.ts \
    resources/translations/octopi_cachecleaner_ja.ts \
    resources/translations/octopi_cachecleaner_eu.ts \
    resources/translations/octopi_cachecleaner_gl_ES.ts \
    resources/translations/octopi_cachecleaner_ast.ts \
    resources/translations/octopi_cachecleaner_hr.ts \
    resources/translations/octopi_cachecleaner_zh-Hans.ts
    resources/translations/octopi_cachecleaner_zh_CN.ts
    
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
    
target.path = $$BINDIR
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro 
sources.path = .

desktop.path = $$DATADIR/applications
desktop.files += cachecleaner/octopi-cachecleaner.desktop

INSTALLS += target desktop
