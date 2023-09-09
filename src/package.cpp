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

#include "package.h"
#include "unixcommand.h"
#include "strconstants.h"

#ifdef ALPM_BACKEND
  #include "alpmbackend.h"
#endif

#include <cctype>
#include <cstring>
#include <iostream>

#include <QTextStream>
#include <QList>
#include <QFile>
#include <QRegularExpression>
#include <QEventLoop>
#include <QtNetwork/QNetworkReply>

/*
 * This class abstracts all the relevant package information and services
 */

/*
 * Retrieves the basic package name, without latest "-" part
 */
QString Package::getBasePkgName(const QString &p)
{
  QString packageBaseName=QLatin1String("");
  int numberOfSegments = p.count(QLatin1Char('-'));

  if (numberOfSegments > 1)
  {
    int lastSep=p.lastIndexOf(QLatin1Char('-'));
    packageBaseName=p.left(lastSep);
  }

  return packageBaseName;
}

/*
 * Retrieves the basic package name, without version numbers
 */
QString Package::getBaseName(const QString& p)
{
	QString packageBaseName=QLatin1String("");
	QString aux(p);
  int numberOfSegments = p.count(QLatin1Char('-'))+1;
	int nameSegment = numberOfSegments-3;

	for (int i=1; i<= nameSegment; i++){
    int a=aux.indexOf(QLatin1String("-"));
    packageBaseName += aux.left(a);
		if (i<nameSegment) packageBaseName += QLatin1String("-");
		aux = aux.mid(a+1);
	}

  if (packageBaseName == QLatin1String("")) packageBaseName += p.left(p.indexOf(QLatin1String("-")));
	return packageBaseName;
}

/*
 * Given a QString, this method searches for a link pattern and inserts an URL html/ftp link tag
 * Returns the modified (or not) QString
 */
QString Package::makeURLClickable(const QString &s)
{
  QString sb = s;
  QRegularExpression rx(QStringLiteral("((ht|f)tp(s?))://(\\S)+[^\"|)|(|.|\\s|\\n]"));
  rx.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
	int search = 0;
	int ini = 0;

	//First we search for the 1st pattern: rx
  //while ( (ini = rx.indexIn( sb, search )) != -1 ){
  //	QString s1 = rx.cap();

  QRegularExpressionMatch match = rx.match(sb);
  while (match.hasMatch())
  {
    QString s1 = match.captured();
    QString ns;

    ns = QLatin1String("<a href=\"") + s1 + QLatin1String("\">") + s1 + QLatin1String("</a>");
    sb.replace( ini, s1.length(), ns);
    search = ini + (2*s1.length()) + 15;
    match = rx.match(sb, search);
	}

  return sb;
}

/*
 * This function was copied from ArchLinux Pacman project
 * A pow() implementation that is specialized for an integer base and small,
 * positive-only integer exponents.
 */
double Package::simplePow(int base, int exp)
{
  double result = 1.0;
  for(; exp > 0; exp--) {
    result *= base;
  }
  return result;
}

/*
 * Converts a size in kbytes to a readable QString representation
 */
QString Package::kbytesToSize(float Bytes)
{
  float gb = 1073741824;
  float mb = 1048576;
  float kb = 1024;
  QString res;

  if( Bytes >= gb )
    res = res.asprintf("%.2f GiB", (float)Bytes/gb);
  else if( Bytes >= mb && Bytes < gb )
    res = res.asprintf("%.2f MiB", (float)Bytes/mb);
  else if( Bytes >= kb && Bytes < mb )
    res = res.asprintf("%.2f KiB", (float)Bytes/kb);
  else if ( Bytes < kb)
    res = res.asprintf("%.2f Bytes", Bytes);
  else
    res = res.asprintf("%.2f Bytes", Bytes);

  return res;
}

/*
 * Parses optional package deps list and returns anchors for them
 */
QString Package::makeAnchorOfOptionalDep(const QString &optionalDeps)
{
  QString newDeps;
  QString newDep;
  QString name;
  QStringList ldeps = optionalDeps.split(QStringLiteral("<br>"), Qt::SkipEmptyParts);

  for(auto dep: ldeps)
  {
    int colon = dep.indexOf(QLatin1String(":"));
    if (colon != -1)
    {
      name = dep.left(colon).trimmed();

      newDep = QLatin1String("<a href=\"goto:") + name + QLatin1String("\">") + name + QLatin1String("</a> ") + dep.right(dep.length()-colon);
      newDeps += newDep + QLatin1String("<br>");
    }
    else
    {
      newDep = QLatin1String("<a href=\"goto:") + dep + QLatin1String("\">") + dep + QLatin1String("</a> ");
      newDeps += newDep + QLatin1String("<br>");
    }
  }

  newDeps.remove(QRegularExpression(QStringLiteral("<br>$")));
  return newDeps;
}

/*
 * Parses package list and returns anchors for those which does not have logical operators
 */
QString Package::makeAnchorOfPackage(const QString &packages)
{
  QString newDeps;
  QString newDep;
  QStringList ldeps = packages.split(QStringLiteral(" "), Qt::SkipEmptyParts);

  for(auto dep: ldeps)
  {
    if (!dep.contains(QLatin1String("=")) &&
        !dep.contains(QLatin1String("<")) &&
        !dep.contains(QLatin1String(">")))
    {
      newDeps += QLatin1String("<a href=\"goto:") + dep + QLatin1String("\">") + dep + QLatin1String("</a> ");
    }
    else
    {
      int p=-1;

      if (dep.contains(QLatin1String("<")))
      {
        if (dep.contains(QLatin1String("=")))
        {
          p = dep.indexOf(QLatin1String("<"));
          newDep = dep.left(p);
        }
        else
        {
          newDep = dep.left(p);
          dep.replace(QLatin1String("<"), QLatin1String("&lt;"));
        }
      }
      else if (dep.contains(QLatin1String(">")))
      {
        p = dep.indexOf(QLatin1String(">"));
        if (dep.contains(QLatin1String("=")))
        {
          p = dep.indexOf(QLatin1String(">"));
          newDep = dep.left(p);
        }
        else
        {
          newDep = dep.left(p);
          dep.replace(QLatin1String(">"), QLatin1String("&gt;"));
        }
      }
      else if (dep.contains(QLatin1String("=")))
      {
        p = dep.indexOf(QLatin1String("="));
        newDep = dep.left(p);
      }

      newDeps += QLatin1String("<a href=\"goto:") + newDep + QLatin1String("\">") + dep + QLatin1String("</a> ");
    }
  }

  newDeps = newDeps.trimmed();

  return newDeps;
}

/*
 * Retrieves the list of unrequired packages (those no other packages depends on)
 */
QSet<QString>* Package::getUnrequiredPackageList()
{
  QSet<QString>* res = new QSet<QString>();

  if (SettingsManager::hasPacmanBackend())
  {
    QString unrequiredPkgList = QString::fromUtf8(UnixCommand::getUnrequiredPackageList());
    QStringList packageTuples = unrequiredPkgList.split(QRegularExpression(QStringLiteral("\\n")), Qt::SkipEmptyParts);

    for(auto packageTuple: packageTuples)
    {
      QStringList parts = packageTuple.split(QLatin1Char(' '));
      {
        res->insert(parts[0]); //We only need the package name!
      }
    }
  }
#ifdef ALPM_BACKEND
  else
  {
    QStringList packageTuples = AlpmBackend::getUnrequiredList();

    for(auto packageTuple: packageTuples)
    {
      res->insert(packageTuple); //We only need the package name!
    }
  }
#endif

  return res;
}

/*
 * Retrieves the list of outdated packages (those which have newer versions available to download)
 */
QStringList *Package::getOutdatedStringList()
{
  QStringList * res = new QStringList();

  if (SettingsManager::hasPacmanBackend())
  {
    QString outPkgList = QString::fromUtf8(UnixCommand::getOutdatedPackageList());
    QStringList packageTuples = outPkgList.split(QRegularExpression(QStringLiteral("\\n")), Qt::SkipEmptyParts);
    QStringList ignorePkgList = UnixCommand::getIgnorePkgsFromPacmanConf();

    for(auto packageTuple: packageTuples)
    {
      QStringList parts = packageTuple.split(QLatin1Char(' '));
      {
        QString pkgName;
        pkgName = parts[0];

        //Let's ignore the "IgnorePkg" list of packages...
        if (!ignorePkgList.contains(pkgName))
        {
          res->append(pkgName); //We only need the package name!
        }
      }
    }

    res->sort();
  }
#ifdef ALPM_BACKEND
  else
  {
    QStringList packageTuples = AlpmBackend::getOutdatedList();
    QStringList ignorePkgList = UnixCommand::getIgnorePkgsFromPacmanConf();

    for(auto packageTuple: packageTuples)
    {
      //Let's ignore the "IgnorePkg" list of packages...
      if (!ignorePkgList.contains(packageTuple))
      {
        res->append(packageTuple);
      }
    }

    res->sort();
  }
#endif

  return res;
}

/*
 * Removes color codes from given str parameter
 */
QString Package::removeColorCodesFromStr(const QString &str)
{
  QString ret = str;

  ret = ret.remove(QStringLiteral("\033"));
  ret = ret.remove(QStringLiteral("[1;31m"));
  ret = ret.remove(QStringLiteral("[1;32m"));
  ret = ret.remove(QStringLiteral("[1;33m"));
  ret = ret.remove(QStringLiteral("[1;34m"));
  ret = ret.remove(QStringLiteral("[1;35m"));
  ret = ret.remove(QStringLiteral("[1;36m"));
  ret = ret.remove(QStringLiteral("[1;39m"));
  ret = ret.remove(QStringLiteral("[m"));
  ret = ret.remove(QStringLiteral("[0m"));
  ret = ret.remove(QStringLiteral("[1m"));
  ret = ret.remove(QStringLiteral("\u001B"));

  return ret;
}

