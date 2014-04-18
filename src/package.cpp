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
#include "stdlib.h"
#include "strconstants.h"
#include <iostream>

#include <QTextStream>
#include <QList>

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
	int a;

	for (int i=1; i<= nameSegment; i++){
		a=aux.indexOf("-");
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

    if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX)
      ns = "<a style=\"color:\'#425823\'\" href=\"" + s1 + "\">" + s1 + "</a>";
    else
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
		if (s1[0] == '\n') ns += "\n";

		int blanks = s1.count(	QRegExp("^|[\\s]+") );
		for (int i=0; i<blanks; i++) ns += " ";

    if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX)
    {
      ns += "<a style=\"color:\'#425823\'\" href=\"http://" +
          s1.trimmed() + "\">" + s1.trimmed() + "</a>";
    }
    else
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

/**
 * This function was copied from ArchLinux Pacman project
 * Converts sizes in bytes into human readable units.
 *
 * @param bytes the size in bytes
 * @param target_unit '\0' or a short label. If equal to one of the short unit
 * labels ('B', 'K', ...) bytes is converted to target_unit; if '\0', the first
 * unit which will bring the value to below a threshold of 2048 will be chosen.
 * @param precision number of decimal places, ensures -0.00 gets rounded to
 * 0.00; -1 if no rounding desired
 * @param label will be set to the appropriate unit label
 *
 * @return the size in the appropriate unit
 */

/*
 * This function was copied from ArchLinux Pacman project
 * Given a file size, it makes this value easier for human reading :-)
 *
 * An example call:
 * human_size = humanize_size(size, 'M', 2, &label);
 * pm_asprintf(&str, "%.2f %s", human_size, label);
 */
double Package::humanizeSize(off_t bytes, const char target_unit, int precision,
    const char **label)
{
  static const char *labels[] = {"B", "KiB", "MiB", "GiB",
    "TiB", "PiB", "EiB", "ZiB", "YiB"};
  static const int unitcount = sizeof(labels) / sizeof(labels[0]);

  double val = (double)bytes;
  int index;

  for(index = 0; index < unitcount - 1; index++) {
    if(target_unit != '\0' && labels[index][0] == target_unit) {
      break;
    } else if(target_unit == '\0' && val <= 2048.0 && val >= -2048.0) {
      break;
    }
    val /= 1024.0;
  }

  if(label) {
    *label = labels[index];
  }

  /* fix FS#27924 so that it doesn't display negative zeroes */
  if(precision >= 0 && val < 0.0 &&
      val > (-0.5 / simplePow(10, precision))) {
    val = 0.0;
  }

  return val;
}

/*
 * Retrieves the list of unrequired packages (those no other packages depends on)
 */
QSet<QString>* Package::getUnrequiredPackageList()
{
  QString unrequiredPkgList = UnixCommand::getUnrequiredPackageList();
  QStringList packageTuples = unrequiredPkgList.split(QRegExp("\\n"), QString::SkipEmptyParts);
  QSet<QString>* res = new QSet<QString>();

  foreach(QString packageTuple, packageTuples)
  {
    QStringList parts = packageTuple.split(' ');
    {
      res->insert(parts[0]); //We only need the package name!
    }
  }
  return res;
}

/*
 * Retrieves the list of outdated packages (those which have newer versions available to download)
 */
QStringList *Package::getOutdatedPackageList()
{
  QString outPkgList = UnixCommand::getOutdatedPackageList();
  QStringList packageTuples = outPkgList.split(QRegExp("\\n"), QString::SkipEmptyParts);
  QStringList * res = new QStringList();
  QStringList ignorePkgList = UnixCommand::getIgnorePkg();

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
  return res;
}

/*
 * Retrieves the list of outdated Yaourt (AUR) packages (those which have newer versions available to download)
 */
