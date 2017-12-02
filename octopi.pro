#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T18:57:08
#
#-------------------------------------------------

QT += core gui network xml dbus widgets
DEFINES += OCTOPI_EXTENSIONS ALPM_BACKEND #TERMWIDGET
CONFIG += qt warn_on debug link_pkgconfig ALPM_BACKEND #TERMWIDGET

ALPM_BACKEND {
  QMAKE_CXXFLAGS += -std=c++11
  PKGCONFIG += glib-2.0 libalpm
  LIBS += -lalpm_octopi_utils
} else {
  QMAKE_CXXFLAGS += -std=c++11
}

TERMWIDGET {
  LIBS += -lqtermwidget5
}

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
        src/terminal.h \
        src/pacmanexec.h \
        src/constants.h \
        src/optionsdialog.h

ALPM_BACKEND{
  HEADERS += src/alpmbackend.h
}

SOURCES += src/QtSolutions/qtsingleapplication.cpp \
        src/QtSolutions/qtlocalpeer.cpp \
        repoeditor/repoentry.cpp \
        src/repoconf.cpp \
        src/main.cpp\
        src/mainwindow.cpp \
        src/strconstants.cpp \
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
        src/terminal.cpp \
        src/pacmanexec.cpp \
        src/optionsdialog.cpp

ALPM_BACKEND{
  SOURCES += src/alpmbackend.cpp
}

FORMS   += ui/mainwindow.ui \
        ui/transactiondialog.ui \
        ui/multiselectiondialog.ui \
        ui/optionsdialog.ui

RESOURCES += resources.qrc

TRANSLATIONS += resources/translations/octopi_pt_BR.ts \
    resources/translations/octopi_fr \
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
    resources/translations/octopi_fi_FI.ts \
    resources/translations/octopi_vi.ts \
    resources/translations/octopi_en.ts \
    resources/translations/octopi_en_GB.ts \
    resources/translations/octopi_pt_PT.ts \
    resources/translations/octopi_sv.ts \
    resources/translations/octopi_sl.ts \
    resources/translations/octopi_hu.ts \
    resources/translations/octopi_zh_TW.ts \
    resources/translations/octopi_ms_MY.ts \
    resources/translations/octopi_uk.ts \
    resources/translations/octopi_sk.ts \
    resources/translations/octopi_ja.ts \
    resources/translations/octopi_eu.ts \
    resources/translations/octopi_gl_ES.ts \
    resources/translations/octopi_ast.ts \
    resources/translations/octopi_hr.ts \
    resources/translations/octopi_zh-Hans.ts \
    resources/translations/octopi_zh_CN.ts
    
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

isEmpty(LIBDIR) {
    LIBDIR = $$PREFIX/lib
}

target.path = $$BINDIR
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS *.pro 
sources.path = .

bin.path = $$BINDIR
bin.files += speedup/speedup-octopi.sh

dbus.path = $$ETCDIR/dbus-1/system.d
dbus.files += notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.conf

desktop.path = $$DATADIR/applications
desktop.files += octopi.desktop
desktop.files += cachecleaner/octopi-cachecleaner.desktop

gnome.path = $$DATADIR/icons/gnome/32x32/apps
gnome.files += resources/images/octopi_green.png
gnome.files += resources/images/octopi.png

icon.path = $$DATADIR/icons
icon.files += resources/images/octopi.png
icon.files += resources/images/octopi_green.png
icon.files += resources/images/octopi_red.png
icon.files += resources/images/octopi_yellow.png

interfaces.path = $$DATADIR/dbus-1/interfaces
interfaces.files += notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.xml

license.path = $$DATADIR/licenses/octopi
license.files += LICENSE

polkit.path = $$DATADIR/polkit-1/actions
polkit.files += notifier/pacmanhelper/polkit/org.octopi.pacman.policy

service.path = $$LIBDIR/systemd/system
service.files += speedup/octopi.service

sys_service.path = $$DATADIR/dbus-1/system-services
sys_service.files += notifier/pacmanhelper/polkit/org.octopi.pacmanhelper.service

INSTALLS += target bin dbus desktop gnome icon interfaces license polkit service sys_service
