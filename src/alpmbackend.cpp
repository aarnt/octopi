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

#include "alpmbackend.h"
#include "strconstants.h"

#include <alpm.h>
#include <alpm_list.h>
#include <alpm_octopi_utils.h>

/*
 * This class encapsulates ALPM methods to retrieve package information
 */

AlpmBackend::AlpmBackend()
{

}

/*
 * Retrieves all packages available (excluding foreign ones)
 */
QStringList AlpmBackend::getPackageList()
{
  QStringList res;

  // create AlpmUtils instance
  AlpmUtils* alpm_utils = alpm_utils_new ("/etc/pacman.conf");
  // return a alpm_list of alpm_pkg, see alpm.h and alpm_list.h
  alpm_list_t* founds = alpm_utils_search_all_dbs (alpm_utils, "\\S");

  // display pkgs information
  // return a alpm_list of alpm_pkg, see alpm.h and alpm_list.h
  //alpm_list_t* founds = alpm_utils_get_all_pkgs(alpm_utils);
  //alpm_list_t* founds = alpm_utils_get_group_pkgs(alpm_utils, "base-devel");
  //alpm_list_t* founds = alpm_utils_get_updates(alpm_utils);
  //alpm_list_t* founds = alpm_utils_get_orphans(alpm_utils);
  //alpm_list_t* founds = alpm_utils_get_unrequired(alpm_utils);
  //alpm_list_t* founds = alpm_utils_get_foreign_pkgs(alpm_utils);
  //alpm_pkg_t* pkg = alpm_utils_get_installed_pkg(alpm_utils, "octopi");

  alpm_list_t *i, *l;
  const char* installedVersion;
  const char* repoVersion;
  QString bInstalled, installReason;
  const char* pkgName;
  alpm_pkg_t* instPkg;
  char size[20], instSize[20], bDate[20], iDate[20];
  QString line;
  alpm_time_t installDate;
  char *temp;
  QString strLicences;

  for (i = founds; i; i = alpm_list_next(i))
  {
    alpm_pkg_t* pkg = (alpm_pkg_t*) i->data;
    alpm_db_t* db = alpm_pkg_get_db(pkg);
    const char* dbname = alpm_db_get_name(db);
    if (!strcmp(dbname, "local")) continue;

    repoVersion = alpm_pkg_get_version(pkg),
    pkgName = alpm_pkg_get_name(pkg),
    instPkg = (alpm_utils_get_installed_pkg(alpm_utils, pkgName));

    if (instPkg)
    {
      bInstalled = QStringLiteral("i");
      installedVersion = alpm_pkg_get_version(instPkg);

      alpm_pkg_t* iPkg = alpm_utils_get_installed_pkg(alpm_utils, pkgName);
      installDate = alpm_pkg_get_installdate(iPkg);
      std::sprintf(iDate, "%ld", installDate);

      alpm_pkgreason_t reason = alpm_pkg_get_reason(instPkg);
      if (reason==ALPM_PKG_REASON_EXPLICIT) installReason=StrConstants::getExplicitly();
      else installReason=StrConstants::getAsDependency();

      if (!strcmp(repoVersion, installedVersion))
      {
        installedVersion = "same_version";
      }
      else
      {
        bInstalled = QStringLiteral("o");
      }
    }
    else
    {
      bInstalled = QStringLiteral("n");
      installedVersion = "same_version";
      installDate = 0;
      installReason.clear();
      std::sprintf(iDate, "%ld", installDate);
    }

    off_t pkgSize = alpm_pkg_get_size(pkg);
    std::sprintf(size, "%ld", pkgSize);

    off_t pkgInstSize = alpm_pkg_get_isize(pkg);
    std::sprintf(instSize, "%ld", pkgInstSize);

    alpm_time_t buildDate = alpm_pkg_get_builddate(pkg);
    std::sprintf(bDate, "%ld", buildDate);

    alpm_list_t *licenses = alpm_pkg_get_licenses(pkg);
    strLicences.clear();

    for (l=licenses; l; l=alpm_list_next(l))
    {
      temp = reinterpret_cast <char*>(l->data);
      strLicences += QLatin1String(temp) + QLatin1String(" ");
    }

    line = bInstalled + QStringLiteral("<o'o>") +
           QString::fromUtf8(dbname) + QStringLiteral("<o'o>") +
           QString::fromUtf8(pkgName) + QStringLiteral("<o'o>") +
           QString::fromUtf8(repoVersion) + QStringLiteral("<o'o>") +
           QString::fromUtf8(installedVersion) + QStringLiteral("<o'o>") +
           QString::fromUtf8(size) + QStringLiteral("<o'o>") +
           QString::fromUtf8(instSize) + QStringLiteral("<o'o>") +
           QString::fromUtf8(bDate) + QStringLiteral("<o'o>") +
           QString::fromUtf8(iDate) + QStringLiteral("<o'o>") +
           strLicences + QStringLiteral("<o'o>") +
           installReason;

    res.append(line);
    line = QLatin1Char('\t') + QString::fromUtf8(alpm_pkg_get_desc(pkg));
    res.append(line);
  }

  // free
  alpm_utils_free (alpm_utils); // this will free all alpm_pkgs but not the alpm_list
  alpm_list_free (founds);

  return res;
}

