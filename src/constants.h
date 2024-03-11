/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2013 Alexandre Albuquerque Arnt
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include <QDir>

/*
 * Collection of Octopi constants and enums
 */

const QString ctn_APPLICATION_VERSION(QStringLiteral("0.16 (dev)"));

//MainWindow related
enum SystemUpgradeOptions { ectn_NO_OPT, ectn_SYNC_DATABASE_OPT, ectn_NOCONFIRM_OPT };

//Tab indices for Properties' tabview
const int ctn_TABINDEX_INFORMATION(0);
const int ctn_TABINDEX_FILES(1);
const int ctn_TABINDEX_ACTIONS(2);
const int ctn_TABINDEX_OUTPUT(3);
const int ctn_TABINDEX_NEWS(4);
const int ctn_TABINDEX_HELPUSAGE(5);
const int ctn_TABINDEX_TERMINAL(6);

//UnixCommand related
const QString ctn_MIRROR_CHECK_APP(QStringLiteral("mirror-check"));

const QString ctn_OCTOPI_COPYRIGHT =
    QLatin1String("/*"
    "* This file is part of Octopi, an open-source GUI for pacman."
    "* Copyright (C) 2013 Alexandre Albuquerque Arnt"
    "*"
    "* This program is free software; you can redistribute it and/or modify"
    "* it under the terms of the GNU General Public License as published by"
    "* the Free Software Foundation; either version 2 of the License, or"
    "* (at your option) any later version."
    "*"
    "* This program is distributed in the hope that it will be useful,"
    "* but WITHOUT ANY WARRANTY; without even the implied warranty of"
    "* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
    "* GNU General Public License for more details."
    "*"
    "* You should have received a copy of the GNU General Public License"
    "* along with this program; if not, write to the Free Software"
    "* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA"
    "*"
    "*/");

enum CommandExecuting { ectn_NONE, ectn_CHECK_UPDATES, ectn_MIRROR_CHECK,
                        ectn_SYNC_DATABASE, ectn_SYSTEM_UPGRADE, ectn_INSTALL, ectn_REMOVE,
                        ectn_CHANGE_INSTALL_REASON,
                        ectn_REMOVE_INSTALL, ectn_REMOVE_KCP_PKG,
                        ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL, ectn_RUN_IN_TERMINAL,
                        ectn_INSTALL_YAY, ectn_SYSINFO };

enum LinuxDistro { ectn_ARCHBANGLINUX, ectn_ARCHCRAFT, ectn_ARCHLINUX, ectn_ARTIXLINUX, ectn_CHAKRA, ectn_CONDRESOS,
                   ectn_ENDEAVOUROS, ectn_GARUDALINUX, ectn_KAOS, ectn_MANJAROLINUX, ectn_OBARUN, ectn_PARABOLA, ectn_UNKNOWN };

enum OptionsDialogTab { ectn_TAB_GENERAL, ectn_TAB_AUR };
enum Language { ectn_LANG_ENGLISH, ectn_LANG_USER_DEFINED };

//PacmanExec - QTextBrowser related
enum TreatString { ectn_TREAT_STRING, ectn_DONT_TREAT_STRING };
enum TreatURLLinks { ectn_TREAT_URL_LINK, ectn_DONT_TREAT_URL_LINK };

//SettingsManager - Octopi related
enum SaveSettingsReason { ectn_AUR_PACKAGELIST, ectn_PACKAGELIST, ectn_CURRENTTABINDEX, ectn_NORMAL=30,
                          ectn_MAXIMIZE_PACKAGES=40, ectn_MAXIMIZE_PROPERTIES=50, ectn_GROUPS=5, ectn_CONSOLE_FONT_SIZE };

