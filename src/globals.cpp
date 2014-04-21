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

#include "globals.h"
#include "mainwindow.h"
#include "packagecontroller.h"

#include <QStandardItem>
#include <QFutureWatcher>

#if QT_VERSION > 0x050000
  #include <QtConcurrent/QtConcurrentMap>
#else
  #include <QtConcurrentMap>
#endif

/*
 * Global functions related to Octopi's multithread code
 */

QFutureWatcher<QString> g_fwToolTip;
QFutureWatcher<QList<PackageListData> *> g_fwPacman;
QFutureWatcher<GroupMemberPair> g_fwPacmanGroup;
QFutureWatcher<QList<PackageListData> *> g_fwYaourt;
QFutureWatcher<QList<PackageListData> *> g_fwYaourtMeta;
QFutureWatcher<YaourtOutdatedPackages *> g_fwOutdatedYaourtPackages;
QFutureWatcher<QString> g_fwDistroNews;

/*
 * Given a packageName, returns its description
 */
QString showPackageInfo(QString pkgName)
{
  MainWindow *mw = MainWindow::returnMainWindow();
  const PackageRepository::PackageData*const package = mw->getFirstPackageFromRepo(pkgName);

  if (package == NULL) {
    return "";
  }

  QString description = package->description;

  if (description.trimmed().isEmpty()) return "";

  int space = description.indexOf(" ");
  QString desc = description.mid(space+1);
  int size = desc.size();

  if (desc.size() > 120)
  {
    desc.chop(size - 120);
    desc = desc + " ...";
  }

  return desc;
}

/*
 * Starts the non blocking search for Pacman packages...
 */
QList<PackageListData> * searchPacmanPackages()
{
  return Package::getPackageList();
}

/*
 * Starts the non blocking search for Pacman packages...
 */
GroupMemberPair searchPacmanPackagesFromGroup(QString groupName)
{
  return std::make_pair(groupName, Package::getPackagesOfGroup(groupName));
}

/*
 * Starts the non blocking search for Yaourt packages...
 */
QList<PackageListData> * searchYaourtPackages(QString searchString)
{
  return Package::getYaourtPackageList(searchString);
}

/*
 * Starts the non blocking retrive of Yaourt outdated package versions...
 * Results in a hash: [key] Yaourt pkg name / [value] Yaourt pkg version available
 */
YaourtOutdatedPackages * getOutdatedYaourtPackages()
{
  YaourtOutdatedPackages * res = new YaourtOutdatedPackages();
  res->content = Package::getYaourtOutdatedPackagesNameVersion();

  return res;
}

/*
 * Starts the non blocking search for RSS distro news...
 */
QString getLatestDistroNews()
{
  return PackageController::retrieveDistroNews(true);
}
