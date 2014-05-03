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

#ifndef PACKAGE_H
#define PACKAGE_H

#include "settingsmanager.h"

#include <QStringList>
#include <QSettings>
#include <QDir>
#include <QFileSystemWatcher>
#include <QDateTime>
#include <QHash>
#include <QSet>

const QString ctn_TEMP_ACTIONS_FILE ( QDir::tempPath() + QDir::separator() + ".qt_temp_" );

const QString ctn_PKG_CONTENT_ERROR = "ERROR";
const QString ctn_ER  				   	  = "([\\w._+]+[-])+";
const QString ctn_ER3 				  	  = "[\\w._+]+[-]";
const QString ctn_STRING_RELEASES   = "(alfa|beta|rc|pre|patch|^[0-9]{8}$|(^[rR][0-9]*)|[0-9]*[uU][0-9]*)";
const QString ctn_DATE_RELEASE      = "^[0-9]{8}$";
const QString ctn_NO_MATCH      	  = "not found!";

const QString ctn_PACMAN_DATABASE_DIR = "/var/lib/pacman";

const int ctn_KNOWN_ARCHS_LEN = 8;
const int ctn_KNOWN_NAMES_LEN = 3;

enum SearchPlace { ectn_INSIDE_INSTALLED_PACKAGES, ectn_INSIDE_DIRECTORY, ectn_INSIDE_QSTDITEMMODEL };

enum DumpInstalledPackageListOptions { ectn_WITH_MODIFIED_DATE, ectn_NO_MODIFIED_DATE };

enum PackageStatus { ectn_INSTALLED, ectn_NON_INSTALLED, ectn_OUTDATED, ectn_NEWER,
                     ectn_FOREIGN, ectn_FOREIGN_OUTDATED };

struct PackageListData{
  QString name;
  QString repository;
  QString version;
  QString description;
  QString outatedVersion;
  double  downloadSize;
  int     popularity; //votes
  PackageStatus status;

  PackageListData(){
    name="";
  }

  PackageListData(QString n, QString v, QString dSize){
    name=n;
    version=v;
    downloadSize=QString(dSize).toDouble();
  }

  PackageListData(QString n, QString r, QString v, PackageStatus pkgStatus, QString outVersion=""){
    name=n;
    repository=r;
    version=v;
    status=pkgStatus;
    outatedVersion=outVersion.trimmed();
  }

  PackageListData(QString n, QString r, QString v, QString d, PackageStatus pkgStatus, QString outVersion=""){
    name=n;
    repository=r;
    version=v;
    description=d;
    status=pkgStatus;
    outatedVersion=outVersion.trimmed();
  }
};

struct PackageInfoData{
  QString name;
  QString repository;
  QString version;
  QString url;
  QString license;
  QString group;
  QString provides;
  QString requiredBy;
  QString optionalFor;
  QString dependsOn;
  QString optDepends;
  QString conflictsWith;
  QString replaces;
  QString packager;
  QString arch;
  QString description;
  QDateTime buildDate;
  double downloadSize;
  double installedSize;
};

class Result;

class Package{  
  private:
    static Result verifyPreReleasePackage(const QStringList &versao1,
                                          const QStringList &versao2, const QString &pacote);

    static QString extractFieldFromInfo(const QString &field, const QString &pkgInfo);
    static double simplePow(int base, int exp);

	public:
    static int rpmvercmp(const char *a, const char *b);
    static QSet<QString>* getUnrequiredPackageList();
    static QStringList * getOutdatedPackageList();
    static QStringList * getOutdatedYaourtPackageList();
    static QStringList * getPackageGroups();
    static QStringList * getPackagesOfGroup(const QString &groupName);
    static QList<PackageListData> * getTargetUpgradeList(const QString &pkgName="");
    static QStringList * getTargetRemovalList(const QString &pkgName, const QString &removeCommand);

    static QList<PackageListData> *getForeignPackageList();
    static QList<PackageListData> *getPackageList();

    //Yaourt methods
    static QList<PackageListData> * getYaourtPackageList(const QString& searchString);

    static PackageInfoData getInformation(const QString &pkgName, bool foreignPackage = false);
    static double getDownloadSizeDescription(const QString &pkgName);
    static QString getInformationDescription(const QString &pkgName, bool foreignPackage = false);
    static QHash<QString, QString> getYaourtOutdatedPackagesNameVersion();
    static QStringList getContents(const QString &pkgName, bool isInstalled);

    static QStringList getOptionalDeps(const QString &pkgName);

    static QString getName(const QString &pkgInfo);
    static QString getVersion(const QString &pkgInfo);
    static QString getRepository(const QString &pkgInfo);
    static QString getURL(const QString &pkgInfo);
    static QString getLicense(const QString &pkgInfo);
    static QString getGroup(const QString &pkgInfo);
    static QString getProvides(const QString &pkgInfo);
    static QString getDependsOn(const QString &pkgInfo);
    static QString getOptDepends(const QString &pkgInfo);
    static QString getConflictsWith(const QString &pkgInfo);
    static QString getReplaces(const QString &pkgInfo);
    static QString getRequiredBy(const QString &pkgInfo);
    static QString getOptionalFor(const QString &pkgInfo);
    static QString getPackager(const QString &pkgInfo);
    static QString getArch(const QString &pkgInfo);
    static QString getDescription(const QString &pkgInfo);
    static QDateTime getBuildDate(const QString &pkgInfo);
    static double getDownloadSize(const QString &pkgInfo);
    static double getInstalledSize(const QString &pkgInfo);
    static double humanizeSize(off_t bytes, const char target_unit, int precision, const char **label);
    static QString makeURLClickable(const QString &information);

    static QString getBaseName( const QString& pkgName );
    static QString dumpInstalledPackageList(DumpInstalledPackageListOptions options = ectn_WITH_MODIFIED_DATE);
    static QString parseSearchString( QString searchStr, bool exactMatch = false );
    static bool isSlackPackage(const QString &filePath);
};

#endif