const QString ctn_ORGANIZATION(QStringLiteral("octopi"));
const QString ctn_APPLICATION(QStringLiteral("octopi"));
const QString ctn_KEY_SHOW_PACKAGE_NUMBERS_OUTPUT(QStringLiteral("Show_Package_Numbers_Output"));
const QString ctn_KEY_ENABLE_TRANSACTION_DIALOG_IN_SYSTEM_UPGRADE(QStringLiteral("Enable_Transaction_Dialog_In_System_Upgrade"));
const QString ctn_KEY_ENABLE_INTERNET_CHECKING(QStringLiteral("Enable_Internet_Checking"));
const QString ctn_KEY_SHOW_STOP_TRANSACTION(QStringLiteral("Show_Stop_Transaction"));
const QString ctn_KEY_AUR_TOOL(QStringLiteral("Aur_Tool_Name"));
const QString ctn_KEY_AUR_OVERWRITE_PARAM(QStringLiteral("Aur_Overwrite_Param"));
const QString ctn_KEY_AUR_NO_CONFIRM_PARAM(QStringLiteral("Aur_No_Confirm_Param"));
const QString ctn_KEY_AUR_NO_EDIT_PARAM(QStringLiteral("Aur_No_Edit_Param"));
const QString ctn_KEY_AUR_BUILDDIR(QStringLiteral("Aur_Builddir"));
const QString ctn_KEY_SEARCH_OUTDATED_AUR_PACKAGES(QStringLiteral("Search_Outdated_Aur_Packages"));
const QString ctn_KEY_ENABLE_AUR_VOTING(QStringLiteral("Enable_Aur_Voting"));
const QString ctn_KEY_AUR_USERNAME(QStringLiteral("Aur_Username"));
const QString ctn_KEY_AUR_PASSWORD(QStringLiteral("Aur_Password"));
const QString ctn_KEY_USE_ALTERNATE_ROW_COLOR(QStringLiteral("Use_Alternate_Row_Color"));
const QString ctn_KEY_CONSOLE_SIZE(QStringLiteral("Console_Font_Size"));
const QString ctn_KEY_BACKEND(QStringLiteral("Backend"));
const QString ctn_KEY_CURRENT_TAB_INDEX(QStringLiteral("Current_Tab_Index"));
const QString ctn_KEY_WINDOW_SIZE(QStringLiteral("Window_Size"));
const QString ctn_KEY_TRANSACTION_WINDOW_SIZE(QStringLiteral("Transaction_Window_Size"));
const QString ctn_KEY_OUTPUTDIALOG_WINDOW_SIZE(QStringLiteral("OutputDialog_Window_Size"));
const QString ctn_KEY_OPTIONALDEPS_WINDOW_SIZE(QStringLiteral("OptionalDeps_Window_Size"));
const QString ctn_KEY_PANEL_ORGANIZING(QStringLiteral("Panel_Organizing"));
const QString ctn_KEY_PACKAGE_LIST_ORDERED_COL(QStringLiteral("PackageList_Ordered_Col"));
const QString ctn_KEY_PACKAGE_LIST_SORT_ORDER(QStringLiteral("PackageList_Sort_Order"));
const QString ctn_KEY_USE_DEFAULT_APP_ICON(QStringLiteral("Use_Default_App_Icon"));
const QString ctn_KEY_OCTOPI_BUSY_ICON_PATH(QStringLiteral("Octopi_Busy_Icon_Path"));
const QString ctn_KEY_OCTOPI_RED_ICON_PATH(QStringLiteral("Octopi_Red_Icon_Path"));
const QString ctn_KEY_OCTOPI_YELLOW_ICON_PATH(QStringLiteral("Octopi_Yellow_Icon_Path"));
const QString ctn_KEY_OCTOPI_GREEN_ICON_PATH(QStringLiteral("Octopi_Green_Icon_Path"));
const QString ctn_KEY_DISTRO_RSS_URL(QStringLiteral("Distro_RSS_Url"));
const QString ctn_KEY_AUR_PACKAGE_LIST_ORDERED_COL(QStringLiteral("Aur_PackageList_Ordered_Col"));
const QString ctn_KEY_AUR_PACKAGE_LIST_SORT_ORDER(QStringLiteral("Aur_PackageList_Sort_Order"));
const QString ctn_KEY_SKIP_MIRRORCHECK_ON_STARTUP(QStringLiteral("Skip_Mirror_Check_At_Startup"));
const QString ctn_KEY_SPLITTER_HORIZONTAL_STATE(QStringLiteral("Splitter_Horizontal_State"));
const QString ctn_KEY_SHOW_GROUPS_PANEL(QStringLiteral("Show_Groups_Panel"));
const QString ctn_KEY_PACKAGE_ICON_COLUMN_WIDTH(QStringLiteral("Package_Icon_Column_Width"));
const QString ctn_KEY_PACKAGE_NAME_COLUMN_WIDTH(QStringLiteral("Package_Name_Column_Width"));
const QString ctn_KEY_PACKAGE_VERSION_COLUMN_WIDTH(QStringLiteral("Package_Version_Column_Width"));
const QString ctn_KEY_PACKAGE_REPOSITORY_COLUMN_WIDTH(QStringLiteral("Package_Repository_Column_Width"));
const QString ctn_KEY_PACKAGE_LICENSES_COLUMN_WIDTH(QStringLiteral("Package_Licenses_Column_Width"));
const QString ctn_KEY_PACKAGE_DOWNLOAD_SIZE_COLUMN_WITH(QStringLiteral("Package_Download_Size_Column_Width"));
const QString ctn_KEY_PACKAGE_INSTALLED_SIZE_COLUMN_WITH(QStringLiteral("Package_Installed_Size_Column_Width"));
const QString ctn_KEY_PACKAGE_BUILD_DATE_COLUMN_WITH(QStringLiteral("Package_Build_Date_Column_Width"));
const QString ctn_KEY_PACKAGE_INSTALL_DATE_COLUMN_WITH(QStringLiteral("Package_Install_Date_Column_Width"));
const QString ctn_KEY_PACKAGE_INSTALL_REASON_COLUMN_WITH(QStringLiteral("Package_Install_Reason_Column_Width"));
const QString ctn_KEY_SHOW_PACKAGE_LICENSES_COLUMN(QStringLiteral("Show_Package_Licenses_Column"));
const QString ctn_KEY_SHOW_PACKAGE_INSTALLED_SIZE_COLUMN(QStringLiteral("Show_Package_Installed_Size_Column"));
const QString ctn_KEY_SHOW_PACKAGE_BUILD_DATE_COLUMN(QStringLiteral("Show_Package_Build_Date_Column"));
const QString ctn_KEY_SHOW_PACKAGE_INSTALL_DATE_COLUMN(QStringLiteral("Show_Package_Install_Date_Column"));
const QString ctn_KEY_SHOW_PACKAGE_INSTALL_REASON_COLUMN(QStringLiteral("Show_Package_Install_Reason_Column"));
const QString ctn_KEY_SU_TOOL(QStringLiteral("SU_Tool_Name"));
const QString ctn_KEY_TERMINAL(QStringLiteral("Terminal"));
const QString ctn_KEY_TERMINAL_COLOR_SCHEME(QStringLiteral("Terminal_Color_Scheme"));
const QString ctn_KEY_TERMINAL_FONT_FAMILY(QStringLiteral("Terminal_Font_Family"));
const QString ctn_KEY_TERMINAL_FONT_POINT_SIZE(QStringLiteral("Terminal_Font_Point_Size"));
const QString ctn_KEY_INSTANT_SEARCH(QStringLiteral("Instant_Search"));
const QString ctn_KEY_PROXY_SETTINGS(QStringLiteral("Proxy_Settings"));
const QString ctn_AUTOMATIC(QStringLiteral("automatic"));