/*
 * Retrieves the list of outdated Yaourt (AUR) packages
 * (those which have newer versions available to download)
 */
QStringList *Package::getOutdatedAURStringList()
{
  QStringList * res = new QStringList();

  if (getForeignRepositoryToolName() != ctn_PACAUR_TOOL &&
      getForeignRepositoryToolName() != ctn_TRIZEN_TOOL &&
      getForeignRepositoryToolName() != ctn_PIKAUR_TOOL &&
      getForeignRepositoryToolName() != ctn_YAY_TOOL &&
      getForeignRepositoryToolName() != ctn_PARU_TOOL &&
      getForeignRepositoryToolName() != ctn_KCP_TOOL) return res;

  QString outPkgList = removeColorCodesFromStr(QString::fromUtf8(UnixCommand::getOutdatedAURPackageList()));
  outPkgList = outPkgList.trimmed();

  QStringList packageTuples = outPkgList.split(QRegularExpression(QStringLiteral("\\n")), Qt::SkipEmptyParts);
  QStringList ignorePkgList = UnixCommand::getIgnorePkgsFromPacmanConf();

  for(auto packageTuple: packageTuples)
  {
    QStringList parts = packageTuple.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    {
      if (getForeignRepositoryToolName() == ctn_TRIZEN_TOOL ||
          getForeignRepositoryToolName() == ctn_PIKAUR_TOOL ||
          getForeignRepositoryToolName() == ctn_YAY_TOOL ||
          getForeignRepositoryToolName() == ctn_PARU_TOOL ||
          getForeignRepositoryToolName() == ctn_KCP_TOOL)
      {
        QString pkgName;
        pkgName = parts[0];

        if (pkgName.contains(StrConstants::getForeignRepositoryTargetPrefix(), Qt::CaseInsensitive))
        {          
          pkgName = pkgName.remove(StrConstants::getForeignRepositoryTargetPrefix());
          //Let's ignore the "IgnorePkg" list of packages...
          if (!ignorePkgList.contains(pkgName))
          {
            res->append(pkgName); //We only need the package name!
          }
        }
        else //We have a TRIZEN output
        {
          if (!ignorePkgList.contains(pkgName))
          {
            res->append(pkgName); //We only need the package name!
          }
        }
      }
      else if (getForeignRepositoryToolName() == ctn_PACAUR_TOOL)
      {
        QString pkgName;
        if (parts.count() >= 2)
        {
          if (parts[1] != StrConstants::getForeignPkgRepositoryName())
            continue;

          pkgName = parts[2];

          //Let's ignore the "IgnorePkg" list of packages...
          if (!ignorePkgList.contains(pkgName))
          {
            res->append(pkgName); //We only need the package name!
          }
        }
      }
    }
  }

  res->sort();
  return res;
}

/*
 * Retrieves the list of all package groups available
 */
QStringList *Package::getPackageGroups()
{
  QString packagesFromGroup = QString::fromUtf8(UnixCommand::getPackageGroups());
  QStringList packageTuples = packagesFromGroup.split(QRegularExpression(QStringLiteral("\\n")), Qt::SkipEmptyParts);
  QStringList * res = new QStringList();

  for(auto packageTuple: packageTuples)
  {
    packageTuple = packageTuple.trimmed();
    if (!packageTuple.isEmpty())
    {
      res->append(packageTuple); //We only need the package name!
    }
  }

  res->removeDuplicates();
  res->sort();
  return res;
}

/*
 * Retrieves the list of packages from a given group name
 */
QStringList *Package::getPackagesOfGroup(const QString &groupName)
{
  QString packagesFromGroup = QString::fromUtf8(UnixCommand::getPackagesFromGroup(groupName));
  QStringList packageTuples = packagesFromGroup.split(QRegularExpression(QStringLiteral("\\n")), Qt::SkipEmptyParts);
  QStringList * res = new QStringList();

  for(auto packageTuple: packageTuples)
  {
    QStringList parts = packageTuple.split(QLatin1Char(' '));
    res->append(parts[1]); //We only need the package name!
  }

  return res;
}

/*
 * Retrieves the list of targets needed to upgrade the entire system OR
 * to install/reinstall/upgrade a given package
 */
QList<PackageListData> *Package::getTargetUpgradeList(const QString &pkgName)
{
  QString targets = QString::fromUtf8(UnixCommand::getTargetUpgradeList(pkgName));
  QStringList packageTuples = targets.split(QRegularExpression(QStringLiteral("\\n")), Qt::SkipEmptyParts);
  QList<PackageListData> *res = new QList<PackageListData>();
  packageTuples.sort();

  for(auto packageTuple: packageTuples)
  {
    //TODO: Need to handle when this list has "::" conflict items!
    if(packageTuple.indexOf(QLatin1String("::"))!=-1)
    {
      continue;
    }

    PackageListData ld;
    QStringList data = packageTuple.split(QStringLiteral(" "));
    if (data.count() == 3)
    {
      ld = PackageListData(data.at(0), data.at(1), data.at(2));
    }
    else if (data.count() == 1)
    {
      ld = PackageListData(data.at(0), QLatin1String(""), QStringLiteral("0"));
    }

    res->append(ld);
  }

  return res;
}

/*
 * Retrieves the list of targets needed to be removed with the given package
 */
QStringList *Package::getTargetRemovalList(const QString &pkgName, const QString &removeCommand)
{
  QString targets = QString::fromUtf8(UnixCommand::getTargetRemovalList(pkgName, removeCommand));
  QStringList packageTuples = targets.split(QRegularExpression(QStringLiteral("\\n")), Qt::SkipEmptyParts);
  QStringList * res = new QStringList();

  for(auto packageTuple: packageTuples)
  {
    res->append(packageTuple);
  }

  res->sort();
  return res;
}

/*
 *Retrieves the list of foreign packages (those installed from unknown repositories like AUR)
 */
QList<PackageListData> *Package::getForeignPackageList()
{
  QList<PackageListData> * res = new QList<PackageListData>();

  if (SettingsManager::hasPacmanBackend())
  {
    QString foreignPkgList = QString::fromUtf8(UnixCommand::getForeignPackageList());
    QStringList packageTuples = foreignPkgList.split(QRegularExpression(QStringLiteral("\\n")), Qt::SkipEmptyParts);

    for(auto packageTuple: packageTuples)
    {
      QStringList parts = packageTuple.split(QLatin1Char(' '));
      if (parts.size() == 2)
      {
        QString desc=parts[0] + QLatin1String(" ") + Package::getInformationDescription(parts[0], true);
        res->append(PackageListData(parts[0], QLatin1String(""), parts[1], desc, ectn_FOREIGN));
      }
    }
  }
#ifdef ALPM_BACKEND
  else
  {
    bool ok;
    QString instSize, buildDate, installDate, license, installReason;
    QStringList packageTuples = AlpmBackend::getForeignList();

    for(auto packageTuple: packageTuples)
    {
      QStringList parts = packageTuple.split(QStringLiteral("<o'o>"));
      //NAME, REPO, VERSION, "NAME DESCRIPTION", FOREIGN
      //res->append(PackageListData(parts[0], QLatin1String(""), parts[1], parts[0] + QLatin1Char(' ') + parts[2], ectn_FOREIGN));
      instSize=parts[3];
      buildDate=parts[4];
      installDate=parts[5];
      license=parts[6];
      installReason=parts[7];

      res->append(PackageListData(parts[0], QLatin1String(""), parts[1], parts[0] + QLatin1Char(' ') + parts[2],
          instSize.toDouble(&ok), buildDate.toDouble(&ok), installDate.toDouble(&ok), license, installReason, ectn_FOREIGN));
    }
  }
#endif

  return res;
}

/*
 * Retrieves the list of all available packages in the database (installed + non-installed)
 */
QList<PackageListData> * Package::getPackageList(const QString &packageName, const QHash<QString, QString> *checkUpdatesOutdatedPackages)
{
#ifndef ALPM_BACKEND
  Q_UNUSED(checkUpdatesOutdatedPackages)
#endif

  //archlinuxfr/yaourt 1.2.2-1 [installed]
  //    A pacman wrapper with extended features and AUR support
  //community/libfm 1.1.0-4 (lxde) [installed: 1.1.0-3]

  QList<PackageListData> * res = new QList<PackageListData>();

  if (SettingsManager::hasPacmanBackend())
  {
    QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion;
    PackageStatus pkgStatus;
    QString pkgList = QString::fromUtf8(UnixCommand::getPackageList(packageName));
    QStringList packageTuples = pkgList.split(QRegularExpression(QStringLiteral("\\n")), Qt::SkipEmptyParts);

    if(!pkgList.isEmpty())
    {
      pkgDescription = QLatin1String("");
      for(auto packageTuple: packageTuples)
      {
        if (!packageTuple[0].isSpace())
        {
          //Do we already have a description?
          if (pkgDescription != QLatin1String(""))
          {
            pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;

            PackageListData pld =
                PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);

            if (packageName.isEmpty() || pkgName == packageName)
            {
              res->append(pld);
            }

            pkgDescription = QLatin1String("");
          }

          //First we get repository and name!
          QStringList parts = packageTuple.split(QLatin1Char(' '));
          QString repoName = parts[0];
          int a = repoName.indexOf(QLatin1String("/"));
          pkgRepository = repoName.left(a);
          pkgName = repoName.mid(a+1);
          pkgVersion = parts[1];

          if(packageTuple.indexOf(QLatin1String("[installed]")) != -1)
          {
            //This is an installed package
            pkgStatus = ectn_INSTALLED;
            pkgOutVersion = QLatin1String("");
          }
          else if (packageTuple.indexOf(QLatin1String("[installed:")) != -1)
          {
            //This is an outdated installed package
            pkgStatus = ectn_OUTDATED;

            int i = packageTuple.indexOf(QLatin1String("[installed:"));
            pkgOutVersion = packageTuple.mid(i+11);
            pkgOutVersion = pkgOutVersion.remove(QLatin1Char(']')).trimmed();
          }
          else
          {
            //This is an uninstalled package
            pkgStatus = ectn_NON_INSTALLED;
            pkgOutVersion = QLatin1String("");
          }
        }
        else
        {
          //This is a description!
          if (!packageTuple.trimmed().isEmpty())
            pkgDescription += packageTuple.trimmed();
          else
            pkgDescription += QLatin1String(" "); //StrConstants::getNoDescriptionAvailabe();
        }
      }

      //And adds the very last package...
      pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;
      PackageListData pld =
          PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);

      if (packageName.isEmpty() || pkgName == packageName)
      {
        res->append(pld);
      }
    }
  }
