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
#include "constants.h"

#include <QStringList>
#include <QSettings>
#include <QDir>
#include <QFileSystemWatcher>
#include <QDateTime>
#include <QHash>
#include <QSet>

struct PackageListData{
  QString name;
  QString repository;
  QString version;
  QString description;
  QString outatedVersion;
  double  downloadSize;
  double installedSize;
  double buildDate;
  double installDate;
  QString license;
  QString installReason;
  int     popularity; //votes
  PackageStatus status;

  PackageListData() : name(QLatin1String("")),
                    downloadSize(0.0),
                    popularity(0),
                    status(ectn_NON_INSTALLED){
  }

  PackageListData(QString n, QString v, QString dSize) 
                                    : name(n), 
                                    version(v), 
                                    downloadSize(QString(dSize).toDouble()),
                                    popularity(0),
                                    status(ectn_NON_INSTALLED){
  }

  PackageListData(QString n, QString r, QString v, PackageStatus pkgStatus, QString outVersion=QLatin1String(""))
                                    : name(n),
                                    repository(r),
                                    version(v),
                                    outatedVersion(outVersion.trimmed()),
                                    downloadSize(0.0),
                                    popularity(0),
                                    status(pkgStatus){
  }

  PackageListData(QString n, QString r, QString v, QString d, PackageStatus pkgStatus,
                  double downSize, QString outVersion=QLatin1String(""))
                                    : name(n),
                                    repository(r),
                                    version(v),
                                    description(d),
                                    outatedVersion(outVersion.trimmed()),
                                    downloadSize(downSize),
                                    popularity(0),
                                    status(pkgStatus){
  }

  //The latest constructor
  PackageListData(QString n, QString r, QString v, QString d, PackageStatus pkgStatus,
                  double downSize, double instSize, double bDate, double iDate, QString licenses, QString iReason, QString outVersion=QLatin1String(""))
                                    : name(n),
                                    repository(r),
                                    version(v),
                                    description(d),
                                    outatedVersion(outVersion.trimmed()),
                                    downloadSize(downSize),
                                    installedSize(instSize),
                                    buildDate(bDate),
                                    installDate(iDate),
                                    license(licenses),
                                    installReason(iReason),
                                    popularity(0),
                                    status(pkgStatus){
  }

  PackageListData(QString n, QString r, QString v, QString d, PackageStatus pkgStatus, QString outVersion=QLatin1String(""))
                                    : name(n),
                                    repository(r),
                                    version(v),
                                    description(d),
                                    outatedVersion(outVersion.trimmed()),
                                    downloadSize(0.0),
                                    popularity(0),
                                    status(pkgStatus){
  }

  PackageListData(QString n, QString r, QString v, QString d, double instSize, double bDate, double iDate,
                  QString licenses, QString iReason, PackageStatus pkgStatus, QString outVersion=QLatin1String(""))
                                    : name(n),
                                    repository(r),
                                    version(v),
                                    description(d),
                                    outatedVersion(outVersion.trimmed()),
                                    downloadSize(0.0),
                                    installedSize(instSize),
                                    buildDate(bDate),
                                    installDate(iDate),
                                    license(licenses),
                                    installReason(iReason),
                                    popularity(0),
                                    status(pkgStatus){
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
  QDateTime installDate;
  QString installReason;
  QString downloadSize;
  QString installedSize;
  QString downloadSizeAsString;
  QString installedSizeAsString;
};

class Result;

class Package{  
  private:
    static Result verifyPreReleasePackage(const QStringList &versao1,
                                          const QStringList &versao2, const QString &pacote);

    static QString extractFieldFromInfo(const QString &field, const QString &pkgInfo);
    static double simplePow(int base, int exp);

	public:
    static void parseEVR(char *evr, const char **ep, const char **vp, const char **rp);
    static int rpmvercmp(const char *a, const char *b);
    static int alpm_pkg_vercmp(const char *a, const char *b);