//SettingsManager - Notifier related
const QString ctn_KEY_LAST_CHECKUPDATES_TIME(QStringLiteral("LastCheckUpdatesTime"));
const QString ctn_KEY_CHECKUPDATES_INTERVAL(QStringLiteral("CheckUpdatesInterval"));
const QString ctn_KEY_CHECKUPDATES_HOUR(QStringLiteral("CheckUpdatesHour"));

//SettingsManager - CacheCleaner related
const QString ctn_KEY_CACHE_CLEANER_WINDOW_SIZE(QStringLiteral("Cache_Cleaner_Window_Size"));
const QString ctn_KEY_KEEP_NUM_INSTALLED(QStringLiteral("Keep_Num_Installed"));
const QString ctn_KEY_KEEP_NUM_UNINSTALLED(QStringLiteral("Keep_Num_Uninstalled"));

//SettingsManager - RepoEditor related
const QString ctn_KEY_REPO_EDITOR_WINDOW_SIZE(QStringLiteral("Repo_Editor_Window_Size"));

//pacman command to retrieve list of upgradable packages with name, version and size
const QString ctn_PACMAN_SUP_COMMAND = QStringLiteral("pacman --print-format \"%n %v %s\" -Spu");

//Package related
const QString ctn_TEMP_ACTIONS_FILE ( QDir::tempPath() + QDir::separator() + QLatin1String(".qt_temp_octopi_") );
const QString ctn_PACMAN_DATABASE_DIR = QStringLiteral("/var/lib/pacman");
const QString ctn_PACMAN_DATABASE_LOCK_FILE(QStringLiteral("/var/lib/pacman/db.lck"));
const QString ctn_PACMAN_CORE_DB_FILE = QStringLiteral("/var/lib/pacman/sync/core.db");