#ifdef ALPM_BACKEND
  else
  {
    QString pkgName, pkgRepository, pkgVersion, pkgDescription,
        pkgOutVersion, pkgSize, pkgISize, pkgBDate, pkgIDate, pkgLicense, pkgInstallReason;
    PackageStatus pkgStatus;
    double pkgDownSize=0;
    double pkgInstSize=0;
    double pkgBuildDate=0;
    double pkgInstallDate=0;

    QStringList pkgList = AlpmBackend::getPackageList();
    bool ok;
    bool hasOutdatedPackages = checkUpdatesOutdatedPackages->count() > 0;

    pkgDescription = QLatin1String("");
    for(auto packageTuple: pkgList)
    {
      if (!packageTuple[0].isSpace())
      {
        //Do we already have a description?
        if (pkgDescription != QLatin1String(""))
        {
          pkgDescription = pkgName + QLatin1Char(' ') + pkgDescription;

          PackageListData pld =
              PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription,
                              pkgStatus, pkgDownSize, pkgInstSize, pkgBuildDate,
                              pkgInstallDate, pkgLicense, pkgInstallReason, pkgOutVersion);

          res->append(pld);
          pkgDescription = QLatin1String("");
        }

        //First we get repository and name!
        QStringList parts = packageTuple.split(QStringLiteral("<o'o>"));
        pkgRepository = parts[1];
        pkgName = parts[2];
        pkgVersion = parts[3];
        pkgSize = parts[5];
        pkgISize = parts[6];
        pkgBDate = parts[7];
        pkgIDate = parts[8];
        pkgLicense = parts[9];
        pkgInstallReason = parts[10];
        pkgDownSize = pkgSize.toLong(&ok);
        pkgInstSize = pkgISize.toLong(&ok);
        pkgBuildDate = pkgBDate.toLong(&ok);
        pkgInstallDate = pkgIDate.toLong(&ok);

        if(parts[0] == QLatin1Char('i'))
        {
          //This is an installed package
          if (!hasOutdatedPackages)
          {
            pkgStatus = ectn_INSTALLED;
            pkgOutVersion = QLatin1String("");
          }
          else
          {
            QString newVersion = checkUpdatesOutdatedPackages->value(pkgName);
            if (newVersion.isEmpty())
            {
              pkgStatus = ectn_INSTALLED;
              pkgOutVersion = QLatin1String("");
            }
            else
            {
              pkgStatus = ectn_OUTDATED;
              pkgOutVersion = pkgVersion;
              pkgVersion = newVersion;
            }
          }
        }
        else if (parts[0] == QLatin1String("o"))
        {
          //This is an outdated installed package
          pkgStatus = ectn_OUTDATED;
          pkgOutVersion = parts[4];
        }
        else if (parts[0] == QLatin1Char('n'))
        {
          //This is an uninstalled package
          pkgStatus = ectn_NON_INSTALLED;
          pkgOutVersion = QLatin1String("");
        }
      }
      else
      {
        //This is a description!
        if (!packageTuple.trimmed().isEmpty())
          pkgDescription += packageTuple.trimmed();
        else
          pkgDescription += QLatin1Char(' ');
      }
    }

    //And adds the very last package...
    pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;
    PackageListData pld =
        PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription,
                        pkgStatus, pkgDownSize, pkgInstSize, pkgBuildDate,
                        pkgInstallDate, pkgLicense, pkgInstallReason, pkgOutVersion);

    res->append(pld);
  }
#endif

  return res;
}

/*
 * Retrieves the list of all AUR packages in the database (installed + non-installed)
 * given the search parameter
 */
QList<PackageListData> * Package::getForeignToolPackageList(const QString &searchString)
{
  //aur/yaourt 1.2.2-1 [installed]
  //    A pacman wrapper with extended features and AUR support
  //aur/libfm 1.1.0-4 (lxde) [installed: 1.1.0-3]

  QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion;
  int pkgVotes;
  bool addPkg;
  PackageStatus pkgStatus;
  QList<PackageListData> * res = new QList<PackageListData>();

  if (UnixCommand::getLinuxDistro() != ectn_KAOS && searchString.isEmpty())
    return res;

  QString aurTool = getForeignRepositoryToolName();
  QString auxSearchString=searchString;
  auxSearchString.remove(QLatin1Char('^'));
  auxSearchString.remove(QLatin1Char('$'));
  QString pkgList = QString::fromUtf8(UnixCommand::getAURPackageList(auxSearchString));
  QStringList packageTuples = pkgList.split(QRegularExpression(QStringLiteral("\\n")), Qt::SkipEmptyParts);

  if (aurTool == ctn_KCP_TOOL)
  {
    return getKcpPackageList(searchString, packageTuples);
  }
  else if (aurTool == ctn_YAY_TOOL)
  {
    return getYayPackageList(searchString, packageTuples);
  }
  else if (aurTool == ctn_PARU_TOOL)
  {
    //return getAURPackageList(searchString);
    return getParuPackageList(searchString, packageTuples);
  }

  pkgDescription = QLatin1String("");
  for(auto packageTuple: qAsConst(packageTuples))
  {
    if (packageTuple[0].isNumber())
    {
      int space=packageTuple.indexOf(QLatin1String(" "));
      packageTuple = packageTuple.mid(space+1);
    }

    if (!packageTuple[0].isSpace())
    {
      addPkg=true;
      //Do we already have a description?
      if (pkgDescription != QLatin1String(""))
      {
        //Tests if pkgName matches "^" or "$" criteria
        if (!searchString.isEmpty() && (searchString.at(0) == QLatin1Char('^') || searchString.at(searchString.length()-1) == QLatin1Char('$')))
        {
          QRegularExpression re(searchString);
          QRegularExpressionMatch match = re.match(pkgName);

          if (!match.hasMatch()) addPkg=false;
        }

        pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;

        PackageListData pld =
            PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);

        pld.popularity = pkgVotes;

        if (addPkg) res->append(pld);
        pkgDescription = QLatin1String("");
      }

      //First we get repository and name!
      QStringList parts = packageTuple.split(QLatin1Char(' '));
      QString repoName = parts[0];
      int a = repoName.indexOf(QLatin1String("/"));
      pkgRepository = repoName.left(a);

      if (pkgRepository != StrConstants::getForeignPkgRepositoryName())
      {
        res->removeAt(res->count()-1);
        continue;
      }

      pkgRepository = StrConstants::getForeignPkgRepositoryName().toUpper();
      pkgName = repoName.mid(a+1);
      pkgVersion = parts[1];

      QStringList strVotes = parts.filter(QStringLiteral("("));
      //Let's see if it's not a Trizen style
      if (strVotes.isEmpty())
        strVotes = parts.filter(QStringLiteral("+]"));

      pkgVotes = 0;

      //Chakra does not have popularity support in CCR
      if (aurTool == ctn_TRIZEN_TOOL)
      {
        if (!strVotes.first().isEmpty())
          pkgVotes = strVotes.first().replace(QLatin1Char('['), QLatin1String("")).replace(QLatin1Char(']'), QLatin1String("")).replace(QLatin1Char('+'), QLatin1String("")).toInt();
        else
          pkgVotes = 0;
      }
      else if (aurTool != ctn_CHASER_TOOL && aurTool != ctn_PACAUR_TOOL && aurTool != ctn_PIKAUR_TOOL && strVotes.count() > 0)
      {
        if (!strVotes.first().isEmpty())
          pkgVotes = strVotes.first().replace(QLatin1Char('('), QLatin1String("")).replace(QLatin1Char(')'), QLatin1String("")).toInt();
        else
          pkgVotes = 0;
      }
      else if ( (aurTool == ctn_PACAUR_TOOL || aurTool == ctn_PIKAUR_TOOL) && strVotes.count() > 0)
      {
        if (!strVotes.first().isEmpty())
        {
          QString str = strVotes.first();
          int comma = str.indexOf(QLatin1String(","));
          pkgVotes = str.mid(1, comma-1).toInt();
        }
        else pkgVotes = 0;
      }

      if (aurTool == ctn_PIKAUR_TOOL)
      {
        if(packageTuple.indexOf(QLatin1String(" [")) != -1)
        {
          if (packageTuple.indexOf(QLatin1String("installed: ")) != -1)
          {
            int i = packageTuple.indexOf(QLatin1String(": "));
            pkgOutVersion = packageTuple.mid(i+2);
            pkgOutVersion = pkgOutVersion.remove(QRegularExpression(QStringLiteral("\\].*"))).trimmed();

            //Compare actual and new version
            char const * pkgOutVersion_temp = pkgOutVersion.toStdString().c_str();
            char const * pkgVersion_temp = pkgVersion.toStdString().c_str();
            int pkgIsUptodate = alpm_pkg_vercmp(pkgOutVersion_temp, pkgVersion_temp);
            if (pkgIsUptodate == -1)
            {
              //This is an outdated installed package
              pkgStatus = ectn_FOREIGN_OUTDATED;
            }
            else
            {
              //This is an installed package
              pkgStatus = ectn_FOREIGN;
              pkgOutVersion = QLatin1String("");
            }
          }
          else
          {
            //This is an installed package
            pkgStatus = ectn_FOREIGN;
            pkgOutVersion = QLatin1String("");
          }
        }
        else
        {
          //This is an uninstalled package
          pkgStatus = ectn_NON_INSTALLED;
          pkgOutVersion = QLatin1String("");
        }
      }
      else if(packageTuple.indexOf(QLatin1String("[installed]")) != -1)
      {
        //This is an installed package
        pkgStatus = ectn_FOREIGN;
        pkgOutVersion = QLatin1String("");
      }
      else if (packageTuple.indexOf(QLatin1String("[installed:")) != -1)
      {
        int i = packageTuple.indexOf(QLatin1String(": "));
        pkgOutVersion = packageTuple.mid(i+2);
        pkgOutVersion = pkgOutVersion.remove(QStringLiteral("("));
        pkgOutVersion = pkgOutVersion.remove(QStringLiteral(")"));
        pkgOutVersion = pkgOutVersion.remove(QRegularExpression(QStringLiteral("\\].*"))).trimmed();

        //Compare actual and new version
        char const * pkgOutVersion_temp = pkgOutVersion.toStdString().c_str();
        char const * pkgVersion_temp = pkgVersion.toStdString().c_str();
        int pkgIsUptodate = alpm_pkg_vercmp(pkgOutVersion_temp, pkgVersion_temp);

        if (pkgIsUptodate == -1)
        {
          //This is an outdated installed package
          pkgStatus = ectn_FOREIGN_OUTDATED;
        }
        else
        {
          //This is an installed package
          pkgStatus = ectn_FOREIGN;
          pkgOutVersion = QLatin1String("");
        }
      }
      else
      {
        //This is an uninstalled package
        pkgStatus = ectn_NON_INSTALLED;
        pkgOutVersion = QLatin1String("");
      }
    }
    else
    {            
      //This is a description!
      if (!packageTuple.trimmed().isEmpty())
        pkgDescription += packageTuple.trimmed();
      else
      {
        pkgDescription += QLatin1String(" ");
      }
    }
  }

  bool addLast=false;

  //And adds the very last package...
  if (packageTuples.count() > 1)
  {
    addLast=true;
    //Tests if pkgName matches "^" or "$" criteria
    if (!searchString.isEmpty() && (searchString.at(0) == QLatin1Char('^') || searchString.at(searchString.length()-1) == QLatin1Char('$')))
    {
      QRegularExpression re(searchString);
      QRegularExpressionMatch match = re.match(pkgName);

      if (!match.hasMatch()) addLast = false;
    }
  }

  if (addLast)
  {
    pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;
    PackageListData pld =
        PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);
    pld.popularity = pkgVotes;

    res->append(pld);
  }

  if (res->count() > 0 && res->at(0).repository !=
      StrConstants::getForeignPkgRepositoryName().toUpper()) res->removeAt(0);

  return res;
}

