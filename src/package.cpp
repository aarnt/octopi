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
#include <QTextStream>
#include <QList>
#include <iostream>

/*
 * Retrieves the basic paclage name, without version numbers
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

  //sb.replace(QRegExp("\n"), "<br>");
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
QStringList *Package::getUnrequiredPackageList()
{
  QString unrequiredPkgList = UnixCommand::getUnrequiredPackageList();
  QStringList packageTuples = unrequiredPkgList.split(QRegExp("\\n"), QString::SkipEmptyParts);
  QStringList * res = new QStringList();

  foreach(QString packageTuple, packageTuples)
  {
    QStringList parts = packageTuple.split(' ');
    {
      res->append(parts[0]); //We only need the package name!
    }
  }

  res->sort();
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

  foreach(QString packageTuple, packageTuples)
  {
    QStringList parts = packageTuple.split(' ');
    {
      res->append(parts[0]); //We only need the package name!
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
    res->append(packageTuple); //We only need the package name!
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
QStringList *Package::getTargetRemovalList(const QString &pkgName)
{
  QString targets = UnixCommand::getTargetRemovalList(pkgName);
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
  QString pkgList = UnixCommand::getPackageList();
  QStringList packageTuples = pkgList.split(QRegExp("\\n"), QString::SkipEmptyParts);
  QList<PackageListData> * res = new QList<PackageListData>();

  foreach(QString packageTuple, packageTuples)
  {
    QStringList parts = packageTuple.split(' ');

    if(parts.size() == 4)
    {
      //This is an installed package!
      res->append(PackageListData(parts[1], parts[0], parts[2], ectn_INSTALLED));
    }
    else if(parts.size() == 5)
    {
      //This is an outdated package!
      QString outVersion = parts[4];
      outVersion.remove(']').trimmed();
      res->append(PackageListData(parts[1], parts[0], parts[2], ectn_OUTDATED, outVersion));
    }
    else
    {
      //This is an non-installed package!
      res->append(PackageListData(parts[1], parts[0], parts[2], ectn_NON_INSTALLED));
    }
  }

  return res;
}

/*
 * Given a QString containing the output of pacman -Si/Qi (pkgInfo),
 * this method returns the contents of the given field (ex: description)
 */
QString Package::extractFieldFromInfo(const QString &field, const QString &pkgInfo)
{
  int fieldPos = pkgInfo.indexOf(field);
  int fieldEnd;
  QString aux;

  if (fieldPos > 0)
  {
    fieldPos = pkgInfo.indexOf(":", fieldPos+1);
    fieldPos+=2;
    aux = pkgInfo.mid(fieldPos);
    fieldEnd = aux.indexOf('\n');
    aux = aux.left(fieldEnd).trimmed();
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
  QString URL = extractFieldFromInfo("URL", pkgInfo);
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
 * Given packageArch, returns if it's a valid one
 */
bool Package::isValidArch(const QString &packageArch)
{
  bool result = false;
  for(uint c=0; c<sizeof(ctn_KNOWN_ARCHS)/sizeof(ctn_KNOWN_ARCHS[0]); c++){
    if (packageArch == ctn_KNOWN_ARCHS[c]){
      result = true;
      break;
    }
  }
  return result;
}

/*
 * Retrieves the file list content of the given package
 */
QStringList Package::getContents(const QString& pkgName)
{
  QStringList rsl;
  QByteArray result = UnixCommand::getPackageContents(pkgName);

  QString aux(result);
  rsl = aux.split("\n", QString::SkipEmptyParts);

  if ( !rsl.isEmpty() ){
    if (rsl.at(0) == "./"){
      rsl.removeFirst();
    }
    rsl.replaceInStrings(QRegExp(pkgName + " "), "");
    rsl.sort();
  }

  return rsl;
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
 * Removes all temporary files used by the app
 */
void Package::removeTempFiles()
{
  QDir d(QDir::tempPath());
  QStringList sl;
  sl << ctn_TEMP_OPEN_FILE_PREFIX + "*";
  QFileInfoList il = d.entryInfoList(sl);

  foreach(QFileInfo fi, il){
    QFile::remove(fi.filePath());
  }
}