    static QSet<QString>* getUnrequiredPackageList();
    static QStringList * getOutdatedStringList();
    static QStringList * getOutdatedAURStringList();
    static QStringList * getPackageGroups();
    static QStringList * getPackagesOfGroup(const QString &groupName);
    static QList<PackageListData> * getTargetUpgradeList(const QString &pkgName = QLatin1String(""));
    static QStringList * getTargetRemovalList(const QString &pkgName, const QString &removeCommand);
    static QList<PackageListData> *getForeignPackageList(QSet<QString> *ignoredPackages);
    static QList<PackageListData> *getPackageList(const QString &packageName, const QHash<QString, QString> *checkUpdatesOutdatedPackages);

    static QList<PackageListData> * getForeignToolPackageList(const QString &searchString);                                //Foreign Tool methods

//#ifdef ALPM_BACKEND
//    static QList<PackageListData> * getAURPackageList(const QString &searchString);                                        //AUR specific pkg list (using rcp)
//#endif

    static QList<PackageListData> * getKcpPackageList(const QString &searchString, const QStringList &packageTuples);      //Kcp specific pkg list
    static QList<PackageListData> * getYayPackageList(const QString &searchString, const QStringList &packageTuples);      //Yay specific pkg list
    static QList<PackageListData> * getParuPackageList(const QString &searchString, const QStringList &packageTuples);     //Paru specific pkg list
    static QHash<QString, QString> getForeignToolOutdatedPackagesNameVersion();    //AUR methods

    static PackageInfoData getKCPInformation(const QString &pkgName);
    static PackageInfoData getInformation(const QString &pkgName, bool foreignPackage = false);
    static double getDownloadSizeDescription(const QString &pkgName);
    static QString getInformationDescription(const QString &pkgName, bool foreignPackage = false);
    static QString getInformationInstalledSize(const QString &pkgName, bool foreignPackage = false);
    static QStringList getContents(const QString &pkgName, bool isInstalled);
    static QStringList getOptionalDeps(const QString &pkgName);
    static QString getName(const QString &pkgInfo);
    static QString getVersion(const QString &pkgInfo);
    static QString getRepository(const QString &pkgInfo);
    static QString getURL(const QString &pkgInfo);
    static QString getLicense(const QString &pkgInfo);
    static QString getMaintainer(const QString &pkgInfo);
    static QString getLastModified(const QString &pkgInfo);
    static QString getOutOfDate(const QString &pkgInfo);
    static QString getGroup(const QString &pkgInfo);
    static QString getProvides(const QString &pkgInfo);
    static QString getDependsOn(const QString &pkgInfo);
    static QString getMakeDeps(const QString &pkgInfo);
    static QString getOptDepends(const QString &pkgInfo);
    static QString getConflictsWith(const QString &pkgInfo);
    static QString getReplaces(const QString &pkgInfo);
    static QString getRequiredBy(const QString &pkgInfo);
    static QString getOptionalFor(const QString &pkgInfo);
    static QString getPackager(const QString &pkgInfo);
    static QString getArch(const QString &pkgInfo);
    static QString getDescription(const QString &pkgInfo);
    static QDateTime getBuildDate(const QString &pkgInfo);
    static QDateTime getInstallDate(const QString &pkgInfo);
    static QString getInstallReason(const QString &pkgInfo);
    static QString getInstallReasonByPkgName(const QString &pkgName);
    static double getDownloadSize(const QString &pkgInfo);
    static QString getDownloadSizeAsString(const QString &pkgInfo);
    static double getInstalledSize(const QString &pkgInfo);
    static QString getInstalledSizeAsString(const QString &pkgInfo);
    static QString kbytesToSize(float Bytes );
    static QString makeAnchorOfOptionalDep(const QString &optionalDeps);
    static QString makeAnchorOfPackage(const QString &packages);
    static QString makeURLClickable(const QString &information);
    static QSet<QString> *extractIgnorePkgList();
    static bool updateIgnorePkgSection(const QSet<QString> *ignoredPackages);
    static QString getBasePkgName(const QString &p);
    static QString getBaseName(const QString& pkgName);
    static QString parseSearchString(QString searchStr, bool exactMatch = false);

    static QString removeColorCodesFromStr(const QString &str);
    static bool hasPacmanDatabase();
    static bool isAURBased();
    static QString getForeignRepositoryToolNameParam();
    static QString getForeignRepositoryToolName();
};

#endif
