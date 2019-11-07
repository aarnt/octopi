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

#include <iostream>
#include <string.h>
#include <ctype.h>

#include <QTextStream>
#include <QList>
#include <QFile>
#include <QRegularExpression>

/*
 * This class abstracts all the relevant package information and services
 */

/*
 * Retrieves the basic package name, without version numbers
 */
QString Package::getBaseName( const QString& p )
{
	QString packageBaseName="";
	QString aux(p);
	int numberOfSegments = p.count('-')+1;
	int nameSegment = numberOfSegments-3;

	for (int i=1; i<= nameSegment; i++){
    int a=aux.indexOf("-");
		packageBaseName += aux.left(a);
		if (i<nameSegment) packageBaseName += "-";
		aux = aux.mid(a+1);
	}

	if (packageBaseName == "") packageBaseName += p.left(p.indexOf("-"));
	return packageBaseName;
}

/*
 * Given a QString, this method searches for a link pattern and inserts an URL html/ftp link tag
 * Returns the modified (or not) QString
 */
QString Package::makeURLClickable( const QString &s )
{
	QString sb = s;
  QRegExp rx("((ht|f)tp(s?))://(\\S)+[^\"|)|(|.|\\s|\\n]");
  QRegExp rx1("^|[\\s]+(www\\.)(\\S)+[^\"|)|(|.|\\s|\\n]");
  rx.setCaseSensitivity( Qt::CaseInsensitive );
	rx1.setCaseSensitivity( Qt::CaseInsensitive );
	int search = 0;
	int ini = 0;

	//First we search for the 1st pattern: rx
	while ( (ini = rx.indexIn( sb, search )) != -1 ){
		QString s1 = rx.cap();
    QString ns;

    ns = "<a href=\"" + s1 + "\">" + s1 + "</a>";
    sb.replace( ini, s1.length(), ns);
		search = ini + (2*s1.length()) + 15;	
	}

	search = 0;
	ini = 0;

  //Now, we search for the 2nd pattern: rx1
	while ( (ini = rx1.indexIn( sb, search )) != -1 ){
		QString s1 = rx1.cap();
		QString ns;
    if (s1[0] == '\n')
      ns += "\n";

		int blanks = s1.count(	QRegularExpression("^|[\\s]+") );
		for (int i=0; i<blanks; i++) ns += " ";

    ns += "<a href=\"http://" + s1.trimmed() + "\">" + s1.trimmed() + "</a>";
    sb.replace( ini, s1.length(), ns);
		search = ini + (2*s1.length()) + 15;	
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
QString Package::kbytesToSize( float Bytes )
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
  QStringList ldeps = optionalDeps.split("<br>", QString::SkipEmptyParts);

  foreach(QString dep, ldeps)
  {
    int colon = dep.indexOf(":");
    if (colon != -1)
    {
      name = dep.left(colon).trimmed();

      newDep = "<a href=\"goto:" + name + "\">" + name + "</a> " + dep.right(dep.length()-colon);
      newDeps += newDep + "<br>";
    }
    else
    {
      newDep = "<a href=\"goto:" + dep + "\">" + dep + "</a> ";
      newDeps += newDep + "<br>";
    }
  }

  newDeps.remove(QRegularExpression("<br>$"));

  return newDeps;
}

/*
 * Parses package list and returns anchors for those which does not have logical operators
 */
QString Package::makeAnchorOfPackage(const QString &packages)
{
  QString newDeps;
  QString newDep;
  QStringList ldeps = packages.split(" ", QString::SkipEmptyParts);

  foreach(QString dep, ldeps)
  {
    if (!dep.contains("=") &&
        !dep.contains("<") &&
        !dep.contains(">"))
    {
      newDeps += "<a href=\"goto:" + dep + "\">" + dep + "</a> ";
    }
    else
    {
      int p=-1;

      if (dep.contains("<"))
      {
        if (dep.contains("="))
        {
          p = dep.indexOf("<");
          newDep = dep.left(p);
        }
        else
        {
          newDep = dep.left(p);
          dep.replace("<", "&lt;");
        }
      }
      else if (dep.contains(">"))
      {
        p = dep.indexOf(">");
        if (dep.contains("="))
        {
          p = dep.indexOf(">");
          newDep = dep.left(p);
        }
        else
        {
          newDep = dep.left(p);
          dep.replace(">", "&gt;");
        }
      }
      else if (dep.contains("="))
      {
        p = dep.indexOf("=");
        newDep = dep.left(p);
      }

      newDeps += "<a href=\"goto:" + newDep + "\">" + dep + "</a> ";
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
    QString unrequiredPkgList = UnixCommand::getUnrequiredPackageList();
    QStringList packageTuples = unrequiredPkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);

    foreach(QString packageTuple, packageTuples)
    {
      QStringList parts = packageTuple.split(' ');
      {
        res->insert(parts[0]); //We only need the package name!
      }
    }
  }
#ifdef ALPM_BACKEND
  else
  {
    QStringList packageTuples = AlpmBackend::getUnrequiredList();

    foreach(QString packageTuple, packageTuples)
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
    QString outPkgList = UnixCommand::getOutdatedPackageList();
    QStringList packageTuples = outPkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
    QStringList ignorePkgList = UnixCommand::getIgnorePkgsFromPacmanConf();

    foreach(QString packageTuple, packageTuples)
    {
      QStringList parts = packageTuple.split(' ');
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

    foreach(QString packageTuple, packageTuples)
    {
      res->append(packageTuple);
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

  ret = ret.remove("\033");
  ret = ret.remove("[1;31m");
  ret = ret.remove("[1;32m");
  ret = ret.remove("[1;33m");
  ret = ret.remove("[1;34m");
  ret = ret.remove("[1;35m");
  ret = ret.remove("[1;36m");
  ret = ret.remove("[1;39m");
  ret = ret.remove("[m");
  ret = ret.remove("[0m");
  ret = ret.remove("[1m");
  ret = ret.remove("\u001B");

  return ret;
}

/*
 * Retrieves the list of outdated Yaourt (AUR) packages
 * (those which have newer versions available to download)
 */
QStringList *Package::getOutdatedAURStringList()
{
  QStringList * res = new QStringList();

  if (getForeignRepositoryToolName() != ctn_YAOURT_TOOL &&
      getForeignRepositoryToolName() != ctn_PACAUR_TOOL &&
      getForeignRepositoryToolName() != ctn_TRIZEN_TOOL &&
      getForeignRepositoryToolName() != ctn_PIKAUR_TOOL &&
      getForeignRepositoryToolName() != ctn_YAY_TOOL &&
      getForeignRepositoryToolName() != ctn_KCP_TOOL)
    return res;

  QString outPkgList = removeColorCodesFromStr(UnixCommand::getOutdatedAURPackageList());
  outPkgList = outPkgList.trimmed();

  QStringList packageTuples = outPkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  QStringList ignorePkgList = UnixCommand::getIgnorePkgsFromPacmanConf();

  foreach(QString packageTuple, packageTuples)
  {
    QStringList parts = packageTuple.split(' ', QString::SkipEmptyParts);
    {
      if (getForeignRepositoryToolName() == ctn_YAOURT_TOOL ||
          getForeignRepositoryToolName() == ctn_TRIZEN_TOOL ||
          getForeignRepositoryToolName() == ctn_PIKAUR_TOOL ||
          getForeignRepositoryToolName() == ctn_YAY_TOOL ||
          getForeignRepositoryToolName() == ctn_KCP_TOOL)
      {
        QString pkgName;
        pkgName = parts[0];

        if (getForeignRepositoryToolName() == ctn_YAOURT_TOOL)
        {
          if (!pkgName.startsWith(StrConstants::getForeignRepositoryTargetPrefix()))
              continue;
        }

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
  QString packagesFromGroup = UnixCommand::getPackageGroups();
  QStringList packageTuples = packagesFromGroup.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  QStringList * res = new QStringList();

  foreach(QString packageTuple, packageTuples)
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
  QString packagesFromGroup = UnixCommand::getPackagesFromGroup(groupName);
  QStringList packageTuples = packagesFromGroup.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  QStringList * res = new QStringList();

  foreach(QString packageTuple, packageTuples)
  {
    QStringList parts = packageTuple.split(' ');
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
  QString targets = UnixCommand::getTargetUpgradeList(pkgName);
  QStringList packageTuples = targets.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  QList<PackageListData> *res = new QList<PackageListData>();
  packageTuples.sort();

  foreach(QString packageTuple, packageTuples)
  {
    //TODO: Need to handle when this list has "::" conflict items!
    if(packageTuple.indexOf("::")!=-1)
    {
      continue;
    }

    PackageListData ld;
    QStringList data = packageTuple.split(" ");
    if (data.count() == 3)
    {
      ld = PackageListData(data.at(0), data.at(1), data.at(2));
    }
    else if (data.count() == 1)
    {
      ld = PackageListData(data.at(0), "", 0);
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
  QString targets = UnixCommand::getTargetRemovalList(pkgName, removeCommand);
  QStringList packageTuples = targets.split(QRegularExpression("\\n"), QString::SkipEmptyParts);
  QStringList * res = new QStringList();

  foreach(QString packageTuple, packageTuples)
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
    QString foreignPkgList = UnixCommand::getForeignPackageList();
    QStringList packageTuples = foreignPkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);

    foreach(QString packageTuple, packageTuples)
    {
      QStringList parts = packageTuple.split(' ');
      if (parts.size() == 2)
      {
        res->append(PackageListData(parts[0], "", parts[1], ectn_FOREIGN));
      }
    }
  }
#ifdef ALPM_BACKEND
  else
  {
    QStringList packageTuples = AlpmBackend::getForeignList();

    foreach(QString packageTuple, packageTuples)
    {
      QStringList parts = packageTuple.split("<o'o>");
      res->append(PackageListData(parts[0], "", parts[1], parts[0] + " " + parts[2], ectn_FOREIGN));
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
  //archlinuxfr/yaourt 1.2.2-1 [installed]
  //    A pacman wrapper with extended features and AUR support
  //community/libfm 1.1.0-4 (lxde) [installed: 1.1.0-3]

  QList<PackageListData> * res = new QList<PackageListData>();

  if (SettingsManager::hasPacmanBackend())
  {
    QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion;
    PackageStatus pkgStatus;
    QString pkgList = UnixCommand::getPackageList(packageName);
    QStringList packageTuples = pkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);

    if(!pkgList.isEmpty())
    {
      pkgDescription = "";
      foreach(QString packageTuple, packageTuples)
      {
        if (!packageTuple[0].isSpace())
        {
          //Do we already have a description?
          if (pkgDescription != "")
          {
            pkgDescription = pkgName + " " + pkgDescription;

            PackageListData pld =
                PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);

            if (packageName.isEmpty() || pkgName == packageName)
            {
              res->append(pld);
            }

            pkgDescription = "";
          }

          //First we get repository and name!
          QStringList parts = packageTuple.split(' ');
          QString repoName = parts[0];
          int a = repoName.indexOf("/");
          pkgRepository = repoName.left(a);
          pkgName = repoName.mid(a+1);
          pkgVersion = parts[1];

          if(packageTuple.indexOf("[installed]") != -1)
          {
            //This is an installed package
            pkgStatus = ectn_INSTALLED;
            pkgOutVersion = "";
          }
          else if (packageTuple.indexOf("[installed:") != -1)
          {
            //This is an outdated installed package
            pkgStatus = ectn_OUTDATED;

            int i = packageTuple.indexOf("[installed:");
            pkgOutVersion = packageTuple.mid(i+11);
            pkgOutVersion = pkgOutVersion.remove(']').trimmed();
          }
          else
          {
            //This is an uninstalled package
            pkgStatus = ectn_NON_INSTALLED;
            pkgOutVersion = "";
          }
        }
        else
        {
          //This is a description!
          if (!packageTuple.trimmed().isEmpty())
            pkgDescription += packageTuple.trimmed();
          else
            pkgDescription += " "; //StrConstants::getNoDescriptionAvailabe();
        }
      }

      //And adds the very last package...
      pkgDescription = pkgName + " " + pkgDescription;
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
    QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion, pkgSize;
    PackageStatus pkgStatus;
    double pkgDownSize=0;
    QStringList pkgList = AlpmBackend::getPackageList();
    bool ok;
    bool hasOutdatedPackages = checkUpdatesOutdatedPackages->count() > 0;

    pkgDescription = "";
    foreach(QString packageTuple, pkgList)
    {
      if (!packageTuple[0].isSpace())
      {
        //Do we already have a description?
        if (pkgDescription != "")
        {
          pkgDescription = pkgName + " " + pkgDescription;

          PackageListData pld =
              PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription,
                              pkgStatus, pkgDownSize, pkgOutVersion);

          res->append(pld);
          pkgDescription = "";
        }

        //First we get repository and name!
        QStringList parts = packageTuple.split(' ');
        pkgRepository = parts[1];
        pkgName = parts[2];
        pkgVersion = parts[3];
        pkgSize = parts[5];
        pkgDownSize = pkgSize.toLong(&ok);

        if(parts[0] == "i")
        {
          //This is an installed package
          if (!hasOutdatedPackages)
          {
            pkgStatus = ectn_INSTALLED;
            pkgOutVersion = "";
          }
          else
          {
            QString newVersion = checkUpdatesOutdatedPackages->value(pkgName);
            if (newVersion.isEmpty())
            {
              pkgStatus = ectn_INSTALLED;
              pkgOutVersion = "";
            }
            else
            {
              pkgStatus = ectn_OUTDATED;
              pkgOutVersion = pkgVersion;
              pkgVersion = newVersion;
            }
          }
        }
        else if (parts[0] == "o")
        {
          //This is an outdated installed package
          pkgStatus = ectn_OUTDATED;
          pkgOutVersion = parts[4];
        }
        else if (parts[0] == "n")
        {
          //This is an uninstalled package
          pkgStatus = ectn_NON_INSTALLED;
          pkgOutVersion = "";
        }
      }
      else
      {
        //This is a description!
        if (!packageTuple.trimmed().isEmpty())
          pkgDescription += packageTuple.trimmed();
        else
          pkgDescription += " ";
      }
    }

    //And adds the very last package...
    pkgDescription = pkgName + " " + pkgDescription;
    PackageListData pld =
        PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgDownSize, pkgOutVersion);

    res->append(pld);
  }
#endif

  return res;
}

/*
 * Helper method to get URL information from remote package
 */
QString Package::getAURUrl(const QString &pkgName)
{
  QString url="";

  if (getForeignRepositoryToolName() != ctn_KCP_TOOL)
  {
    QString pkgInfo = UnixCommand::getAURUrl(pkgName);
    url = getURL(pkgInfo);
  }

  return url;
}

/*
 * Retrieves the list of all AUR packages in the database (installed + non-installed)
 * given the search parameter
 */
QList<PackageListData> * Package::getAURPackageList(const QString& searchString)
{
  //aur/yaourt 1.2.2-1 [installed]
  //    A pacman wrapper with extended features and AUR support
  //aur/libfm 1.1.0-4 (lxde) [installed: 1.1.0-3]

  QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion;
  int pkgVotes;
  PackageStatus pkgStatus;
  QList<PackageListData> * res = new QList<PackageListData>();

  if (UnixCommand::getLinuxDistro() != ectn_KAOS && searchString.isEmpty())
    return res;

  QString aurTool = getForeignRepositoryToolName();
  QString pkgList = UnixCommand::getAURPackageList(searchString);
  QStringList packageTuples = pkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);

  if (aurTool == ctn_YAY_TOOL)
  {
    return getYayPackageList(packageTuples);
  }

  pkgDescription = "";
  foreach(QString packageTuple, packageTuples)
  {
    if (packageTuple[0].isNumber())
    {
      int space=packageTuple.indexOf(" ");
      packageTuple = packageTuple.mid(space+1);
    }

    if ((UnixCommand::getLinuxDistro() != ectn_KAOS && !packageTuple[0].isSpace()) ||
        (UnixCommand::getLinuxDistro() == ectn_KAOS && packageTuple[0] != '\t'))
    {
      //Do we already have a description?
      if (pkgDescription != "")
      {
        pkgDescription = pkgName + " " + pkgDescription;

        PackageListData pld =
            PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);

        pld.popularity = pkgVotes;

        res->append(pld);
        pkgDescription = "";
      }

      //First we get repository and name!
      QStringList parts = packageTuple.split(' ');

      if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      {
        parts[0] = parts[0].remove("[1;35m");
      }

      QString repoName = parts[0];
      int a = repoName.indexOf("/");
      pkgRepository = repoName.left(a);

      if (pkgRepository != StrConstants::getForeignPkgRepositoryName())
      {
        res->removeAt(res->count()-1);
        continue;
      }

      pkgRepository = StrConstants::getForeignPkgRepositoryName().toUpper();
      pkgName = repoName.mid(a+1);
      pkgVersion = parts[1];

      QStringList strVotes = parts.filter("(");
      //Let's see if it's not a Trizen style
      if (strVotes.isEmpty())
        strVotes = parts.filter("+]");

      pkgVotes = 0;

      //Chakra does not have popularity support in CCR
      //QString aurTool = getForeignRepositoryToolName();
      if (aurTool == ctn_TRIZEN_TOOL)
      {
        if (!strVotes.first().isEmpty())
          pkgVotes = strVotes.first().replace('[', "").replace(']', "").replace('+', "").toInt();
        else
          pkgVotes = 0;
      }
      else if (aurTool != ctn_CHASER_TOOL && aurTool != ctn_PACAUR_TOOL && aurTool != ctn_PIKAUR_TOOL && strVotes.count() > 0)
      {
        if (!strVotes.first().isEmpty())
          pkgVotes = strVotes.first().replace('(', "").replace(')', "").toInt();
        else
          pkgVotes = 0;
      }
      else if ( (aurTool == ctn_PACAUR_TOOL || aurTool == ctn_PIKAUR_TOOL) && strVotes.count() > 0)
      {
        if (!strVotes.first().isEmpty())
        {
          QString str = strVotes.first();
          int comma = str.indexOf(",");
          pkgVotes = str.mid(1, comma-1).toInt();
        }
        else pkgVotes = 0;
      }

      if (aurTool == ctn_PIKAUR_TOOL)
      {
        if(packageTuple.indexOf(" [") != -1)
        {
          if (packageTuple.indexOf("installed: ") != -1)
          {
            int i = packageTuple.indexOf(": ");
            pkgOutVersion = packageTuple.mid(i+2);
            pkgOutVersion = pkgOutVersion.remove(QRegularExpression("\\].*")).trimmed();

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
              pkgOutVersion = "";
            }
          }
          else
          {
            //This is an installed package
            pkgStatus = ectn_FOREIGN;
            pkgOutVersion = "";
          }
        }
        else
        {
          //This is an uninstalled package
          pkgStatus = ectn_NON_INSTALLED;
          pkgOutVersion = "";
        }
      }
      else if(packageTuple.indexOf("[installed]") != -1)
      {
        //This is an installed package
        pkgStatus = ectn_FOREIGN;
        pkgOutVersion = "";
      }
      else if (packageTuple.indexOf("[installed:") != -1)
      {
        int i = packageTuple.indexOf(": ");
        pkgOutVersion = packageTuple.mid(i+2);
        pkgOutVersion = pkgOutVersion.remove("(");
        pkgOutVersion = pkgOutVersion.remove(")");
        pkgOutVersion = pkgOutVersion.remove(QRegularExpression("\\].*")).trimmed();

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
          pkgOutVersion = "";
        }

        //This is an outdated installed package
        //pkgStatus = ectn_FOREIGN_OUTDATED;

        //int i = packageTuple.indexOf("[installed:");
        //pkgOutVersion = packageTuple.mid(i+11);
        //pkgOutVersion = pkgOutVersion.remove(QRegularExpression("\\].*")).trimmed();
      }
      else
      {
        //This is an uninstalled package
        pkgStatus = ectn_NON_INSTALLED;
        pkgOutVersion = "";
      }
    }
    else
    {            
      //This is a description!
      if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      {        
        pkgDescription = packageTuple;
        pkgDescription.remove("\t");

        if (pkgDescription.isEmpty())
          pkgDescription += " ";
      }
      else
      {
        if (!packageTuple.trimmed().isEmpty())
          pkgDescription += packageTuple.trimmed();
        else
        {
          pkgDescription += " ";
        }
      }
    }
  }

  //And adds the very last package...
  if (packageTuples.count() > 1)
  {
    pkgDescription = pkgName + " " + pkgDescription;
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
QList<PackageListData> *Package::getYayPackageList(const QStringList &packageTuples)
{
  QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion;
  int pkgVotes;
  PackageStatus pkgStatus;
  QList<PackageListData> * res = new QList<PackageListData>();

  //QString pkgList = UnixCommand::getAURPackageList(searchString);
  //QStringList packageTuples = pkgList.split(QRegularExpression("\\n"), QString::SkipEmptyParts);

  pkgDescription = "";
  foreach(QString packageTuple, packageTuples)
  {
    if (packageTuple[0].isNumber())
    {
      int space=packageTuple.indexOf(" ");
      packageTuple = packageTuple.mid(space+1);
    }

    if ((UnixCommand::getLinuxDistro() != ectn_KAOS && !packageTuple[0].isSpace()) ||
        (UnixCommand::getLinuxDistro() == ectn_KAOS && packageTuple[0] != '\t'))
    {
      //Do we already have a description?
      if (pkgDescription != "")
      {
        pkgDescription = pkgName + " " + pkgDescription;

        PackageListData pld =
            PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);

        pld.popularity = pkgVotes;

        res->append(pld);
        pkgDescription = "";
      }

      //First we get repository and name!
      QStringList parts = packageTuple.split(' ');
      QString repoName = parts[0];
      int a = repoName.indexOf("/");
      pkgRepository = repoName.left(a);

      if (pkgRepository != StrConstants::getForeignPkgRepositoryName())
      {
        res->removeAt(res->count()-1);
        continue;
      }

      pkgRepository = StrConstants::getForeignPkgRepositoryName().toUpper();
      pkgName = repoName.mid(a+1);
      pkgVersion = parts[1];

      QStringList strVotes = parts.filter("(+");

      pkgVotes = 0;
      if (strVotes.count() > 0)
      {
        if (!strVotes.first().isEmpty())
        {
          //(+65 1.69%)
          strVotes.first().replace('(', "").replace('+', "");
          int space = strVotes.first().indexOf(" ");
          strVotes = strVotes.mid(0, space);
          if (!strVotes.isEmpty()) pkgVotes = strVotes.first().toInt();
        }
        else
          pkgVotes = 0;
      }

      if(packageTuple.indexOf("(Installed)") != -1)
      {
        //This is an installed package
        pkgStatus = ectn_FOREIGN;
        pkgOutVersion = "";
      }
      else if (packageTuple.indexOf("(Installed:") != -1)
      {
        //This is an outdated installed package
        pkgStatus = ectn_FOREIGN_OUTDATED;

        int i = packageTuple.indexOf("(Installed:");
        pkgOutVersion = packageTuple.mid(i+11);
        pkgOutVersion = pkgOutVersion.remove("(");
        pkgOutVersion = pkgOutVersion.remove(")");
        pkgOutVersion = pkgOutVersion.remove(QRegularExpression("\\].*")).trimmed();
      }
      else
      {
        //This is an uninstalled package
        pkgStatus = ectn_NON_INSTALLED;
        pkgOutVersion = "";
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
          pkgDescription += " ";
        }
      }
    }
  }

  //And adds the very last package...
  if (packageTuples.count() > 1)
  {
    pkgDescription = pkgName + " " + pkgDescription;
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
    if(field == "Optional Deps")
    {
      fieldPos = pkgInfo.indexOf(":", fieldPos+1);
      fieldPos+=2;
      aux = pkgInfo.mid(fieldPos);

      fieldEnd = aux.indexOf("Conflicts With");
      int fieldEnd2 = aux.indexOf("Required By");

      if(fieldEnd > fieldEnd2 && fieldEnd2 != -1) fieldEnd = fieldEnd2;

      aux = aux.left(fieldEnd).trimmed();
      aux = aux.replace("\n", "<br>");

      if (aux.indexOf(":") == -1)
        aux = aux.replace(" ", "");
    }
    else
    {
      fieldPos = pkgInfo.indexOf(":", fieldPos+1);
      fieldPos+=2;
      aux = pkgInfo.mid(fieldPos);
      fieldEnd = aux.indexOf('\n');
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
  return extractFieldFromInfo("Name", pkgInfo);
}

/*
 * Retrieves "Version" field of the given package information string represented by pkgInfo
 */
QString Package::getVersion(const QString &pkgInfo)
{
  return extractFieldFromInfo("Version", pkgInfo);
}

/*
 * Retrieves "Repository" field of the given package information string represented by pkgInfo
 */
QString Package::getRepository(const QString &pkgInfo)
{
  return extractFieldFromInfo("Repository", pkgInfo);
}

/*
 * Retrieves "URL" field of the given package information string represented by pkgInfo
 */
QString Package::getURL(const QString &pkgInfo)
{
  QString URL = extractFieldFromInfo("\nURL", pkgInfo);
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
  return extractFieldFromInfo("Licenses", pkgInfo);
}

/*
 * Retrieves "Groups" field of the given package information string represented by pkgInfo
 */
QString Package::getGroup(const QString &pkgInfo)
{
  return extractFieldFromInfo("Groups", pkgInfo);
}

/*
 * Retrieves "Provides" field of the given package information string represented by pkgInfo
 */
QString Package::getProvides(const QString &pkgInfo)
{
  return extractFieldFromInfo("Provides", pkgInfo);
}

/*
 * Retrieves "Depends On" field of the given package information string represented by pkgInfo
 */
QString Package::getDependsOn(const QString &pkgInfo)
{
  QString res = extractFieldFromInfo("Depends On", pkgInfo);

  if (res.isEmpty())
    res = extractFieldFromInfo("Depends on", pkgInfo);

  return res;
}

/*
 * Retrieves "Optional Deps" field of the given package information string represented by pkgInfo
 */
QString Package::getOptDepends(const QString &pkgInfo)
{
  return extractFieldFromInfo("Optional Deps", pkgInfo);
}

/*
 * Retrieves "Conflicts With" field of the given package information string represented by pkgInfo
 */
QString Package::getConflictsWith(const QString &pkgInfo)
{
  return extractFieldFromInfo("Conflicts With", pkgInfo);
}

/*
 * Retrieves "Replaces" field of the given package information string represented by pkgInfo
 */
QString Package::getReplaces(const QString &pkgInfo)
{
  return extractFieldFromInfo("Replaces", pkgInfo);
}

/*
 * Retrieves "RequiredBy" field of the given package information string represented by pkgInfo
 */
QString Package::getRequiredBy(const QString &pkgInfo)
{
  return extractFieldFromInfo("Required By", pkgInfo);
}

/*
 * Retrieves "OptionalFor" field of the given package information string represented by pkgInfo
 */
QString Package::getOptionalFor(const QString &pkgInfo)
{
  return extractFieldFromInfo("Optional For", pkgInfo);
}

/*
 * Retrieves "Packager" field of the given package information string represented by pkgInfo
 */
QString Package::getPackager(const QString &pkgInfo)
{
  return extractFieldFromInfo("Packager", pkgInfo);
}

/*
 * Retrieves "Architecture" field of the given package information string represented by pkgInfo
 */
QString Package::getArch(const QString &pkgInfo)
{
  return extractFieldFromInfo("Architecture", pkgInfo);
}

/*
 * Retrieves "Build Date" field of the given package information string represented by pkgInfo
 */
QDateTime Package::getBuildDate(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo("Build Date", pkgInfo);
  return QDateTime::fromString(aux); //"ddd MMM d hh:mm:ss yyyy");
}

/*
 * Retrieves "Download Size" field of the given package information string represented by pkgInfo
 */
double Package::getDownloadSize(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo("Download Size", pkgInfo);
  bool isKByte = (aux.indexOf("KiB", Qt::CaseInsensitive) != -1);
  bool isMega = (aux.indexOf("MiB", Qt::CaseInsensitive) != -1);
  //bool isByte = (aux.indexOf(" B", Qt::CaseInsensitive) != -1);

  aux = aux.section(QRegularExpression("\\s"), 0, 0);

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
 * Retrieves "Download Size" field of the given package information string represented by pkgInfo
 */
QString Package::getDownloadSizeAsString(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo("Download Size", pkgInfo);
  return aux;
}

/*
 * Retrieves "Installed Size" field of the given package information string represented by pkgInfo
 */
double Package::getInstalledSize(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo("Installed Size", pkgInfo);
  bool isKByte = (aux.indexOf("KiB", Qt::CaseInsensitive) != -1);
  bool isMega = (aux.indexOf("MiB", Qt::CaseInsensitive) != -1);

  aux = aux.section(QRegularExpression("\\s"), 0, 0);

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
  return extractFieldFromInfo("Install Reason", pkgInfo);
}

/*
 * Retrieves "Installed Size" field of the given package information string represented by pkgInfo
 */
QString Package::getInstalledSizeAsString(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo("Installed Size", pkgInfo);
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
    release = NULL;
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
  return extractFieldFromInfo("Description", pkgInfo);
}

/*
 * Retrieves all information for a given KCP package name
 */
PackageInfoData Package::getKCPInformation(const QString &pkgName)
{
  PackageInfoData res;
  QString pkgInfo = UnixCommand::getKCPPackageInformation(pkgName);
  pkgInfo.remove("\033[0;1m");
  pkgInfo.remove("\033[0m");
  pkgInfo.remove("[1;33m");
  pkgInfo.remove("[00;31m");
  pkgInfo.remove("\033[1;34m");
  pkgInfo.remove("\033[0;1m");
  pkgInfo.remove("c");
  pkgInfo.remove("C");
  pkgInfo.remove("");
  pkgInfo.remove("[m[0;37m");
  pkgInfo.remove("o");
  pkgInfo.remove("[m");
  pkgInfo.remove(";37m");
  pkgInfo.remove("[c");
  pkgInfo.remove("[mo");
  pkgInfo.remove("[1m");
  pkgInfo.remove("[m");

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
  //res = AlpmBackend::getPackageInfo(pkgName, foreignPackage);

  QString pkgInfo = UnixCommand::getPackageInformation(pkgName, foreignPackage);

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
  res.description = getDescription(pkgInfo);
  res.downloadSize = getDownloadSize(pkgInfo);
  res.installedSize = getInstalledSize(pkgInfo);
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
  QString pkgInfo = UnixCommand::getPackageInformation(pkgName, false);
  return getDownloadSize(pkgInfo);
}

/*
 * Helper to get only the Description field of package information, for use in tooltips
 */
QString Package::getInformationDescription(const QString &pkgName, bool foreignPackage)
{
  if (foreignPackage)
  {
    return UnixCommand::getExpacInfo(pkgName, "d");
  }
  else
  {
    QString pkgInfo = UnixCommand::getPackageInformation(pkgName, foreignPackage);
    return getDescription(pkgInfo);
  }
}

/*
 * Helper to get only the Installed Size field of package information, for use in tooltips
 */
QString Package::getInformationInstalledSize(const QString &pkgName, bool foreignPackage)
{
  QString pkgInfo = UnixCommand::getPackageInformation(pkgName, foreignPackage);
  return kbytesToSize(getInstalledSize(pkgInfo));
}

/*
 * Helper to get only the Version field of AUR package information
 */
QHash<QString, QString> Package::getAUROutdatedPackagesNameVersion()
{
  QHash<QString, QString> hash;

  if(UnixCommand::getLinuxDistro() == ectn_CHAKRA ||
      (getForeignRepositoryToolName() != ctn_YAOURT_TOOL &&
      getForeignRepositoryToolName() != ctn_PACAUR_TOOL &&
      getForeignRepositoryToolName() != ctn_TRIZEN_TOOL &&
      getForeignRepositoryToolName() != ctn_PIKAUR_TOOL &&
      getForeignRepositoryToolName() != ctn_KCP_TOOL &&
      getForeignRepositoryToolName() != ctn_YAY_TOOL))
  {
    return hash;
  }

  QString res = removeColorCodesFromStr(UnixCommand::getOutdatedAURPackageList());
  res = res.trimmed();

  QStringList listOfPkgs = res.split("\n", QString::SkipEmptyParts);
  QStringList ignorePkgList = UnixCommand::getIgnorePkgsFromPacmanConf();

  if ((getForeignRepositoryToolName() == ctn_YAOURT_TOOL) ||
      (getForeignRepositoryToolName() == ctn_TRIZEN_TOOL) ||
      (getForeignRepositoryToolName() == ctn_PIKAUR_TOOL) ||
      (getForeignRepositoryToolName() == ctn_YAY_TOOL) ||
      (getForeignRepositoryToolName() == ctn_KCP_TOOL))
  {
    foreach (QString line, listOfPkgs)
    {
      if (line.contains(StrConstants::getForeignRepositoryTargetPrefix(), Qt::CaseInsensitive))
      {
        line = line.remove(StrConstants::getForeignRepositoryTargetPrefix());
        QStringList nameVersion = line.split(" ", QString::SkipEmptyParts);
        QString pkgName = nameVersion.at(0);

        if (getForeignRepositoryToolName() == ctn_KCP_TOOL)
        {
          //Let's ignore the "IgnorePkg" list of packages...
          if (!ignorePkgList.contains(pkgName))
          {
            hash.insert(pkgName, nameVersion.at(1));
          }
        }
        else //It's yaourt!
        {
          //Let's ignore the "IgnorePkg" list of packages...
          if (!ignorePkgList.contains(pkgName))
          {
            if (nameVersion.size() == 2)
              hash.insert(pkgName, nameVersion.at(1));
            else if (nameVersion.size() == 4)
              hash.insert(pkgName, nameVersion.at(3));
          }
        }
      }
      else //We have TRIZEN output here
      {
        QStringList nameVersion = line.split(" ", QString::SkipEmptyParts);
        QString pkgName = nameVersion.at(0);

        if (pkgName=="::") continue;

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
    foreach (QString line, listOfPkgs)
    {
      QStringList sl = line.split(" ", QString::SkipEmptyParts);

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

  if (isInstalled)
  {
    result = UnixCommand::getPackageContentsUsingPacman(pkgName);
  }
  else if (UnixCommand::getLinuxDistro() == ectn_ARCHBANGLINUX ||
           UnixCommand::getLinuxDistro() == ectn_ARCHLINUX ||
           UnixCommand::getLinuxDistro() == ectn_KAOS ||
           //UnixCommand::getLinuxDistro() == ectn_SWAGARCH ||
           UnixCommand::getLinuxDistro() == ectn_PARABOLA)
  {
    result = UnixCommand::getPackageContentsUsingPkgfile(pkgName);
  }

  QString aux(result);
  QStringList rsl = aux.split("\n", QString::SkipEmptyParts);

  if ( !rsl.isEmpty() ){
    if (rsl.at(0) == "./"){
      rsl.removeFirst();
    }
    if (isInstalled)
    {
      rsl.replaceInStrings(QRegularExpression(pkgName + " "), "");
      rsl.sort();
      slResult = rsl;
    }
    /* If the filelist came from pkgfile, it's something like this:
       apps/kdemultimedia-juk  /usr/share/doc/kde/html/en/juk/search-playlist.png */
    else
    {
      QStringList rsl2;
      foreach(QString line, rsl)
      {
        QStringList slAux = line.split("\t", QString::SkipEmptyParts);
        rsl2.append(QString(slAux.at(1).trimmed()));
      }

      rsl2.sort();
      slResult = rsl2;
    }
  }

  return slResult;
}

/*
 * Retrieves the list of optional dependencies of the given package
 */
QStringList Package::getOptionalDeps(const QString &pkgName)
{
  QString pkgInfo = UnixCommand::getPackageInformation(pkgName, false);
  QString aux = Package::getOptDepends(pkgInfo);
  QStringList result = aux.split("<br>", QString::SkipEmptyParts);
  result.removeAll("None");

  return result;
}

/*
 * Returns a modified RegExp-based string given the string entered by the user
 */
QString Package::parseSearchString(QString searchStr, bool exactMatch)
{
  if (searchStr.indexOf("*.") == 0){
    searchStr = searchStr.remove(0, 2);
    searchStr.insert(0, "\\S+\\.");
  }

  if (searchStr.indexOf("*") == 0){
    searchStr = searchStr.remove(0, 1);
    searchStr.insert(0, "\\S+");
  }

  if (searchStr.endsWith("*")){
    searchStr.remove(searchStr.length()-1, 1);
    searchStr.append("\\S*");
  }

  if (searchStr.indexOf("^") == -1 && searchStr.indexOf("\\S") != 0){
    if (!exactMatch) searchStr.insert(0, "\\S*");
    else searchStr.insert(0, "^");
  }

  if (searchStr.indexOf("$") == -1){
    if (!exactMatch && !searchStr.endsWith("\\S*")) searchStr.append("\\S*");
    else searchStr.append("$");
  }

  searchStr.replace("?", ".");
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
    QFile f(ctn_PACMAN_CORE_DB_FILE);
    answer = f.exists();
    done = true;
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
    ret = QLatin1String( "chaser" );
  else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
    ret = QLatin1String( "kcp" );
  else // We are talking about ARCH based distros...
    ret = SettingsManager::getAURTool();

  return ret;
}

/*
 * Retrives user base package tool name
 */
QString Package::getForeignRepositoryToolName()
{
  static QString ret;

  if( UnixCommand::getLinuxDistro() == ectn_CHAKRA )
    ret = QLatin1String( "chaser" );
  else if (UnixCommand::getLinuxDistro() == ectn_KAOS)
    ret = QLatin1String( "kcp" );
  else // We are talking about ARCH based distros...
    ret = SettingsManager::getAURToolName();

  return ret;
}