/*
 * Retrieves the list of all KCP packages in the database using Kcp (installed + non-installed)
 * given the search parameter
 */
QList<PackageListData> *Package::getKcpPackageList(const QString &searchString, const QStringList &packageTuples)
{
  QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion;
  bool addPkg;
  int pkgVotes;
  PackageStatus pkgStatus;
  QList<PackageListData> * res = new QList<PackageListData>();

  pkgDescription = QLatin1String("");
  for(auto packageTuple: packageTuples)
  {
    if (packageTuple[0].isNumber())
    {
      int space=packageTuple.indexOf(QLatin1String(" "));
      packageTuple = packageTuple.mid(space+1);
    }

    if (packageTuple[0] != QLatin1Char('\t')) //!packageTuple[0].isSpace())
    {
      addPkg=true;
      //Do we already have a description?
      if (pkgDescription != QLatin1String(""))
      {
        //Tests if pkgName matches "^" or "$" criteria
        if (!searchString.isEmpty() && (searchString.at(0) == QLatin1Char('^') || searchString.at(searchString.length()-1) == QLatin1Char('$')))
        {
          QRegularExpression re(searchString);
          QRegularExpressionMatch match = re.match(pkgName);

          if (!match.hasMatch()) addPkg=false;
        }

        pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;

        PackageListData pld =
            PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);

        pld.popularity = pkgVotes;

        if (addPkg) res->append(pld);
        pkgDescription = QLatin1String("");
      }

      //First we get repository and name!
      QStringList parts = packageTuple.split(QLatin1Char(' '));
      parts[0] = parts[0].remove(QStringLiteral("[1;35m"));

      QString repoName = parts[0];
      int a = repoName.indexOf(QLatin1String("/"));
      pkgRepository = repoName.left(a);

      if (pkgRepository != StrConstants::getForeignPkgRepositoryName())
      {
        res->removeAt(res->count()-1);
        continue;
      }

      pkgRepository = StrConstants::getForeignPkgRepositoryName().toUpper();
      pkgName = repoName.mid(a+1);
      pkgVersion = parts[1];

      QStringList strVotes = parts.filter(QStringLiteral("("));

      if (strVotes.isEmpty())
        strVotes = parts.filter(QStringLiteral("+]"));

      pkgVotes = 0;

      if (strVotes.count() > 0)
      {
        if (!strVotes.first().isEmpty())
          pkgVotes = strVotes.first().replace(QLatin1Char('('), QLatin1String("")).replace(QLatin1Char(')'), QLatin1String("")).toInt();
        else
          pkgVotes = 0;
      }

      if(packageTuple.indexOf(QLatin1String("[installed]")) != -1)
      {
        //This is an installed package
        pkgStatus = ectn_FOREIGN;
        pkgOutVersion = QLatin1String("");
      }
      else if (packageTuple.indexOf(QLatin1String("[installed:")) != -1)
      {
        int i = packageTuple.indexOf(QLatin1String(": "));
        pkgOutVersion = packageTuple.mid(i+2);
        pkgOutVersion = pkgOutVersion.remove(QStringLiteral("("));
        pkgOutVersion = pkgOutVersion.remove(QStringLiteral(")"));
        pkgOutVersion = pkgOutVersion.remove(QRegularExpression(QStringLiteral("\\].*"))).trimmed();

        //Compare actual and new version
        char const * pkgOutVersion_temp = pkgOutVersion.toStdString().c_str();
        char const * pkgVersion_temp = pkgVersion.toStdString().c_str();
        int pkgIsUptodate = alpm_pkg_vercmp(pkgOutVersion_temp, pkgVersion_temp);

        if (pkgIsUptodate == -1)
        {
          //This is an outdated installed package
          pkgStatus = ectn_FOREIGN_OUTDATED;
        }
        else
        {
          //This is an installed package
          pkgStatus = ectn_FOREIGN;
          pkgOutVersion = QLatin1String("");
        }
      }
      else
      {
        //This is an uninstalled package
        pkgStatus = ectn_NON_INSTALLED;
        pkgOutVersion = QLatin1String("");
      }
    }
    else
    {
      //This is a description!
      pkgDescription = packageTuple;
      pkgDescription.remove(QStringLiteral("\t"));

      if (pkgDescription.isEmpty())
        pkgDescription += QLatin1String(" ");
    }
  }

  bool addLast=false;

  //And adds the very last package...
  if (packageTuples.count() > 1)
  {
    addLast=true;
    //Tests if pkgName matches "^" or "$" criteria
    if (!searchString.isEmpty() && (searchString.at(0) == QLatin1Char('^') || searchString.at(searchString.length()-1) == QLatin1Char('$')))
    {
      QRegularExpression re(searchString);
      QRegularExpressionMatch match = re.match(pkgName);

      if (!match.hasMatch()) addLast = false;
    }
  }

  if (addLast)
  {
    pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;
    PackageListData pld =
        PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);
    pld.popularity = pkgVotes;

    res->append(pld);
  }

  if (res->count() > 0 && res->at(0).repository !=
      StrConstants::getForeignPkgRepositoryName().toUpper()) res->removeAt(0);

  return res;
}

/*
 * Retrieves the list of all AUR packages in the database using Yay (installed + non-installed)
 * given the search parameter
 */