/*
 * Retrieves unrequired packages (pacman -Qt)
 */
QStringList AlpmBackend::getUnrequiredList()
{
  QStringList res;

  // create AlpmUtils instance
  AlpmUtils* alpm_utils = alpm_utils_new ("/etc/pacman.conf");

  // return a alpm_list of alpm_pkg, see alpm.h and alpm_list.h
  alpm_list_t* founds = alpm_utils_get_unrequired(alpm_utils);

  // display pkgs information
  alpm_list_t* i;
  const char* pkgName;

  for (i = founds; i; i = alpm_list_next(i))
  {
    alpm_pkg_t* pkg = (alpm_pkg_t*) i->data;
    pkgName = alpm_pkg_get_name(pkg),
    res.append(QString::fromUtf8(pkgName));
  }

  // free
  alpm_utils_free (alpm_utils); // this will free all alpm_pkgs but not the alpm_list
  alpm_list_free (founds);

  return res;
}

/*
 * Retrieves non-db packages
 */
QStringList AlpmBackend::getForeignList()
{
  QStringList res;

  // create AlpmUtils instance
  AlpmUtils* alpm_utils = alpm_utils_new ("/etc/pacman.conf");

  // return a alpm_list of alpm_pkg, see alpm.h and alpm_list.h
  alpm_list_t* founds = alpm_utils_get_foreign_pkgs(alpm_utils);

  // display pkgs information
  alpm_list_t *i, *l;
  QString version;
  QString line;
  QString description;
  char *temp;
  QString strLicences;
  QString installReason;
  alpm_time_t installDate;
  char instSize[20], bDate[20], iDate[20];

  for (i = founds; i; i = alpm_list_next(i))
  {
    alpm_pkg_t* pkg = (alpm_pkg_t*) i->data;
    const char* pkgName = alpm_pkg_get_name(pkg);
    version = QString::fromUtf8(alpm_pkg_get_version(pkg));
    description = QString::fromUtf8(alpm_pkg_get_desc(pkg));

    off_t pkgInstSize = alpm_pkg_get_isize(pkg);
    std::sprintf(instSize, "%ld", pkgInstSize);

    alpm_time_t buildDate = alpm_pkg_get_builddate(pkg);
    std::sprintf(bDate, "%ld", buildDate);

    alpm_pkg_t* iPkg = alpm_utils_get_installed_pkg(alpm_utils, pkgName);
    installDate = alpm_pkg_get_installdate(iPkg);
    std::sprintf(iDate, "%ld", installDate);
    alpm_list_t *licenses = alpm_pkg_get_licenses(iPkg);
    strLicences.clear();
    for (l=licenses; l; l=alpm_list_next(l))
    {
      temp = reinterpret_cast <char*>(l->data);
      strLicences += QLatin1String(temp) + QLatin1String(" ");
    }

    alpm_pkgreason_t reason = alpm_pkg_get_reason(iPkg);
    if (reason==ALPM_PKG_REASON_EXPLICIT) installReason=StrConstants::getExplicitly();
    else installReason=StrConstants::getAsDependency();

    line = QString::fromUtf8(pkgName) + QLatin1String("<o'o>") +
        QString(version) + QLatin1String("<o'o>") +
        QString(description) + QLatin1String("<o'o>") +
        QString::fromUtf8(instSize) + QLatin1String("<o'o>") +
        QString::fromUtf8(bDate) + QLatin1String("<o'o>") +
        QString::fromUtf8(iDate) + QLatin1String("<o'o>") +
        strLicences + QLatin1String("<o'o>") +
        installReason;

    res.append(line);
  }

  // free
  alpm_utils_free (alpm_utils); // this will free all alpm_pkgs but not the alpm_list
  alpm_list_free (founds);

  return res;
}

/*
 * Retrieves outdated packages
 */
QStringList AlpmBackend::getOutdatedList()
{
  QStringList res;

  // create AlpmUtils instance
  AlpmUtils* alpm_utils = alpm_utils_new ("/etc/pacman.conf");

  // return a alpm_list of alpm_pkg, see alpm.h and alpm_list.h
  alpm_list_t* founds = alpm_utils_get_updates(alpm_utils);

  // display pkgs information
  alpm_list_t* i;
  const char* pkgName;

  for (i = founds; i; i = alpm_list_next(i))
  {
    alpm_pkg_t* pkg = (alpm_pkg_t*) i->data;
    pkgName = alpm_pkg_get_name(pkg),
    res.append(QString::fromUtf8(pkgName));
  }

  // free
  alpm_utils_free (alpm_utils); // this will free all alpm_pkgs but not the alpm_list
  alpm_list_free (founds);

  return res;
}

