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

#ifndef MAINWINDOW_GLOBALS_H
#define MAINWINDOW_GLOBALS_H

#include "strconstants.h"
#include "model/packagemodel.h"

#include <QStandardItem>
#include <QFutureWatcher>

struct PkgDesc{
  QString name;
  QString description;
  bool isForeign;
};

struct FTOutdatedPackages //aka ForeignToolOutdatedPackages
{
  public:
    QHash<QString, QString> content;
};

typedef std::pair<QString, QStringList*> GroupMemberPair;

inline QFutureWatcher<QString> g_fwToolTip;
inline QFutureWatcher<QString> g_fwToolTipInfo;
inline QFutureWatcher<QList<PackageListData> *> g_fwPacman;
inline QFutureWatcher<QList<PackageListData> *> g_fwForeignPacman;
inline QFutureWatcher<GroupMemberPair>          g_fwPacmanGroup;
inline QFutureWatcher<QList<PackageListData> *> g_fwAUR;
inline QFutureWatcher<QList<PackageListData> *> g_fwAURMeta;
inline QFutureWatcher<FTOutdatedPackages *> g_fwOutdatedAURPackages;
inline QFutureWatcher<QString> g_fwDistroNews;
inline QFutureWatcher<QString> g_fwPackageOwnsFile;
inline QFutureWatcher<QList<PackageListData> *> g_fwMarkForeignPackages;
inline QFutureWatcher<QSet<QString> *> g_fwUnrequiredPacman;
inline QFutureWatcher<PackageInfoData> g_fwKCPInformation;
inline QFutureWatcher<QStringList *> g_fwOutdatedPkgStringList;
inline QFutureWatcher<QStringList *> g_fwOutdatedAURStringList;
inline QFutureWatcher<QByteArray> g_fwCommandToExecute;
inline QFutureWatcher<QString> g_fwGenerateSysInfo;
inline QFutureWatcher<bool> g_fwDownloadTempYayHelper;

//QString showPackageDescription(QString pkgName);
QString showPackageDescriptionExt(PkgDesc pkgDesc); //const PackageRepository::PackageData*const package);
QList<PackageListData> * searchPacmanPackages(const QHash<QString, QString> *checkUpdatesOutdatedPackages);
QSet<QString> * searchUnrequiredPacmanPackages();
QList<PackageListData> * searchForeignPackages();
QList<PackageListData> * markForeignPackagesInPkgList(bool hasAURTool, QStringList *outdatedAURStringList);
QList<PackageListData> * searchForeignToolPackages(QString searchString);
QString searchPacmanPackagesByFile(const QString &file);
GroupMemberPair          searchPacmanPackagesFromGroup(QString groupName);
FTOutdatedPackages * getOutdatedForeignToolPackages();
QString getLatestDistroNews();
PackageInfoData getKCPInformation(QString pkgName);
QByteArray execCommand(QString cmd);
void execCommandInAnotherThread(QString cmd, QStringList &params);
bool downloadTempYayHelper();

QStringList            * getOutdatedPkgStringList();
QStringList            * getOutdatedAURStringList();

#endif // MAINWINDOW_GLOBALS_H