QList<PackageListData> *Package::getYayPackageList(const QString& searchString, const QStringList &packageTuples)
{
  QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion;
  bool addPkg;
  int pkgVotes;
  PackageStatus pkgStatus;
  QList<PackageListData> * res = new QList<PackageListData>();

  pkgDescription = QLatin1String("");
  for(auto packageTuple: packageTuples)
  {
    if (packageTuple[0].isNumber())
    {
      int space=packageTuple.indexOf(QLatin1String(" "));
      packageTuple = packageTuple.mid(space+1);
    }

    if (!packageTuple[0].isSpace())
    {
      addPkg=true;
      //Do we already have a description?
      if (pkgDescription != QLatin1String(""))
      {
        //Tests if pkgName matches "^" or "$" criteria
        if (searchString.at(0) == QLatin1Char('^') || searchString.at(searchString.length()-1) == QLatin1Char('$'))
        {
          QRegularExpression re(searchString);
          QRegularExpressionMatch match = re.match(pkgName);

          if (!match.hasMatch()) addPkg=false;
        }

        pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;

        PackageListData pld =
            PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);

        pld.popularity = pkgVotes;

        if (addPkg) res->append(pld);
        pkgDescription = QLatin1String("");
      }

      //First we get repository and name!
      QStringList parts = packageTuple.split(QLatin1Char(' '));
      QString repoName = parts[0];
      int a = repoName.indexOf(QLatin1String("/"));
      pkgRepository = repoName.left(a);

      if (pkgRepository != StrConstants::getForeignPkgRepositoryName())
      {
        res->removeAt(res->count()-1);
        continue;
      }

      pkgRepository = StrConstants::getForeignPkgRepositoryName().toUpper();
      pkgName = repoName.mid(a+1);
      pkgVersion = parts[1];

      QStringList strVotes = parts.filter(QStringLiteral("(+"));

      pkgVotes = 0;
      if (strVotes.count() > 0)
      {
        if (!strVotes.first().isEmpty())
        {
          //(+65 1.69%)
          strVotes.first().replace(QLatin1Char('('), QLatin1String("")).replace(QLatin1Char('+'), QLatin1String(""));
          int space = strVotes.first().indexOf(QLatin1String(" "));
          strVotes = strVotes.mid(0, space);
          if (!strVotes.isEmpty()) pkgVotes = strVotes.first().toInt();
        }
        else
          pkgVotes = 0;
      }

      if(packageTuple.indexOf(QLatin1String("(Installed)")) != -1)
      {
        //This is an installed package
        pkgStatus = ectn_FOREIGN;
        pkgOutVersion = QLatin1String("");
      }
      else if (packageTuple.indexOf(QLatin1String("(Installed:")) != -1)
      {
        //This is an outdated installed package
        pkgStatus = ectn_FOREIGN_OUTDATED;

        int i = packageTuple.indexOf(QLatin1String("(Installed:"));
        pkgOutVersion = packageTuple.mid(i+11);
        pkgOutVersion = pkgOutVersion.remove(QStringLiteral("("));
        pkgOutVersion = pkgOutVersion.remove(QStringLiteral(")"));
        pkgOutVersion = pkgOutVersion.remove(QRegularExpression(QStringLiteral("\\].*"))).trimmed();
      }
      else
      {
        //This is an uninstalled package
        pkgStatus = ectn_NON_INSTALLED;
        pkgOutVersion = QLatin1String("");
      }
    }
    else
    {
      //This is a description!
      //else
      {
        if (!packageTuple.trimmed().isEmpty())
          pkgDescription += packageTuple.trimmed();
        else
        {
          pkgDescription += QLatin1String(" ");
        }
      }
    }
  }

  bool addLast=false;

  //And adds the very last package...
  if (packageTuples.count() > 1)
  {
    addLast=true;
    //Tests if pkgName matches "^" or "$" criteria
    if (searchString.at(0) == QLatin1Char('^') || searchString.at(searchString.length()-1) == QLatin1Char('$'))
    {
      QRegularExpression re(searchString);
      QRegularExpressionMatch match = re.match(pkgName);

      if (!match.hasMatch()) addLast = false;
    }
  }

  if (addLast)
  {
    pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;
    PackageListData pld =
        PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);
    pld.popularity = pkgVotes;

    res->append(pld);
  }

  if (res->count() > 0 && res->at(0).repository !=
      StrConstants::getForeignPkgRepositoryName().toUpper()) res->removeAt(0);

  return res;
}

/*
 * Retrieves the list of all AUR packages in the database using Paru (installed + non-installed)
 * given the search parameter
 */
QList<PackageListData> *Package::getParuPackageList(const QString &searchString, const QStringList &packageTuples)
{
  QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion;
  bool addPkg;
  int pkgVotes;
  PackageStatus pkgStatus;
  QList<PackageListData> * res = new QList<PackageListData>();

  pkgDescription = QLatin1String("");
  for(auto packageTuple: packageTuples)
  {
    if (packageTuple[0].isNumber())
    {
      int space=packageTuple.indexOf(QLatin1String(" "));
      packageTuple = packageTuple.mid(space+1);
    }

    if (!packageTuple[0].isSpace())
    {
      addPkg=true;
      //Do we already have a description?
      if (pkgDescription != QLatin1String(""))
      {
        //Tests if pkgName matches "^" or "$" criteria
        if (searchString.at(0) == QLatin1Char('^') || searchString.at(searchString.length()-1) == QLatin1Char('$'))
        {
          QRegularExpression re(searchString);
          QRegularExpressionMatch match = re.match(pkgName);

          if (!match.hasMatch()) addPkg=false;
        }

        pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;

        PackageListData pld =
            PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);

        pld.popularity = pkgVotes;

        if (addPkg) res->append(pld);
        pkgDescription = QLatin1String("");
      }

      //First we get repository and name!
      QStringList parts = packageTuple.split(QLatin1Char(' '));
      QString repoName = parts[0];
      int a = repoName.indexOf(QLatin1String("/"));
      pkgRepository = repoName.left(a);

      if (pkgRepository != StrConstants::getForeignPkgRepositoryName())
      {
        res->removeAt(res->count()-1);
        continue;
      }

      pkgRepository = StrConstants::getForeignPkgRepositoryName().toUpper();
      pkgName = repoName.mid(a+1);
      pkgVersion = parts[1];

      QStringList strVotes = parts.filter(QStringLiteral("[+"));

      pkgVotes = 0;
      if (strVotes.count() > 0)
      {
        if (!strVotes.first().isEmpty())
        {
          //[+65 ~1.69%]
          strVotes.first().replace(QLatin1Char('['), QLatin1String("")).replace(QLatin1Char('+'), QLatin1String(""));
          int space = strVotes.first().indexOf(QLatin1String(" "));
          strVotes = strVotes.mid(0, space);
          if (!strVotes.isEmpty()) pkgVotes = strVotes.first().toInt();
        }
        else
          pkgVotes = 0;
      }

      if(packageTuple.indexOf(QLatin1String("[Installed]")) != -1)
      {
        //This is an installed package
        pkgStatus = ectn_FOREIGN;
        pkgOutVersion = QLatin1String("");
      }
      else if (packageTuple.indexOf(QLatin1String("[Installed:")) != -1)
      {
        //This is an outdated installed package
        pkgStatus = ectn_FOREIGN_OUTDATED;

        int i = packageTuple.indexOf(QLatin1String("[Installed:"));
        pkgOutVersion = packageTuple.mid(i+11);
        pkgOutVersion = pkgOutVersion.remove(QStringLiteral("["));
        pkgOutVersion = pkgOutVersion.remove(QStringLiteral("["));
        pkgOutVersion = pkgOutVersion.remove(QRegularExpression(QStringLiteral("\\].*"))).trimmed();
      }
      else
      {
        //This is an uninstalled package
        pkgStatus = ectn_NON_INSTALLED;
        pkgOutVersion = QLatin1String("");
      }
    }
    else
    {
      //This is a description!
      if (!packageTuple.trimmed().isEmpty())
        pkgDescription += packageTuple.trimmed();
      else
      {
        pkgDescription += QLatin1String(" ");
      }
    }
  }

  bool addLast=false;

  //And adds the very last package...
  if (packageTuples.count() > 1)
  {
    addLast=true;
    //Tests if pkgName matches "^" or "$" criteria
    if (searchString.at(0) == QLatin1Char('^') || searchString.at(searchString.length()-1) == QLatin1Char('$'))
    {
      QRegularExpression re(searchString);
      QRegularExpressionMatch match = re.match(pkgName);

      if (!match.hasMatch()) addLast = false;
    }
  }

  if (addLast)
  {
    pkgDescription = pkgName + QLatin1String(" ") + pkgDescription;
    PackageListData pld =
        PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);
    pld.popularity = pkgVotes;

    res->append(pld);
  }

  if (res->count() > 0 && res->at(0).repository !=
      StrConstants::getForeignPkgRepositoryName().toUpper()) res->removeAt(0);

  return res;
}

/*
 * Given a QString containing the output of pacman -Si/Qi (pkgInfo),
 * this method returns the contents of the given field (ex: description)
 */
QString Package::extractFieldFromInfo(const QString &field, const QString &pkgInfo)
{
  int fieldPos = pkgInfo.indexOf(field);
  QString aux;

  if (fieldPos > 0)
  {
    int fieldEnd;
    if(field == QLatin1String("Optional Deps"))
    {
      fieldPos = pkgInfo.indexOf(QLatin1String(":"), fieldPos+1);
      fieldPos+=2;
      aux = pkgInfo.mid(fieldPos);
      fieldEnd = aux.indexOf(QLatin1String("Conflicts With"));
      int fieldEnd2 = aux.indexOf(QLatin1String("Required By"));

      if(fieldEnd > fieldEnd2 && fieldEnd2 != -1) fieldEnd = fieldEnd2;

      aux = aux.left(fieldEnd).trimmed();
      aux = aux.replace(QLatin1String("\n"), QLatin1String("<br>"));

      if (aux.indexOf(QLatin1String(":")) == -1)
        aux = aux.replace(QLatin1String(" "), QLatin1String(""));
    }
    else
    {
      fieldPos = pkgInfo.indexOf(QLatin1String(":"), fieldPos+1);
      fieldPos+=2;
      aux = pkgInfo.mid(fieldPos);
      fieldEnd = aux.indexOf(QLatin1Char('\n'));
      aux = aux.left(fieldEnd).trimmed();
    }
  }

  return aux;
}

