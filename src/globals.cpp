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
QFutureWatcher<QList<PackageListData> *> g_fwForeignPacman;
QFutureWatcher<GroupMemberPair>          g_fwPacmanGroup;
QFutureWatcher<QList<PackageListData> *> g_fwAUR;
QFutureWatcher<QList<PackageListData> *> g_fwAURMeta;
QFutureWatcher<AUROutdatedPackages *> g_fwOutdatedAURPackages;
QFutureWatcher<QString> g_fwDistroNews;
QFutureWatcher<QString> g_fwPackageOwnsFile;
QFutureWatcher<QList<PackageListData> *> g_fwMarkForeignPackages;
QFutureWatcher<QSet<QString> *> g_fwUnrequiredPacman;

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

  bool isForeignPkg = (package->status == ectn_FOREIGN || package->status == ectn_FOREIGN_OUTDATED);
  QString description = package->description;

  if (description.trimmed().isEmpty())
  {
    if (isForeignPkg)
    {
      description = pkgName + " " + Package::getInformationDescription(pkgName, true);
    }
    else return "";
  }

  int space = description.indexOf(" ");
  QString desc = description.mid(space+1);
  int size = desc.size();

  if (desc.size() > 120)
  {
    desc.chop(size - 120);
    desc = desc + " ...";
  }

  QString installedSize = Package::getInformationInstalledSize(pkgName, isForeignPkg);

  if (!installedSize.isEmpty() && installedSize != "0.00 Bytes")
    return desc + " -> " + installedSize;
  else
    return desc;
}

/*
 * Starts the non blocking search for Pacman packages...
 */
QList<PackageListData> * searchPacmanPackages()
{
  return Package::getPackageList("");
}

/*
 * Starts the non blocking search for Pacman Foreign packages...
 */
QList<PackageListData> * searchForeignPackages()
{
  return Package::getForeignPackageList();
}

/*
 * Starts the non blocking search for Unrequired Pacman packages...
 */
QSet<QString> *searchUnrequiredPacmanPackages()
{
  return Package::getUnrequiredPackageList();
}

/*
 * Starts the non blocking search for a Pacman package that owns the given file...
 */
QString searchPacmanPackagesByFile(const QString &file)
{
  QString result;

  if (!file.isEmpty())
  {
    result = UnixCommand::getPackageByFilePath(file);
  }
  else
    result = "";

  return result;
}

/*
 * Starts the non blocking search for Pacman packages...
 */
GroupMemberPair searchPacmanPackagesFromGroup(QString groupName)
{
  return std::make_pair(groupName, Package::getPackagesOfGroup(groupName));
}

/*
 * Starts the non blocking search for AUR packages...
 */
QList<PackageListData> * searchAURPackages(QString searchString)
{
  return Package::getAURPackageList(searchString);
}

/*
 * Starts the non blocking retrive of AUR outdated package versions...
 * Results in a hash: [key] AUR pkg name / [value] AUR pkg version available
 */
AUROutdatedPackages * getOutdatedAURPackages()
{
  AUROutdatedPackages * res = new AUROutdatedPackages();
  res->content = Package::getAUROutdatedPackagesNameVersion();

  return res;
}

/*
 * Starts the non blocking search for RSS distro news...
 */
QString getLatestDistroNews()
{
  return utils::retrieveDistroNews(true);
}

/*
 * Marks the packages installed by AUR/KCP (alien icons in pkg list).
 */
QList<PackageListData> * markForeignPackagesInPkgList(bool hasAURTool, QStringList *outdatedAURStringList)
{
  // Fetch foreign package list
  QList<PackageListData> * result = new QList<PackageListData>();
  std::unique_ptr<QList<PackageListData> > listForeign(Package::getForeignPackageList());
  PackageListData pld;

  QList<PackageListData>::const_iterator itForeign = listForeign->begin();

  while (itForeign != listForeign->end())
  {
    if (!hasAURTool || !outdatedAURStringList->contains(itForeign->name))
    {
      pld = PackageListData(
            itForeign->name, itForeign->repository, itForeign->version,
            itForeign->name + " " + Package::getInformationDescription(itForeign->name, true),
            ectn_FOREIGN);
    }
    else
    {
      pld = PackageListData(
            itForeign->name, itForeign->repository, itForeign->version,
            itForeign->name + " " + Package::getInformationDescription(itForeign->name, true),
            ectn_FOREIGN_OUTDATED);
    }

    result->append(pld);
    ++itForeign;
  }

  return result;
}
