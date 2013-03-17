/*
* This file is part of Octopi, an open-source GUI for ArchLinux pacman.
* Copyright (C) 2013  Alexandre Albuquerque Arnt
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

const QString ctn_ROOT_HOME("/root");
const char* const ctn_INSTALLED_PACKAGES_DIR 	= "/var/log/packages/";

const char* const ctn_FILELIST    	= "FILE LIST:\n";

const QString ctn_VAR_DIRECTORY = "/var";
const QString ctn_SLACKPKG_CACHE_DIR = "/var/cache/packages";
const QString ctn_SLAPTSRC_TEMP_DIR = "/tmp/SBo";

const QString ctn_TEMP_ACTIONS_FILE ( QDir::tempPath() + QDir::separator() + ".qt_temp_" );
const QString ctn_TEMP_OPEN_FILE_PREFIX = "qtgz_"; //Prefix for temporary opened files from uninstalled packages

const QString ctn_PKG_CONTENT_ERROR = "ERROR";
const QString ctn_ER  				   	  = "([\\w._+]+[-])+";
const QString ctn_ER3 				  	  = "[\\w._+]+[-]";
const QString ctn_STRING_RELEASES   = "(alfa|beta|rc|pre|patch|^[0-9]{8}$|(^[rR][0-9]*)|[0-9]*[uU][0-9]*)";
const QString ctn_DATE_RELEASE      = "^[0-9]{8}$";
const QString ctn_DUMP_FILE					= "installed_packages_list_";
const QString ctn_PACKAGES_DIR   	  = "/var/log/packages";
const QString ctn_NO_MATCH      	  = "not found!";
const QString ctn_TGZ_PACKAGE_EXTENSION = ".tgz"; //The old Slackware package extension (using "gzip compression)
const QString ctn_TXZ_PACKAGE_EXTENSION = ".txz"; //The new Slackware package extension (using "xz compression")
const QString ctn_RPM_PACKAGE_EXTENSION = ".rpm";

const QString ctn_KNOWN_ARCHS[]     = {"noarch", "i386", "i486", "i586", "i686", "x86_64", "arm", "armhfp", "armv5t"};
const QString ctn_KNOWN_NAMES[]   	= {"cdparanoia", "libjpeg", "slib"};

const QString ctn_PKGTOOLS_REMOVE    = "/sbin/removepkg";
const QString ctn_PKGTOOLS_UPGRADE   = "/sbin/upgradepkg";
const QString ctn_PKGTOOLS_INSTALL   = "/sbin/installpkg";
const QString ctn_PKGTOOLS_REINSTALL = "/sbin/upgradepkg --reinstall";

const QString ctn_SPKG               = "spkg";

const QString ctn_SPKG_REMOVE        = " --verbose --remove";
const QString ctn_SPKG_UPGRADE       = " --verbose --upgrade";
const QString ctn_SPKG_INSTALL       = " --verbose --install";
const QString ctn_SPKG_REINSTALL     = " --verbose --reinstall";

const QString ctn_PACMAN_DATABASE_DIR = "/var/lib/pacman";

const int ctn_KNOWN_ARCHS_LEN = 8;
const int ctn_KNOWN_NAMES_LEN = 3;

/*enum Classification { ectn_NOT_INSTALLED, ectn_INSTALLED, ectn_INFERIOR_VERSION, ectn_SUPERIOR_VERSION,
                      ectn_OTHER_VERSION, ectn_OTHER_ARCH, ectn_INTERNAL_ERROR, ectn_FROZEN, ectn_RPM, ectn_DUMP_FILE };*/

enum SearchPlace { ectn_INSIDE_INSTALLED_PACKAGES, ectn_INSIDE_DIRECTORY, ectn_INSIDE_QSTDITEMMODEL };

enum DumpInstalledPackageListOptions { ectn_WITH_MODIFIED_DATE, ectn_NO_MODIFIED_DATE };

enum PackageStatus { ectn_INSTALLED, ectn_NON_INSTALLED, ectn_OUTDATED, ectn_FOREIGN };

struct PackageListData{
  QString name;
  QString repository;
  QString version;
  QString outatedVersion;
  PackageStatus status;