/*
 * Retrieves package download size
 */
double AlpmBackend::getPackageSize(const QString &pkgName)
{
  // create AlpmUtils instance
  AlpmUtils* alpm_utils = alpm_utils_new ("/etc/pacman.conf");
  alpm_list_t* i;
  off_t pkgSize=0;

  std::string str = pkgName.toStdString();
  const char* p = str.c_str();

  alpm_list_t* founds = alpm_utils_search_all_dbs(alpm_utils, p);

  for (i = founds; i; i = alpm_list_next(i))
  {
    alpm_pkg_t* pkg = (alpm_pkg_t*) i->data;
    alpm_db_t* db = alpm_pkg_get_db(pkg);

    const char* dbname = alpm_db_get_name(db);
    if (!strcmp(dbname, "local")) continue;

    if (pkg && strcmp(alpm_pkg_get_name(pkg),pkgName.toLatin1().constData())==0)
    {
      pkgSize = alpm_pkg_get_size(pkg);
      break;
    }
  }

  // free
  alpm_utils_free (alpm_utils); // this will free all alpm_pkgs but not the alpm_list
  alpm_list_free (founds);

  return pkgSize;
}

QString AlpmBackend::getPackageVersion(const QString &pkgName)
{
  // create AlpmUtils instance
  AlpmUtils* alpm_utils = alpm_utils_new ("/etc/pacman.conf");
  alpm_list_t* i;
  QString pkgVersion;

  std::string str = pkgName.toStdString();
  const char* p = str.c_str();

  alpm_list_t* founds = alpm_utils_search_all_dbs(alpm_utils, p);

  for (i = founds; i; i = alpm_list_next(i))
  {
    alpm_pkg_t* pkg = (alpm_pkg_t*) i->data;
    alpm_db_t* db = alpm_pkg_get_db(pkg);

    const char* dbname = alpm_db_get_name(db);
    if (!strcmp(dbname, "local")) continue;

    if (pkg && strcmp(alpm_pkg_get_name(pkg),pkgName.toLatin1().constData())==0)
    {
      pkgVersion = QString::fromUtf8(alpm_pkg_get_version(pkg));
      break;
    }
  }

  // free
  alpm_utils_free (alpm_utils); // this will free all alpm_pkgs but not the alpm_list
  alpm_list_free (founds);

  return pkgVersion;
}

/*
 * Retrieves package information a la "pacman -Qi/-Si"
 */
/*PackageInfoData AlpmBackend::getPackageInfo(const QString &pkgName, bool isForeign)
{
  PackageInfoData info;
  // create AlpmUtils instance
  AlpmUtils* alpm_utils = alpm_utils_new ("/etc/pacman.conf");

  alpm_list_t *i, *j;
  char size[20];

  std::string str = pkgName.toStdString();
  const char* p = str.c_str();

  alpm_list_t *founds = alpm_utils_search_all_dbs(alpm_utils, p);

  for (i = founds; i; i = alpm_list_next(i))
  {
    alpm_pkg_t* pkg = (alpm_pkg_t*) i->data;
    alpm_db_t* db = alpm_pkg_get_db(pkg);
    const char* dbname = alpm_db_get_name(db);
    if (!isForeign && !strcmp(dbname, "local")) continue;

    if (pkg && strcmp(alpm_pkg_get_name(pkg),pkgName.toLatin1())==0)
    {
      info.url = alpm_pkg_get_url(pkg);
      info.arch = alpm_pkg_get_arch(pkg);
      char *temp;
      QString strLicences;

      alpm_list_t *licenses = alpm_pkg_get_groups(pkg);
      for (j=licenses; j; j=alpm_list_next(j))
      {
        temp = reinterpret_cast <char*>(j->data);
        strLicences += QString(temp) + " ";
      }

      info.packager = alpm_pkg_get_packager(pkg);
      alpm_time_t buildDate = alpm_pkg_get_builddate(pkg);
      info.buildDate = QDateTime::fromTime_t(buildDate);

      std::sprintf(size, "%ld", alpm_pkg_get_size(pkg));
      info.downloadSizeAsString = size;
      info.downloadSize = alpm_pkg_get_size(pkg);
      std::sprintf(size, "%ld", alpm_pkg_get_isize(pkg));
      info.installedSizeAsString = size;
      info.installedSize = alpm_pkg_get_isize(pkg);

      break;
    }
  }

  // free
  alpm_utils_free (alpm_utils); // this will free all alpm_pkgs but not the alpm_list
  alpm_list_free (founds);

  return info;
}*/