/*
 * Retrieves "Name" field of the given package information string represented by pkgInfo
 */
QString Package::getName(const QString& pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Name"), pkgInfo);
}

/*
 * Retrieves "Version" field of the given package information string represented by pkgInfo
 */
QString Package::getVersion(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Version"), pkgInfo);
}

/*
 * Retrieves "Repository" field of the given package information string represented by pkgInfo
 */
QString Package::getRepository(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Repository"), pkgInfo);
}

/*
 * Retrieves "URL" field of the given package information string represented by pkgInfo
 */
QString Package::getURL(const QString &pkgInfo)
{
  QString URL = extractFieldFromInfo(QStringLiteral("\nURL"), pkgInfo);
  if (!URL.isEmpty())
    return makeURLClickable(URL);
  else
    return URL;
}

/*
 * Retrieves "Licenses" field of the given package information string represented by pkgInfo
 */
QString Package::getLicense(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Licenses"), pkgInfo);
}

/*
 * Retrieves "Maintainer" field of the given package information string represented by pkgInfo
 */
QString Package::getMaintainer(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Maintainer"), pkgInfo);
}

/*
 * Retrieves "Last Modified" field of the given package information string represented by pkgInfo
 */
QString Package::getLastModified(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Last Modified"), pkgInfo);
}

/*
 * Retrieves "Out-of-date" field of the given package information string represented by pkgInfo
 */
QString Package::getOutOfDate(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Out-of-date"), pkgInfo);
}

/*
 * Retrieves "Groups" field of the given package information string represented by pkgInfo
 */
QString Package::getGroup(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Groups"), pkgInfo);
}

/*
 * Retrieves "Provides" field of the given package information string represented by pkgInfo
 */
QString Package::getProvides(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Provides"), pkgInfo);
}

/*
 * Retrieves "Depends On" field of the given package information string represented by pkgInfo
 */
QString Package::getDependsOn(const QString &pkgInfo)
{
  QString res = extractFieldFromInfo(QStringLiteral("Depends On"), pkgInfo);

  if (res.isEmpty())
    res = extractFieldFromInfo(QStringLiteral("Depends on"), pkgInfo);

  return res;
}

/*
 * Retrieves "Make Deps" field of the given AUR package information string represented by pkgInfo
 */
QString Package::getMakeDeps(const QString &pkgInfo)
{
  QString res = extractFieldFromInfo(QStringLiteral("Make Deps"), pkgInfo);

  if (res.isEmpty())
    res = extractFieldFromInfo(QStringLiteral("Make Deps"), pkgInfo);

  return res;
}

/*
 * Retrieves "Optional Deps" field of the given package information string represented by pkgInfo
 */
QString Package::getOptDepends(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Optional Deps"), pkgInfo);
}

/*
 * Retrieves "Conflicts With" field of the given package information string represented by pkgInfo
 */
QString Package::getConflictsWith(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Conflicts With"), pkgInfo);
}

/*
 * Retrieves "Replaces" field of the given package information string represented by pkgInfo
 */
QString Package::getReplaces(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Replaces"), pkgInfo);
}

/*
 * Retrieves "RequiredBy" field of the given package information string represented by pkgInfo
 */
QString Package::getRequiredBy(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Required By"), pkgInfo);
}

/*
 * Retrieves "OptionalFor" field of the given package information string represented by pkgInfo
 */
QString Package::getOptionalFor(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Optional For"), pkgInfo);
}

/*
 * Retrieves "Packager" field of the given package information string represented by pkgInfo
 */
QString Package::getPackager(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Packager"), pkgInfo);
}

/*
 * Retrieves "Architecture" field of the given package information string represented by pkgInfo
 */
QString Package::getArch(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Architecture"), pkgInfo);
}

/*
 * Retrieves "Build Date" field of the given package information string represented by pkgInfo
 */
QDateTime Package::getBuildDate(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo(QStringLiteral("Build Date"), pkgInfo);
  return QDateTime::fromString(aux); //"ddd MMM d hh:mm:ss yyyy");
}

/*
 * Retrieves "Install Date" field of the given package information string represented by pkgInfo
 */
QDateTime Package::getInstallDate(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo(QStringLiteral("Install Date"), pkgInfo);
  return QDateTime::fromString(aux); //"ddd MMM d hh:mm:ss yyyy");
}

/*
 * Retrieves "Download Size" field of the given package information string represented by pkgInfo
 */
double Package::getDownloadSize(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo(QStringLiteral("Download Size"), pkgInfo);

  bool isKByte = (aux.indexOf(QLatin1String("KiB"), Qt::CaseInsensitive) != -1);
  bool isMega = (aux.indexOf(QLatin1String("MiB"), Qt::CaseInsensitive) != -1);
  aux = aux.section(QRegularExpression(QStringLiteral("\\s")), 0, 0);

  bool ok;
  double res = aux.toDouble(&ok);

  if (ok)
  {
    if (isKByte) res *= 1024;
    else if (isMega)
    {
      res *= 1024;
      res *= 1024;
    }
    return res;
  }
  else
    return 0;
}

/*
 * Retrieves "Download Size" field of the given package information string represented by pkgInfo
 */
QString Package::getDownloadSizeAsString(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo(QStringLiteral("Download Size"), pkgInfo);
  return aux;
}

/*
 * Retrieves "Installed Size" field of the given package information string represented by pkgInfo
 */
double Package::getInstalledSize(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo(QStringLiteral("Installed Size"), pkgInfo);
  bool isKByte = (aux.indexOf(QLatin1String("KiB"), Qt::CaseInsensitive) != -1);
  bool isMega = (aux.indexOf(QLatin1String("MiB"), Qt::CaseInsensitive) != -1);

  aux = aux.section(QRegularExpression(QStringLiteral("\\s")), 0, 0);

  bool ok;
  double res = aux.toDouble(&ok);

  if (ok)
  {
    if (isKByte) res *= 1024;
    else if (isMega)
    {
      res *= 1024;
      res *= 1024;
    }
    //else if (isByte) res /= 1024.00;
    return res;
  }
  else
    return 0;
}

/*
 * Retrieves "Install Reason" field of the given package information string represented by pkgInfo
 */
QString Package::getInstallReason(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Install Reason"), pkgInfo);
}

/*
 * Retrieves "Install Reason" field of the given package
 */
QString Package::getInstallReasonByPkgName(const QString &pkgName)
{
  QString pkgInfo = QString::fromUtf8(UnixCommand::getPackageInformation(pkgName, true));
  QString instReason = getInstallReason(pkgInfo);
  return getInstallReason(pkgInfo);
}

/*
 * Retrieves "Installed Size" field of the given package information string represented by pkgInfo
 */
QString Package::getInstalledSizeAsString(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo(QStringLiteral("Installed Size"), pkgInfo);
  return aux;
}

/**
 * Split EVR into epoch, version, and release components.
 * @param evr		[epoch:]version[-release] string
 * @retval *ep		pointer to epoch
 * @retval *vp		pointer to version
 * @retval *rp		pointer to release
 */
void Package::parseEVR(char *evr, const char **ep, const char **vp,
    const char **rp)
{
  const char *epoch;
  const char *version;
  const char *release;
  char *s, *se;

  s = evr;
  /* s points to epoch terminator */
  while(*s && isdigit(*s)) s++;
  /* se points to version terminator */
  se = strrchr(s, '-');

  if(*s == ':') {
    epoch = evr;
    *s++ = '\0';
    version = s;
    if(*epoch == '\0') {
      epoch = "0";
    }
  } else {
    /* different from RPM- always assume 0 epoch */
    epoch = "0";
    version = evr;
  }
  if(se) {
    *se++ = '\0';
    release = se;
  } else {
    release = nullptr;
  }

  if(ep) *ep = epoch;
  if(vp) *vp = version;
  if(rp) *rp = release;
}

/**
 * This function was copied from ArchLinux Pacman project
 *
 * Compare alpha and numeric segments of two versions.
 * return 1: a is newer than b
 *        0: a and b are the same version
 *       -1: b is newer than a
 */
