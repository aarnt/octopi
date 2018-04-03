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
    static int getAURPackageListOrderedCol();
    static int getAURPackageListSortOrder();

    static int getPackageIconColumnWidth();
    static int getPackageNameColumnWidth();
    static int getPackageVersionColumnWidth();
    static int getPackageRepositoryColumnWidth();

    static bool getUseDefaultAppIcon();    
    static QString getOctopiBusyIconPath();
    static QString getOctopiRedIconPath();
    static QString getOctopiYellowIconPath();
    static QString getOctopiGreenIconPath();

    static bool getShowPackageNumbersOutput();
    static bool getShowStopTransaction();

    static QString getAURTool();
    static QString getAURToolName();
    static bool getPacaurNoConfirmParam();
    static bool getPacaurNoEditParam();
    static bool getYaourtNoConfirmParam();
    static bool getTrizenNoConfirmParam();
    static bool getTrizenNoEditParam();
    static bool getPikaurNoConfirmParam();
    static bool getPikaurNoEditParam();
    static bool getSearchOutdatedAURPackages();

    static QString readSUToolValue();
    static QString getSUTool();
    static bool hasPacmanBackend();

    //CacheCleaner related
    static QByteArray getCacheCleanerWindowSize();
    static int getKeepNumInstalledPackages();
    static int getKeepNumUninstalledPackages();
    static void setCacheCleanerWindowSize(QByteArray newValue);
    static void setKeepNumInstalledPackages(int newValue);
    static void setKeepNumUninstalledPackages(int newValue);

    //Notifier related
    static int getSyncDbHour();
    static int getSyncDbInterval();
    static QDateTime getLastSyncDbTime();
    static bool getSkipMirrorCheckAtStartup();
    static bool getShowGroupsPanel();

    static QByteArray getWindowSize();
    static QByteArray getTransactionWindowSize();
    static QByteArray getOutputDialogWindowSize();
    static QByteArray getOptionalDepsWindowSize();
    static QByteArray getSplitterHorizontalState();

    static bool isValidSUToolSelected();
    static bool isValidTerminalSelected();
    static bool isInstantSearchSelected();

    static void setCurrentTabIndex(int newValue);
    static void setPanelOrganizing(int newValue);
    static void setUseDefaultAppIcon(bool newValue);
    static void setOctopiBusyIconPath(const QString& newValue);
    static void setOctopiRedIconPath(const QString& newValue);
    static void setOctopiYellowIconPath(const QString& newValue);
    static void setOctopiGreenIconPath(const QString& newValue);
    static void setBackend(const QString& newValue);

    static void setPackageListOrderedCol(int newValue);
    static void setPackageListSortOrder(int newValue);    
    static void setAURPackageListOrderedCol(int newValue);
    static void setAURPackageListSortOrder(int newValue);

    static void setShowGroupsPanel(int newValue);
    static void setWindowSize(QByteArray newValue);
    static void setTransactionWindowSize(QByteArray newValue);
    static void setOutputDialogWindowSize(QByteArray newValue);
    static void setOptionalDepsWindowSize(QByteArray newValue);
    static void setSplitterHorizontalState(QByteArray newValue);

    static void setSyncDbHour(int newValue);
    static void setSyncDbInterval(int newValue);
    static void setLastSyncDbTime(QDateTime newValue);

    static void setTerminal(const QString& newValue);
    static void setPackageIconColumnWidth(int newValue);
    static void setPackageNameColumnWidth(int newValue);
    static void setPackageVersionColumnWidth(int newValue);
    static void setPackageRepositoryColumnWidth(int newValue);

    static void setShowPackageNumbersOutput(bool newValue);
    static void setShowStopTransaction(bool newValue);

    static void setAURTool(const QString& newValue);        
    static void setPacaurNoConfirmParam(bool newValue);
    static void setPacaurNoEditParam(bool newValue);
    static void setYaourtNoConfirmParam(bool newValue);
    static void setTrizenNoConfirmParam(bool newValue);
    static void setTrizenNoEditParam(bool newValue);
    static void setPikaurNoConfirmParam(bool newValue);
    static void setPikaurNoEditParam(bool newValue);
    static void setSearchOutdatedAURPackages(bool newValue);
    static void setSUTool(const QString& newValue);
    static void setInstantSearchSelected(bool newValue);
};

#endif // SETTINGSMANAGER_H
