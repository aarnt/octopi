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
#include <QtConcurrent/QtConcurrentMap>
#include <QRandomGenerator>
#include <QRegularExpression>

/*
 * Global functions related to Octopi's multithread code
 */

QFutureWatcher<QString> g_fwToolTip;
QFutureWatcher<QString> g_fwToolTipInfo;
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
QFutureWatcher<PackageInfoData> g_fwKCPInformation;
QFutureWatcher<QStringList *> g_fwOutdatedPkgStringList;
QFutureWatcher<QStringList *> g_fwOutdatedAURStringList;
QFutureWatcher<QByteArray> g_fwCommandToExecute;
QFutureWatcher<QString> g_fwGenerateSysInfo;
QFutureWatcher<bool> g_fwInstallTempYayHelper;

/*
 * Given a packageName, returns its description
 */
QString showPackageDescription(QString pkgName)
{
  MainWindow *mw = MainWindow::returnMainWindow();
  const PackageRepository::PackageData*const package = mw->getFirstPackageFromRepo(pkgName);

  if (package == nullptr) {
    return QLatin1String("");
  }

  bool isForeignPkg = (package->status == ectn_FOREIGN || package->status == ectn_FOREIGN_OUTDATED);
  QString description = package->description;

  /*if (description.trimmed().isEmpty())
  {
    if (isForeignPkg)
    {
      description = pkgName + " " + Package::getInformationDescription(pkgName, true);
    }
    else return "";
  }*/

  int space = description.indexOf(QLatin1String(" "));
  QString desc = description.mid(space+1);
  int size = desc.size();

  if (desc.size() > 120)
  {
    desc.chop(size - 120);
    desc = desc + QLatin1String(" ...");
  }

  QString installedSize = Package::getInformationInstalledSize(pkgName, isForeignPkg);

  if (!installedSize.isEmpty() && installedSize != QLatin1String("0.00 Bytes"))
    return desc + QString::fromUtf8(" → ") + installedSize;
  else
    return desc;
}

/*
 * Starts the non blocking search for Pacman packages...
 */
QList<PackageListData> * searchPacmanPackages(const QHash<QString, QString> *checkUpdatesOutdatedPackages)
{
  return Package::getPackageList(QLatin1String(""), checkUpdatesOutdatedPackages);
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
    result = QLatin1String("");

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
  QList<PackageListData> * result = new QList<PackageListData>();
  std::unique_ptr<QList<PackageListData>> listForeign(Package::getForeignPackageList());
  PackageListData pld;

  QList<PackageListData>::const_iterator itForeign = listForeign->cbegin();

  while (itForeign != listForeign->cend())
  {
    if (!hasAURTool || !outdatedAURStringList->contains(itForeign->name))
    {
      pld = PackageListData(
            itForeign->name, itForeign->repository, itForeign->version,
            /*itForeign->name + " " +*/ itForeign->description,
            ectn_FOREIGN);
    }
    else
    {
      pld = PackageListData(
            itForeign->name, itForeign->repository, itForeign->version,
            /*itForeign->name + " " +*/ itForeign->description,
            ectn_FOREIGN_OUTDATED);
    }

    result->append(pld);
    ++itForeign;
  }

  return result;
}

/*
 * Retrieves KCP package information.
 */
PackageInfoData getKCPInformation(QString pkgName)
{
  return Package::getKCPInformation(pkgName);
}

/*
 * Retrives outdated package names
 */
QStringList *getOutdatedPkgStringList()
{
  QStringList *res = Package::getOutdatedStringList();

  return res;
}

/*
 * Retrieves outdated AUR package names
 */
QStringList *getOutdatedAURStringList()
{
  QStringList *res = Package::getOutdatedAURStringList();

  return res;
}

/*
 * Executes given cmd with QProcess class
 */
QByteArray execCommand(QString cmd)
{
  return UnixCommand::execCommandAsNormalUserExt(cmd);
}

/*
 * Executes given cmd with a non blocking QProcess class
 */
void execCommandInAnotherThread(QString cmd)
{
  QProcess p;
  p.startDetached(cmd);
}

/*
 * Gets a temporary Yay-bin AUR helper to install one of the supported AUR helpers!
 * Returns true if all went fine
 *
 */
bool installTempYayHelper()
{
  bool res=true;
  QString url=QStringLiteral("https://github.com/Jguer/yay/releases/latest/");
  QString curl = QStringLiteral("curl -L %1 --output %2");
  QString tar = QStringLiteral("tar xzf %1 -C %2 %3");
  QString ln = QStringLiteral("ln -s %1 %2");
  QString removeLN = QStringLiteral("rm %1");
  QString htmlLatestYay=QStringLiteral("latestYay.html");
  QString octopiConfDir = QDir::homePath() + QDir::separator() + QLatin1String(".config/octopi");
  curl=curl.arg(url, octopiConfDir + QDir::separator() + htmlLatestYay);

  QProcess p;
  //First we download latest html page of yay-bin at github.com
  p.execute(curl);

  QFile file(octopiConfDir + QDir::separator() + htmlLatestYay);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return false;

  QString html = QString::fromUtf8(file.readAll());
  QString yayUrl;
  QString yayTarball;
  QString yayFile;

  //We have to find this kind of string:
  //<a href="/Jguer/yay/releases/download/v9.3.1/yay_9.3.1_x86_64.tar.gz"
  QRegularExpression re(QStringLiteral("<a href=\"(?<site>\\S+/(?<file>yay\\S+_x86_64.tar.gz))\""));
  QRegularExpressionMatch rem;
  if (html.contains(re, &rem))
  {
    yayUrl = rem.captured(QStringLiteral("site"));
    yayTarball = rem.captured(QStringLiteral("file"));
    yayFile = yayTarball + QDir::separator() + QLatin1String("yay");
    yayFile = yayFile.remove(QStringLiteral(".tar.gz"));
    yayUrl = QLatin1String("https://github.com") + yayUrl;
    file.close();
    file.remove();
  }
  else return false;

  //Let's download latest version of yay-bin tarball
  curl = QStringLiteral("curl -L %1 --output %2");
  curl=curl.arg(yayUrl, octopiConfDir + QDir::separator() + yayTarball);
  p.execute(curl);

  //Then we extract binary from tarball
  tar = tar.arg(octopiConfDir + QDir::separator() + yayTarball, octopiConfDir, yayFile);
  p.execute(tar);

  //Now we must symlink yay file to octopiConfDir/yay
  removeLN = removeLN.arg(octopiConfDir + QDir::separator() + QLatin1String("yay"));
  if (QFile::exists(octopiConfDir + QDir::separator() + QLatin1String("yay"))) p.execute(removeLN);

  //Now we must symlink yay file to octopiConfDir/yayFile/yay
  ln = ln.arg(octopiConfDir + QDir::separator() + yayFile, octopiConfDir + QDir::separator() + QLatin1String("yay"));
  p.execute(ln);

  //Remove tarball
  QFile::remove(octopiConfDir + QDir::separator() + yayTarball);
  return res;
}