int Package::rpmvercmp(const char *a, const char *b){
  char oldch1, oldch2;
  char *str1, *str2;
  char *ptr1, *ptr2;
  char *one, *two;
  int rc;
  int isnum;
  int ret = 0;

  /* easy comparison to see if versions are identical */
  if(strcmp(a, b) == 0) return 0;

  str1 = strdup(a);
  str2 = strdup(b);

  one = ptr1 = str1;
  two = ptr2 = str2;

  /* loop through each version segment of str1 and str2 and compare them */
  while (*one && *two) {
    while (*one && !isalnum((int)*one)) one++;
    while (*two && !isalnum((int)*two)) two++;

    /* If we ran to the end of either, we are finished with the loop */
    if (!(*one && *two)) break;

    /* If the separator lengths were different, we are also finished */
    if ((one - ptr1) != (two - ptr2)) {
      return (one - ptr1) < (two - ptr2) ? -1 : 1;
    }

    ptr1 = one;
    ptr2 = two;

    /* grab first completely alpha or completely numeric segment */
    /* leave one and two pointing to the start of the alpha or numeric */
    /* segment and walk ptr1 and ptr2 to end of segment */
    if (isdigit((int)*ptr1)) {
      while (*ptr1 && isdigit((int)*ptr1)) ptr1++;
      while (*ptr2 && isdigit((int)*ptr2)) ptr2++;
      isnum = 1;
    } else {
      while (*ptr1 && isalpha((int)*ptr1)) ptr1++;
      while (*ptr2 && isalpha((int)*ptr2)) ptr2++;
      isnum = 0;
    }

    /* save character at the end of the alpha or numeric segment */
    /* so that they can be restored after the comparison */
    oldch1 = *ptr1;
    *ptr1 = '\0';
    oldch2 = *ptr2;
    *ptr2 = '\0';

    /* this cannot happen, as we previously tested to make sure that */
    /* the first string has a non-null segment */
    if (one == ptr1) {
      ret = -1;       /* arbitrary */
      goto cleanup;
    }

    /* take care of the case where the two version segments are */
    /* different types: one numeric, the other alpha (i.e. empty) */
    /* numeric segments are always newer than alpha segments */
    /* XXX See patch #60884 (and details) from bugzilla #50977. */
    if (two == ptr2) {
      ret = isnum ? 1 : -1;
      goto cleanup;
    }

    if (isnum) {
      /* this used to be done by converting the digit segments */
      /* to ints using atoi() - it's changed because long  */
      /* digit segments can overflow an int - this should fix that. */

      /* throw away any leading zeros - it's a number, right? */
      while (*one == '0') one++;
      while (*two == '0') two++;

      /* whichever number has more digits wins */
      if (strlen(one) > strlen(two)) {
        ret = 1;
        goto cleanup;
      }
      if (strlen(two) > strlen(one)) {
        ret = -1;
        goto cleanup;
      }
    }

    /* strcmp will return which one is greater - even if the two */
    /* segments are alpha or if they are numeric.  don't return  */
    /* if they are equal because there might be more segments to */
    /* compare */
    rc = strcmp(one, two);
    if (rc) {
      ret = rc < 1 ? -1 : 1;
      goto cleanup;
    }

    /* restore character that was replaced by null above */
    *ptr1 = oldch1;
    one = ptr1;
    *ptr2 = oldch2;
    two = ptr2;
  }

  /* this catches the case where all numeric and alpha segments have */
  /* compared identically but the segment separating characters were */
  /* different */
  if ((!*one) && (!*two)) {
    ret = 0;
    goto cleanup;
  }

  /* the final showdown. we never want a remaining alpha string to
         * beat an empty string. the logic is a bit weird, but:
         * - if one is empty and two is not an alpha, two is newer.
         * - if one is an alpha, two is newer.
         * - otherwise one is newer.
         * */
  if ( (!*one && !isalpha((int)*two))
       || isalpha((int)*one) ) {
    ret = -1;
  } else {
    ret = 1;
  }

cleanup:
  free(str1);
  free(str2);
  return ret;
}

/** Compare two version strings and determine which one is 'newer'.
 * Returns a value comparable to the way strcmp works. Returns 1
 * if a is newer than b, 0 if a and b are the same version, or -1
 * if b is newer than a.
 *
 * Different epoch values for version strings will override any further
 * comparison. If no epoch is provided, 0 is assumed.
 *
 * Keep in mind that the pkgrel is only compared if it is available
 * on both versions handed to this function. For example, comparing
 * 1.5-1 and 1.5 will yield 0; comparing 1.5-1 and 1.5-2 will yield
 * -1 as expected. This is mainly for supporting versioned dependencies
 * that do not include the pkgrel.
 */
int Package::alpm_pkg_vercmp(const char *a, const char *b)
{
  char *full1, *full2;
  const char *epoch1, *ver1, *rel1;
  const char *epoch2, *ver2, *rel2;
  int ret;

  /* ensure our strings are not null */
  if(!a && !b) {
    return 0;
  } else if(!a) {
    return -1;
  } else if(!b) {
    return 1;
  }
  /* another quick shortcut- if full version specs are equal */
  if(strcmp(a, b) == 0) {
    return 0;
  }

  /* Parse both versions into [epoch:]version[-release] triplets. We probably
   * don't need epoch and release to support all the same magic, but it is
   * easier to just run it all through the same code. */
  full1 = strdup(a);
  full2 = strdup(b);

  /* parseEVR modifies passed in version, so have to dupe it first */
  parseEVR(full1, &epoch1, &ver1, &rel1);
  parseEVR(full2, &epoch2, &ver2, &rel2);

  ret = rpmvercmp(epoch1, epoch2);
  if(ret == 0) {
    ret = rpmvercmp(ver1, ver2);
    if(ret == 0 && rel1 && rel2) {
      ret = rpmvercmp(rel1, rel2);
    }
  }

  free(full1);
  free(full2);
  return ret;
}

/*
 * Retrieves "Description" field of the given package information string represented by pkgInfo
 */
QString Package::getDescription(const QString &pkgInfo)
{
  return extractFieldFromInfo(QStringLiteral("Description"), pkgInfo);
}

/*
 * Retrieves all information for a given KCP package name
 */
PackageInfoData Package::getKCPInformation(const QString &pkgName)
{
  PackageInfoData res;
  QString pkgInfo = QString::fromUtf8(UnixCommand::getKCPPackageInformation(pkgName));
  pkgInfo.remove(QStringLiteral("\033[0;1m"));
  pkgInfo.remove(QStringLiteral("\033[0m"));
  pkgInfo.remove(QStringLiteral("[1;33m"));
  pkgInfo.remove(QStringLiteral("[00;31m"));
  pkgInfo.remove(QStringLiteral("\033[1;34m"));
  pkgInfo.remove(QStringLiteral("\033[0;1m"));
  pkgInfo.remove(QStringLiteral("c"));
  pkgInfo.remove(QStringLiteral("C"));
  pkgInfo.remove(QStringLiteral(""));
  pkgInfo.remove(QStringLiteral("[m[0;37m"));
  pkgInfo.remove(QStringLiteral("o"));
  pkgInfo.remove(QStringLiteral("[m"));
  pkgInfo.remove(QStringLiteral(";37m"));
  pkgInfo.remove(QStringLiteral("[c"));
  pkgInfo.remove(QStringLiteral("[mo"));
  pkgInfo.remove(QStringLiteral("[1m"));
  pkgInfo.remove(QStringLiteral("[m"));

  res.url = getURL(pkgInfo);
  res.license = getLicense(pkgInfo);
  res.dependsOn = getDependsOn(pkgInfo);
  res.optDepends = getOptDepends(pkgInfo);
  res.provides = getProvides(pkgInfo);
  res.replaces = getReplaces(pkgInfo);
  res.conflictsWith = getConflictsWith(pkgInfo);
  res.description = getDescription(pkgInfo);

  return res;
}

/*
 * Retrieves all information for a given package name
 */
PackageInfoData Package::getInformation(const QString &pkgName, bool foreignPackage)
{
  PackageInfoData res;

  QString pkgInfo = QString::fromUtf8(UnixCommand::getPackageInformation(pkgName, foreignPackage));

  res.name = pkgName;
  res.version = getVersion(pkgInfo);
  res.url = getURL(pkgInfo);
  res.license = getLicense(pkgInfo);
  res.dependsOn = getDependsOn(pkgInfo);
  res.optDepends = getOptDepends(pkgInfo);
  res.group = getGroup(pkgInfo);
  res.provides = getProvides(pkgInfo);
  res.replaces = getReplaces(pkgInfo);
  res.requiredBy = getRequiredBy(pkgInfo);
  res.optionalFor = getOptionalFor(pkgInfo);
  res.conflictsWith = getConflictsWith(pkgInfo);
  res.packager = getPackager(pkgInfo);
  res.arch = getArch(pkgInfo);
  res.buildDate = getBuildDate(pkgInfo);
  res.installDate = getInstallDate(pkgInfo);
  res.description = getDescription(pkgInfo);
  res.downloadSize = getDownloadSizeAsString(pkgInfo);
  res.installedSize = getInstalledSizeAsString(pkgInfo);
  res.downloadSizeAsString = getDownloadSizeAsString(pkgInfo);
  res.installedSizeAsString = getInstalledSizeAsString(pkgInfo);
  res.installReason = getInstallReason(pkgInfo);

  return res;
}

/*
 * Helper to get only the Download Size field of package information
 */
double Package::getDownloadSizeDescription(const QString &pkgName)
{
  QString pkgInfo = QString::fromUtf8(UnixCommand::getPackageInformation(pkgName, false));
  return getDownloadSize(pkgInfo);
}

/*
 * Helper to get only the Description field of package information, for use in tooltips
 */
QString Package::getInformationDescription(const QString &pkgName, bool foreignPackage)
{
  QString pkgInfo = QString::fromUtf8(UnixCommand::getPackageInformation(pkgName, foreignPackage));
  return getDescription(pkgInfo);
}

/*
 * Helper to get only the Installed Size field of package information, for use in tooltips
 */
QString Package::getInformationInstalledSize(const QString &pkgName, bool foreignPackage)
{
  QString pkgInfo = QString::fromUtf8(UnixCommand::getPackageInformation(pkgName, foreignPackage));
  return kbytesToSize(getInstalledSize(pkgInfo));
}

/*
 * Helper to get only the Version field of AUR package information
 */