//ARTIX MAIN DB FILE
const QString ctn_PACMAN_SYSTEM_DB_FILE = QStringLiteral("/var/lib/pacman/sync/system.db");

enum PackageStatus { ectn_INSTALLED, ectn_NON_INSTALLED, ectn_OUTDATED, ectn_NEWER,
                     ectn_FOREIGN, ectn_FOREIGN_OUTDATED };

enum ViewOptions { ectn_ALL_PKGS, ectn_INSTALLED_PKGS, ectn_NON_INSTALLED_PKGS, ectn_OUTDATED_PKGS };

//Supported AUR user base package tools
const QString ctn_PACAUR_TOOL(QStringLiteral("pacaur"));
const QString ctn_PARU_TOOL(QStringLiteral("paru"));
const QString ctn_PIKAUR_TOOL(QStringLiteral("pikaur"));
const QString ctn_TRIZEN_TOOL(QStringLiteral("trizen"));
//const QString ctn_YAOURT_TOOL(QStringLiteral("yaourt"));
const QString ctn_YAY_TOOL(QStringLiteral("yay"));
const QString ctn_NO_AUR_TOOL(QStringLiteral("DO_NOT_USE_AUR"));

//KaOS user base package tool
const QString ctn_KCP_TOOL(QStringLiteral("kcp"));
//Chakra user base package tool
const QString ctn_CHASER_TOOL(QStringLiteral("chaser"));

//TransactionDialog related
const int ctn_RUN_IN_TERMINAL(328);

//WMHelper related
const QString ctn_NO_SU_COMMAND(QStringLiteral("none"));
const QString ctn_ROOT_SH(QStringLiteral("/bin/sh -c "));

const QString ctn_OCTOPISUDO(QStringLiteral("/usr/local/bin/qt-sudo"));
const QString ctn_OCTOPISUDO_PARAMS(QStringLiteral("-d"));

const QString ctn_KDE_DESKTOP(QStringLiteral("kwin"));
const QString ctn_KDE_X11_DESKTOP(QStringLiteral("kwin_x11"));
const QString ctn_KDE_WAYLAND_DESKTOP(QStringLiteral("kwin_wayland"));
const QString ctn_KDE_EDITOR(QStringLiteral("kwrite"));
const QString ctn_KDE_FILE_MANAGER(QStringLiteral("kfmclient"));
const QString ctn_KDE_TERMINAL(QStringLiteral("konsole"));
const QString ctn_KDE4_OPEN(QStringLiteral("kde-open"));
const QString ctn_KDE5_OPEN(QStringLiteral("kde-open5"));
const QString ctn_KDE4_FILE_MANAGER(QStringLiteral("dolphin"));
const QString ctn_KDE4_EDITOR(QStringLiteral("kate"));

const QString ctn_TDESU(QStringLiteral("tdesu"));
const QString ctn_TDE_DESKTOP(QStringLiteral("twin"));
const QString ctn_TDE_EDITOR(QStringLiteral("kwrite"));
const QString ctn_TDE_FILE_MANAGER(QStringLiteral("kfmclient"));
const QString ctn_TDE_TERMINAL(QStringLiteral("konsole"));

const QString ctn_ANTERGOS_FILE_MANAGER(QStringLiteral("nautilus"));

const QString ctn_ARCHBANG_EDITOR(QStringLiteral("medit"));
const QString ctn_ARCHBANG_FILE_MANAGER(QStringLiteral("spacefm"));

const QString ctn_RXVT_TERMINAL(QStringLiteral("urxvt"));