QStringList *Package::getOutdatedYaourtPackageList()
{
  QString outPkgList = UnixCommand::getOutdatedYaourtPackageList();
  QStringList packageTuples = outPkgList.split(QRegExp("\\n"), QString::SkipEmptyParts);
  QStringList * res = new QStringList();
  QStringList ignorePkgList = UnixCommand::getIgnorePkg();

  foreach(QString packageTuple, packageTuples)
  {
    QStringList parts = packageTuple.split(' ');
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
  QStringList packageTuples = packagesFromGroup.split(QRegExp("\\n"), QString::SkipEmptyParts);
  QStringList * res = new QStringList();

  foreach(QString packageTuple, packageTuples)
  {
    packageTuple = packageTuple.trimmed();
    if (!packagesFromGroup.isEmpty())
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
  QStringList packageTuples = packagesFromGroup.split(QRegExp("\\n"), QString::SkipEmptyParts);
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
  QStringList packageTuples = targets.split(QRegExp("\\n"), QString::SkipEmptyParts);

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
  QStringList packageTuples = targets.split(QRegExp("\\n"), QString::SkipEmptyParts);
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
  QString foreignPkgList = UnixCommand::getForeignPackageList();
  QStringList packageTuples = foreignPkgList.split(QRegExp("\\n"), QString::SkipEmptyParts);
  QList<PackageListData> * res = new QList<PackageListData>();

  foreach(QString packageTuple, packageTuples)
  {
    QStringList parts = packageTuple.split(' ');
    if (parts.size() == 2)
    {
      res->append(PackageListData(parts[0], "", parts[1], ectn_FOREIGN));
    }
  }

  return res;
}

/*
 * Retrieves the list of all available packages in the database (installed + non-installed)
 */
QList<PackageListData> * Package::getPackageList()
{
  //archlinuxfr/yaourt 1.2.2-1 [installed]
  //    A pacman wrapper with extended features and AUR support
  //community/libfm 1.1.0-4 (lxde) [installed: 1.1.0-3]

  QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion;
  PackageStatus pkgStatus;
  QString pkgList = UnixCommand::getPackageList();
  QStringList packageTuples = pkgList.split(QRegExp("\\n"), QString::SkipEmptyParts);
  QList<PackageListData> * res = new QList<PackageListData>();

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

        res->append(pld);

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
        pkgOutVersion.remove(']').trimmed();
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
  res->append(pld);

  return res;
}

/*
 * Retrieves the list of all yaourt packages in the database (installed + non-installed)
 * given the search parameter
 */
QList<PackageListData> * Package::getYaourtPackageList(const QString& searchString)
{
  //aur/yaourt 1.2.2-1 [installed]
  //    A pacman wrapper with extended features and AUR support
  //aur/libfm 1.1.0-4 (lxde) [installed: 1.1.0-3]

  QString pkgName, pkgRepository, pkgVersion, pkgDescription, pkgOutVersion;
  PackageStatus pkgStatus;
  QList<PackageListData> * res = new QList<PackageListData>();

  if (searchString.isEmpty())
    return res;

  QString pkgList = UnixCommand::getYaourtPackageList(searchString);
  QStringList packageTuples = pkgList.split(QRegExp("\\n"), QString::SkipEmptyParts);

  pkgDescription = "";
  foreach(QString packageTuple, packageTuples)
  {
    if (packageTuple[0].isNumber())
    {
      int space=packageTuple.indexOf(" ");
      packageTuple = packageTuple.mid(space+1);
    }

    if (!packageTuple[0].isSpace())
    {
      //Do we already have a description?
      if (pkgDescription != "")
      {
        pkgDescription = pkgName + " " + pkgDescription;

        PackageListData pld =
            PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);

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
        pkgOutVersion.remove(QRegExp("\\].*")).trimmed();
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
      {
        pkgDescription += " "; //StrConstants::getNoDescriptionAvailabe();
      }
    }
  }

  //And adds the very last package...
  if (packageTuples.count() > 1)
  {
    pkgDescription = pkgName + " " + pkgDescription;
    PackageListData pld =
        PackageListData(pkgName, pkgRepository, pkgVersion, pkgDescription, pkgStatus, pkgOutVersion);
    res->append(pld);
  }

  if (res->count() > 0 && res->at(0).repository != StrConstants::getForeignPkgRepositoryName().toUpper()) res->removeAt(0);

  return res;
}

/*
 * Given a QString containing the output of pacman -Si/Qi (pkgInfo),
 * this method returns the contents of the given field (ex: description)
 */
QString Package::extractFieldFromInfo(const QString &field, const QString &pkgInfo)
{
  int fieldPos = pkgInfo.indexOf(field);
  int fieldEnd, fieldEnd2;
  QString aux;

  if (fieldPos > 0)
  {
    if(field == "Optional Deps")
    {
      fieldPos = pkgInfo.indexOf(":", fieldPos+1);
      fieldPos+=2;
      aux = pkgInfo.mid(fieldPos);

      fieldEnd = aux.indexOf("Conflicts With");
      fieldEnd2 = aux.indexOf("Required By");

      if(fieldEnd > fieldEnd2 && fieldEnd2 != -1) fieldEnd = fieldEnd2;

      aux = aux.left(fieldEnd).trimmed();
      aux = aux.replace("\n", "<br>");
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
  return extractFieldFromInfo("Depends On", pkgInfo);
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
  aux = aux.section(QRegExp("\\s"), 0, 0);

  bool ok;
  double res = aux.toDouble(&ok);

  if (ok)
    return res;
  else
    return 0;
}

/*
 * Retrieves "Installed Size" field of the given package information string represented by pkgInfo
 */
double Package::getInstalledSize(const QString &pkgInfo)
{
  QString aux = extractFieldFromInfo("Installed Size", pkgInfo);
  aux = aux.section(QRegExp("\\s"), 0, 0);

  bool ok;
  double res = aux.toDouble(&ok);

  if (ok)
    return res;
  else
    return 0;
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

/*
 * Retrieves "Description" field of the given package information string represented by pkgInfo
 */
QString Package::getDescription(const QString &pkgInfo)
{
  return extractFieldFromInfo("Description", pkgInfo);
}

/*
 * Retrieves all information for a given package name
 */
PackageInfoData Package::getInformation(const QString &pkgName, bool foreignPackage)
{
  PackageInfoData res;
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
  QString pkgInfo = UnixCommand::getPackageInformation(pkgName, foreignPackage);
  return getDescription(pkgInfo);
}

/*
 * Helper to get only the Version field of Yaourt package information
 */
QHash<QString, QString> Package::getYaourtOutdatedPackagesNameVersion()
{
  QHash<QString, QString> hash;

  if(UnixCommand::getLinuxDistro() == ectn_CHAKRA)
    return hash;

  QString res = UnixCommand::getYaourtPackageVersionInformation();

  QStringList listOfPkgs = res.split("\n", QString::SkipEmptyParts);
  foreach (QString line, listOfPkgs)
  {
    if (line.contains(StrConstants::getForeignRepositoryTargetPrefix(), Qt::CaseInsensitive))
    {
      line = line.remove(StrConstants::getForeignRepositoryTargetPrefix());
      QStringList nameVersion = line.split(" ", QString::SkipEmptyParts);
      hash.insert(nameVersion.at(0), nameVersion.at(1));
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
           UnixCommand::getLinuxDistro() == ectn_KAOS)
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
      rsl.replaceInStrings(QRegExp(pkgName + " "), "");
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