  PackageListData(QString n, QString r, QString v, PackageStatus pkgStatus, QString outVersion=""){
    name=n;
    repository=r;
    version=v;
    status=pkgStatus;
    outatedVersion=outVersion;
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

//Holds the information obtained by the process of opening a snapshot.
struct SnapshotList{
  private:

  QStringList newPackagesList;
    QStringList details;

  public:
    QStringList getNewPackagesList(){ return newPackagesList; }
    QStringList getDetails() { return details; }

    void setNewPackagesList( QStringList n ){ newPackagesList = n; }
    void setDetails( QStringList d ){ details = d; }
};

class Result;

class Package{  
  private:
    static QString showRegExp( const QString&, const QString& );
    static Result verifyPreReleasePackage(const QStringList &versao1,
                                          const QStringList &versao2, const QString &pacote);
    static QDateTime _getModificationDate(const QString packageName);
    static bool isValidArch(const QString &packageArch);

    static QString extractFieldFromInfo(const QString &field, const QString &pkgInfo);
    static double simplePow(int base, int exp);

	public:
    static QStringList * getUnrequiredPackageList();
    static QStringList * getOutdatedPackageList();
    static QStringList * getPackageGroups();
    static QStringList * getPackagesOfGroup(const QString &groupName);
    static QStringList * getTargetUpgradeList(const QString &pkgName="");
    static QStringList * getTargetRemovalList(const QString &pkgName);

    static QList<PackageListData> * getForeignPackageList();
    static QList<PackageListData> * getPackageList();

    static PackageInfoData getInformation(const QString &pkgName, bool foreignPackage = false);
    static QString getInformationDescription(const QString &pkgName, bool foreignPackage = false);
    static QStringList getContents(const QString &pkgName);

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
    static QString getPackager(const QString &pkgInfo);
    static QString getArch(const QString &pkgInfo);
    static QString getDescription(const QString &pkgInfo);
    static QDateTime getBuildDate(const QString &pkgInfo);
    static double getDownloadSize(const QString &pkgInfo);
    static double getInstalledSize(const QString &pkgInfo);
    static double humanizeSize(off_t bytes, const char target_unit, int precision, const char **label);

    static QString makeURLClickable(const QString &information);

    static QStringList getInstalledPackageNames();
    static QString getBaseName( const QString& pkgName );
    static bool isValid( const QString& pkgName );
    static QString dumpInstalledPackageList(DumpInstalledPackageListOptions options = ectn_WITH_MODIFIED_DATE);
    //static Result getStatus( const QString& pkgToVerify );
    static QString parseSearchString( QString searchStr, bool exactMatch = false );
    static bool isSlackPackage(const QString &filePath);
    //static SnapshotList processSnapshotOfInstalledPackageList(QString dumpedList);
    static QString getModificationDate(const QString packageName);
    static void removeTempFiles(); //Remove the temporary opened files from uninstalled packages
};

/*class Result{
  private:
    //Classification classification;
		QString installedPackage;
	
	public:
		Result( Classification c, const QString& p ){
			classification = c;
			installedPackage = p;
		}
	
		Classification getClassification(){
			return classification;
		}
	
		QString getInstalledPackage(){
			return installedPackage;
		}
};*/

class InstalledPkgListSingleton: public QObject {
  Q_OBJECT

  private:
    static InstalledPkgListSingleton *m_pinstance;
    QStringList m_pkgList;

    InstalledPkgListSingleton(const InstalledPkgListSingleton&);
    InstalledPkgListSingleton& operator= (const InstalledPkgListSingleton&);
    InstalledPkgListSingleton();

  public:
    static InstalledPkgListSingleton* instance();
    void setFileSystemWatcher(QFileSystemWatcher* fsw);
    QStringList getFileList();

  public slots:
    void installedPkgDirChanged();
};


/*class FrozenPkgListSingleton : public QStringList {
  private:
		static FrozenPkgListSingleton *m_pinstance;
		
    FrozenPkgListSingleton():QStringList(){
      *this << SettingsManager::instance()->getFrozenPkgList();
		}	

    FrozenPkgListSingleton(const FrozenPkgListSingleton&);
    FrozenPkgListSingleton& operator= (const FrozenPkgListSingleton&);

	public:
    void save(){
      SettingsManager::instance()->setFrozenPkgList(*this);
    }
		
		static FrozenPkgListSingleton* instance(){
      if (m_pinstance == 0) m_pinstance = new FrozenPkgListSingleton();
      return m_pinstance;
		}		
};*/

#endif