const QString ctn_GNOME_DESKTOP(QStringLiteral("mutter"));
const QString ctn_GNOME_EDITOR(QStringLiteral("gedit"));
const QString ctn_GNOME_FILE_MANAGER(QStringLiteral("nautilus"));
const QString ctn_GNOME_TERMINAL(QStringLiteral("gnome-terminal"));

const QString ctn_XDG_OPEN(QStringLiteral("xdg-open"));

const QString ctn_XFCE_DESKTOP(QStringLiteral("xfdesktop"));
const QString ctn_XFCE_EDITOR(QStringLiteral("mousepad"));
const QString ctn_XFCE_EDITOR_ALT(QStringLiteral("leafpad"));
const QString ctn_XFCE_FILE_MANAGER(QStringLiteral("thunar"));
const QString ctn_XFCE_TERMINAL(QStringLiteral("xfce4-terminal"));

const QString ctn_OPENBOX_DESKTOP(QStringLiteral("openbox"));
const QString ctn_LXDE_DESKTOP(QStringLiteral("lxsession"));
const QString ctn_LXDE_TERMINAL(QStringLiteral("lxterminal"));
const QString ctn_LXDE_FILE_MANAGER(QStringLiteral("pcmanfm"));

const QString ctn_LUMINA_DESKTOP(QStringLiteral("lumina-desktop"));
const QString ctn_LUMINA_EDITOR(QStringLiteral("lumina-textedit"));
const QString ctn_LUMINA_FILE_MANAGER(QStringLiteral("lumina-fm"));
const QString ctn_LUMINA_OPEN(QStringLiteral("lumina-open"));

const QString ctn_LXQT_DESKTOP(QStringLiteral("lxqt-session"));
const QString ctn_LXQT_TERMINAL(QStringLiteral("qterminal"));
const QString ctn_LXQT_FILE_MANAGER(QStringLiteral("pcmanfm-qt"));
const QString ctn_LXQT_EDITOR(QStringLiteral("juffed"));

const QString ctn_MATE_DESKTOP(QStringLiteral("mate-session"));
const QString ctn_MATE_EDITOR(QStringLiteral("pluma"));
const QString ctn_MATE_FILE_MANAGER(QStringLiteral("caja"));
const QString ctn_MATE_TERMINAL(QStringLiteral("mate-terminal"));

const QString ctn_CINNAMON_DESKTOP(QStringLiteral("cinnamon-session"));
const QString ctn_CINNAMON_EDITOR(QStringLiteral("gedit"));
const QString ctn_CINNAMON_FILE_MANAGER(QStringLiteral("nemo"));
const QString ctn_CINNAMON_TERMINAL(QStringLiteral("gnome-terminal"));

const QString ctn_PEK_TERMINAL(QStringLiteral("sakura"));
const QString ctn_XTERM(QStringLiteral("xterm"));
const QString ctn_QTERMWIDGET(QStringLiteral("qtermwidget5"));

enum EditOptions { ectn_EDIT_AS_ROOT, ectn_EDIT_AS_NORMAL_USER };

//Octopi-notifier related  -------------------------------------------------------------------------------

const QString ctn_CHECKUPDATES_BINARY = QStringLiteral("/usr/bin/checkupdates");
enum ExecOpt { ectn_NORMAL_EXEC_OPT, ectn_CHECKUPDATES_EXEC_OPT, ectn_SYSUPGRADE_EXEC_OPT,
               ectn_SYSUPGRADE_NOCONFIRM_EXEC_OPT, ectn_AUR_UPGRADE_EXEC_OPT };

//Octopi-helper related ----------------------------------------------------------------------------------

const QString ctn_OCTOPI_HELPER_NAME(QStringLiteral("octphelper"));
const QString ctn_OCTOPI_HELPER_PATH(QStringLiteral("/usr/lib/octopi/octphelper"));

const int ctn_NO_TEMP_ACTIONS_FILE(1);
const int ctn_PACMAN_PROCESS_EXECUTING(2);
const int ctn_SUSPICIOUS_ACTIONS_FILE(3);
const int ctn_NO_ROOT_RUNNING(4);
const int ctn_TIMEOUT_CONNECTING(5);
const int ctn_NO_TRANSACTION_EXECUTING(6);
const int ctn_SUSPICIOUS_EXECUTION_METHOD(7);
const int ctn_COULD_NOT_ATTACH_TO_MEM(8);

#endif // CONSTANTS