QHash<QString, QString> Package::getForeignToolOutdatedPackagesNameVersion()
{
  QHash<QString, QString> hash;

  if(UnixCommand::getLinuxDistro() == ectn_CHAKRA ||
      (getForeignRepositoryToolName() != ctn_PACAUR_TOOL &&
      getForeignRepositoryToolName() != ctn_TRIZEN_TOOL &&
      getForeignRepositoryToolName() != ctn_PIKAUR_TOOL &&
      getForeignRepositoryToolName() != ctn_KCP_TOOL &&
      getForeignRepositoryToolName() != ctn_YAY_TOOL &&
      getForeignRepositoryToolName() != ctn_PARU_TOOL))
  {
    return hash;
  }

  QString res = removeColorCodesFromStr(QString::fromUtf8(UnixCommand::getOutdatedAURPackageList()));
  res = res.trimmed();

  QStringList listOfPkgs = res.split(QStringLiteral("\n"), Qt::SkipEmptyParts);
  QStringList ignorePkgList = UnixCommand::getIgnorePkgsFromPacmanConf();

  if ((getForeignRepositoryToolName() == ctn_TRIZEN_TOOL) ||
      (getForeignRepositoryToolName() == ctn_PIKAUR_TOOL) ||
      (getForeignRepositoryToolName() == ctn_YAY_TOOL) ||
      (getForeignRepositoryToolName() == ctn_PARU_TOOL) ||
      (getForeignRepositoryToolName() == ctn_KCP_TOOL))
  {
    for (QString line: listOfPkgs)
    {
      line = line.trimmed();
      if (line.contains(QLatin1String("Reading "))) continue;
      if (line.contains(StrConstants::getForeignRepositoryTargetPrefix(), Qt::CaseInsensitive))
      {
        line = line.remove(StrConstants::getForeignRepositoryTargetPrefix());
        QStringList nameVersion = line.split(QStringLiteral(" "), Qt::SkipEmptyParts);
        const QString& pkgName = nameVersion.at(0);

        if (getForeignRepositoryToolName() == ctn_KCP_TOOL)
        {
          //Let's ignore the "IgnorePkg" list of packages...
          if (!ignorePkgList.contains(pkgName))
          {
            hash.insert(pkgName, nameVersion.at(1));
          }
        }
      }
      else if (getForeignRepositoryToolName() == ctn_PIKAUR_TOOL)
      {
        QStringList nameVersion = line.split(QStringLiteral(" "), Qt::SkipEmptyParts);
        const QString& pkgName = nameVersion.at(0);

        //Let's ignore the "IgnorePkg" list of packages...
        if (!ignorePkgList.contains(pkgName))
        {
          if (nameVersion.size() == 2)
            hash.insert(pkgName, nameVersion.at(1));
          else if (nameVersion.size() >= 4)
          {
            hash.insert(pkgName, nameVersion.at(3));
          }
        }
      }
      else if (getForeignRepositoryToolName() == ctn_YAY_TOOL ||
               getForeignRepositoryToolName() == ctn_PARU_TOOL ||
               getForeignRepositoryToolName() == ctn_TRIZEN_TOOL)
      {
        QStringList nameVersion = line.split(QStringLiteral(" "), Qt::SkipEmptyParts);
        const QString& pkgName = nameVersion.at(0);

        if (pkgName==QLatin1String("::")) continue;

        if (!ignorePkgList.contains(pkgName))
        {
          if (nameVersion.size() == 2)
            hash.insert(pkgName, nameVersion.at(1));
          else if (nameVersion.size() == 4)
            hash.insert(pkgName, nameVersion.at(3));
        }
      }
    }
  }
  else if (getForeignRepositoryToolName() == ctn_PACAUR_TOOL)
  {
    for (const QString& line: listOfPkgs)
    {
      QStringList sl = line.split(QStringLiteral(" "), Qt::SkipEmptyParts);

      if (sl[1] != StrConstants::getForeignPkgRepositoryName())
        continue;

      if (sl.count() >= 6)
      {
        hash.insert(sl.at(2), sl.at(5));
      }
    }
  }

  return hash;
}

/*
 * Retrieves the file list content of the given package
 */
QStringList Package::getContents(const QString& pkgName, bool isInstalled)
{
  QStringList slResult;
  QByteArray result;

  //OLD pkgfile code:
  /*if (isInstalled)
  {
    result = UnixCommand::getPackageContentsUsingPacman(pkgName, isInstalled);
  }
  else if (UnixCommand::getLinuxDistro() == ectn_ARCHBANGLINUX ||
           UnixCommand::getLinuxDistro() == ectn_ARCHLINUX ||
           UnixCommand::getLinuxDistro() == ectn_ARTIXLINUX ||
           UnixCommand::getLinuxDistro() == ectn_KAOS ||
           UnixCommand::getLinuxDistro() == ectn_PARABOLA)
  {
    result = UnixCommand::getPackageContentsUsingPkgfile(pkgName);
  }*/

  result = UnixCommand::getPackageContentsUsingPacman(pkgName, isInstalled);

  QString aux(QString::fromUtf8(result));
  QStringList rsl = aux.split(QStringLiteral("\n"), Qt::SkipEmptyParts);

  if ( !rsl.isEmpty() ){
    if (rsl.at(0) == QLatin1String("./")){
      rsl.removeFirst();
    }
    if (isInstalled)
    {
      rsl.replaceInStrings(QRegularExpression(pkgName + QLatin1Char(' ')), QLatin1String(""));
      rsl.sort();
      slResult = rsl;
    }
    /* If the filelist came from pkgfile, it's something like this:
       apps/kdemultimedia-juk  /usr/share/doc/kde/html/en/juk/search-playlist.png */
    else
    {
      rsl.replaceInStrings(QRegularExpression(pkgName + QLatin1Char(' ')), QLatin1String("/"));
      rsl.sort();
      slResult = rsl;

      //OLD pkgfile code:
      /*QStringList rsl2;
      for(auto line: qAsConst(rsl))
      {
        QStringList slAux = line.split(QStringLiteral("\t"), Qt::SkipEmptyParts);
        rsl2.append(QString(slAux.at(1).trimmed()));
      }

      rsl2.sort();
      slResult = rsl2;*/
    }
  }

  return slResult;
}

/*
 * Retrieves the list of optional dependencies of the given package
 */
QStringList Package::getOptionalDeps(const QString &pkgName)
{
  QString pkgInfo = QString::fromUtf8(UnixCommand::getPackageInformation(pkgName, false));
  QString aux = Package::getOptDepends(pkgInfo);
  QStringList result = aux.split(QStringLiteral("<br>"), Qt::SkipEmptyParts);
  result.removeAll(QStringLiteral("None"));

  return result;
}

/*
 * Returns a modified RegExp-based string given the string entered by the user
 */
QString Package::parseSearchString(QString searchStr, bool exactMatch)
{
  if (searchStr.indexOf(QLatin1String("*.")) == 0){
    searchStr = searchStr.remove(0, 2);
    searchStr.insert(0, QLatin1String("\\S+\\."));
  }

  if (searchStr.indexOf(QLatin1String("*")) == 0){
    searchStr = searchStr.remove(0, 1);
    searchStr.insert(0, QLatin1String("\\S+"));
  }

  if (searchStr.endsWith(QLatin1String("*"))){
    searchStr.remove(searchStr.length()-1, 1);
    searchStr.append(QLatin1String("\\S*"));
  }

  if (searchStr.indexOf(QLatin1String("^")) == -1 && searchStr.indexOf(QLatin1String("\\S")) != 0){
    if (!exactMatch) searchStr.insert(0, QLatin1String("\\S*"));
    else searchStr.insert(0, QLatin1Char('^'));
  }

  if (searchStr.indexOf(QLatin1String("$")) == -1){
    if (!exactMatch && !searchStr.endsWith(QLatin1String("\\S*"))) searchStr.append(QLatin1String("\\S*"));
    else searchStr.append(QLatin1Char('$'));
  }

  searchStr.replace(QLatin1String("+"), QLatin1String("\\+"));
  searchStr.replace(QLatin1String("?"), QLatin1String("."));
  return searchStr;
}

/*
 * Checks if this system has the core.db
 */
bool Package::hasPacmanDatabase()
{
  static bool done = false;
  static bool answer = false;

  if (!done)
  {
    if (UnixCommand::getLinuxDistro() == ectn_ARTIXLINUX)
    {
      QFile f(ctn_PACMAN_SYSTEM_DB_FILE);
      answer = f.exists();
      done = true;
    }
    else
    {
      QFile f(ctn_PACMAN_CORE_DB_FILE);
      answer = f.exists();
      done = true;
    }
  }

  return answer;
}

/*
 * Retrives user base package tool name (with any parameters)
 */
QString Package::getForeignRepositoryToolNameParam()
{
  static QString ret;

  if( UnixCommand::getLinuxDistro() == ectn_CHAKRA )
    ret = QStringLiteral( "chaser" );
  else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
    ret = QStringLiteral( "kcp" );
  else // We are talking about ARCH based distros...
    ret = SettingsManager::getAURTool();

  return ret;
}

/*
 * Checks if the system is AUR based
 */
bool Package::isAURBased()
{
  static bool firstTime=true;
  static bool ret=false;

  if (firstTime)
  {
    QString ft=getForeignRepositoryToolName();

    ret = ft != QStringLiteral("chaser") && ft != QStringLiteral("kcp");

    firstTime=false;
  }

  return ret;
}

/*
 * Retrives user base package tool name
 */
QString Package::getForeignRepositoryToolName()
{
  static QString ret;

  if( UnixCommand::getLinuxDistro() == ectn_CHAKRA )
    ret = QStringLiteral( "chaser" );
  else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
    ret = QStringLiteral( "kcp" );
  else // We are talking about ARCH based distros...
    ret = SettingsManager::getAURToolName();

  return ret;
}
