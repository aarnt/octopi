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

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QString>
#include <QVariant>
#include <QSettings>
#include <QDir>

const QString ctn_ORGANIZATION("octopi");
const QString ctn_APPLICATION("octopi");
const QString ctn_KEY_CURRENT_TAB_INDEX("Current_Tab_Index");
const QString ctn_KEY_WINDOW_SIZE("Window_Size");
const QString ctn_KEY_PANEL_ORGANIZING("Panel_Organizing");
const QString ctn_KEY_PACKAGE_LIST_ORDERED_COL("PackageList_Ordered_Col");
const QString ctn_KEY_PACKAGE_LIST_SORT_ORDER("PackageList_Sort_Order");
const QString ctn_KEY_SKIP_MIRRORCHECK_ON_STARTUP("Skip_Mirror_Check_At_Startup");
const QString ctn_KEY_SPLITTER_HORIZONTAL_STATE("Splitter_Horizontal_State");
const QString ctn_KEY_SHOW_GROUPS_PANEL("Show_Groups_Panel");
const QString ctn_KEY_PACKAGE_ICON_COLUMN_WIDTH("Package_Icon_Column_Width");
const QString ctn_KEY_PACKAGE_NAME_COLUMN_WIDTH("Package_Name_Column_Width");
const QString ctn_KEY_PACKAGE_VERSION_COLUMN_WIDTH("Package_Version_Column_Width");

//Notifier related
const QString ctn_KEY_LAST_SYNC_DB_TIME("LastSyncDbTime");
const QString ctn_KEY_SYNC_DB_INTERVAL("SyncDbInterval");
const QString ctn_KEY_SYNC_DB_HOUR("SyncDbHour");

const QString ctn_KEY_TERMINAL("Terminal");
const QString ctn_AUTOMATIC("automatic");
const QString ctn_KEEP_NUM_INSTALLED("Keep_Num_Installed");
const QString ctn_KEEP_NUM_UNINSTALLED("Keep_Num_Uninstalled");

enum SaveSettingsReason { ectn_PackageList, ectn_CurrentTabIndex, ectn_NORMAL=30,
                          ectn_MAXIMIZE_PACKAGES=40, ectn_MAXIMIZE_PROPERTIES=50, ectn_GROUPS=5 };

class SettingsManager
{

  private:

    static SettingsManager *m_pinstance;
    QSettings *m_SYSsettings;

    SettingsManager(const SettingsManager&);
    SettingsManager& operator= (const SettingsManager&);
    SettingsManager();
    virtual ~SettingsManager();
    QSettings* getSYSsettings() { return m_SYSsettings; }

  public:

    static SettingsManager* instance();
    static QString getTerminal();

    static QString getOctopiConfPath()
    {
      return QDir::homePath() +
          QDir::separator() + ".config/octopi/octopi.conf";
    }

    static int getCurrentTabIndex();
    static int getPanelOrganizing();
    static int getPackageListOrderedCol();
    static int getPackageListSortOrder();
    static int getPackageIconColumnWidth();
    static int getPackageNameColumnWidth();
    static int getPackageVersionColumnWidth();

    //Notifier related
    static int getSyncDbHour();
    static int getSyncDbInterval();
    static QDateTime getLastSyncDbTime();

    static bool getSkipMirrorCheckAtStartup();
    static bool getShowGroupsPanel();
    static QByteArray getWindowSize();
    static QByteArray getSplitterHorizontalState();
    static bool isValidTerminalSelected();

    //CacheCleaner related
    static int getKeepNumInstalledPackages();
    static int getKeepNumUninstalledPackages();

    static void setCurrentTabIndex(int newValue);
    static void setPanelOrganizing(int newValue);
    static void setPackageListOrderedCol(int newValue);
    static void setPackageListSortOrder(int newValue);
    static void setShowGroupsPanel(int newValue);
    static void setWindowSize(QByteArray newValue);
    static void setSplitterHorizontalState(QByteArray newValue);

    static void setSyncDbHour(int newValue);
    static void setSyncDbInterval(int newValue);
    static void setLastSyncDbTime(QDateTime newValue);

    static void setTerminal(QString newValue);
    static void setKeepNumInstalledPackages(int newValue);
    static void setKeepNumUninstalledPackages(int newValue);

    static void setPackageIconColumnWidth(int newValue);
    static void setPackageNameColumnWidth(int newValue);
    static void setPackageVersionColumnWidth(int newValue);
};

#endif // SETTINGSMANAGER_H
