#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T18:57:08
#
#-------------------------------------------------

QT += core gui network widgets

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
    
TRANSLATIONS += resources/translations/octopi_repoeditor_pt_BR.ts \
    resources/translations/octopi_repoeditor_fr_FR.ts \
    resources/translations/octopi_repoeditor_fr.ts \
    resources/translations/octopi_repoeditor_es_ES.ts \
    resources/translations/octopi_repoeditor_es_419.ts \
    resources/translations/octopi_repoeditor_es_AR.ts \
    resources/translations/octopi_repoeditor_el.ts \
    resources/translations/octopi_repoeditor_de.ts \
    resources/translations/octopi_repoeditor_it.ts \
    resources/translations/octopi_repoeditor_ro.ts \
    resources/translations/octopi_repoeditor_tr_TR.ts \
    resources/translations/octopi_repoeditor_ar.ts \
    resources/translations/octopi_repoeditor_ca.ts \
    resources/translations/octopi_repoeditor_bg.ts \
    resources/translations/octopi_repoeditor_pl.ts \
    resources/translations/octopi_repoeditor_id.ts \
    resources/translations/octopi_repoeditor_da.ts \
    resources/translations/octopi_repoeditor_cs.ts \
    resources/translations/octopi_repoeditor_he.ts \
    resources/translations/octopi_repoeditor_lt.ts \
    resources/translations/octopi_repoeditor_nb.ts \
    resources/translations/octopi_repoeditor_sr@latin.ts \
    resources/translations/octopi_repoeditor_sr_RS.ts \
    resources/translations/octopi_repoeditor_nl.ts \
    resources/translations/octopi_repoeditor_ru.ts \
    resources/translations/octopi_repoeditor_uz.ts \
    resources/translations/octopi_repoeditor_fi.ts \
    resources/translations/octopi_repoeditor_fi_FI.ts \
    resources/translations/octopi_repoeditor_vi.ts \
    resources/translations/octopi_repoeditor_en_GB.ts \
    resources/translations/octopi_repoeditor_pt_PT.ts \
    resources/translations/octopi_repoeditor_sv.ts \
    resources/translations/octopi_repoeditor_sl.ts \
    resources/translations/octopi_repoeditor_hu.ts \
    resources/translations/octopi_repoeditor_zh_TW.ts \
    resources/translations/octopi_repoeditor_ms_MY.ts \
    resources/translations/octopi_repoeditor_uk.ts \
    resources/translations/octopi_repoeditor_sk.ts \
    resources/translations/octopi_repoeditor_ja.ts \
    resources/translations/octopi_repoeditor_eu.ts \
    resources/translations/octopi_repoeditor_gl_ES.ts \
    resources/translations/octopi_repoeditor_ast.ts \    
    resources/translations/octopi_repoeditor_hr.ts \
    resources/translations/octopi_repoeditor_zh-Hans.ts
    resources/translations/octopi_repoeditor_zh_CN.ts
