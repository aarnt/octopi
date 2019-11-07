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
  //alpm_list_t* founds = alpm_utils_get_all_pkgs(alpm_utils);
  //alpm_list_t* founds = alpm_utils_get_group_pkgs(alpm_utils, "base-devel");
  //alpm_list_t* founds = alpm_utils_get_updates(alpm_utils);
  //alpm_list_t* founds = alpm_utils_get_orphans(alpm_utils);
  //alpm_list_t* founds = alpm_utils_get_unrequired(alpm_utils);
  //alpm_list_t* founds = alpm_utils_get_foreign_pkgs(alpm_utils);
  //alpm_pkg_t* pkg = alpm_utils_get_installed_pkg(alpm_utils, "octopi");

  // display pkgs information
  alpm_list_t* i;
  const char* installedVersion;
  const char* repoVersion;
  QString bInstalled;
  const char* pkgName;
  alpm_pkg_t* instPkg;
  QString line;

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
      bInstalled = "i";
      installedVersion = alpm_pkg_get_version(instPkg);

      if (!strcmp(repoVersion, installedVersion))
      {
        installedVersion = "same_version";
      }
      else
      {
        bInstalled = "o";
      }
    }
    else
    {
      bInstalled = "n";
      installedVersion = "same_version";
    }

    char size[10];
    off_t pkgSize = alpm_pkg_get_size(pkg);

    std::sprintf(size, "%ld", pkgSize);

    line = bInstalled + " " + dbname + " " +
           pkgName + " " + repoVersion + " " +
           installedVersion + " " + QString(size);

    res.append(line);
    line = "\t" + QString(alpm_pkg_get_desc(pkg));
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
    res.append(QString(pkgName));
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
  alpm_list_t* i;  
  QString version;
  QString line;
  QString description;

  for (i = founds; i; i = alpm_list_next(i))
  {
    alpm_pkg_t* pkg = (alpm_pkg_t*) i->data;
    const char* pkgName = alpm_pkg_get_name(pkg);
    version = alpm_pkg_get_version(pkg);
    description = alpm_pkg_get_desc(pkg);

    line = QString(pkgName) + "<o'o>" + QString(version) + "<o'o>" + QString(description);

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
    res.append(QString(pkgName));
  }

  // free
  alpm_utils_free (alpm_utils); // this will free all alpm_pkgs but not the alpm_list
  alpm_list_free (founds);

  return res;
}

/*
 * Retrieves package download size
 */
QString AlpmBackend::getPackageSize(const QString &pkgName)
{
  // create AlpmUtils instance
  AlpmUtils* alpm_utils = alpm_utils_new ("/etc/pacman.conf");

  alpm_list_t* i;
  char size[10];

  std::string str = pkgName.toStdString();
  const char* p = str.c_str();

  alpm_list_t* founds = alpm_utils_search_all_dbs(alpm_utils, p);

  for (i = founds; i; i = alpm_list_next(i))
  {
    alpm_pkg_t* pkg = (alpm_pkg_t*) i->data;
    alpm_db_t* db = alpm_pkg_get_db(pkg);

    const char* dbname = alpm_db_get_name(db);
    if (!strcmp(dbname, "local")) continue;

    if (pkg && strcmp(alpm_pkg_get_name(pkg),pkgName.toLatin1())==0)
    {
      off_t pkgSize = alpm_pkg_get_size(pkg);
      std::sprintf(size, "%ld", pkgSize);
      break;
    }
  }

  // free
  alpm_utils_free (alpm_utils); // this will free all alpm_pkgs but not the alpm_list
  alpm_list_free (founds);

  return QString(size);
}

/*
 * Retrieves package information a la "pacman -Qi/-Si"
 */
PackageInfoData AlpmBackend::getPackageInfo(const QString &pkgName, bool isForeign)
{
  PackageInfoData info;
  // create AlpmUtils instance
  AlpmUtils* alpm_utils = alpm_utils_new ("/etc/pacman.conf");

  alpm_list_t *i, *j;
  char size[10];

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
      /*char *temp;
      QString strLicences;

      alpm_list_t *licenses = alpm_pkg_get_groups(pkg);
      for (j=licenses; j; j=alpm_list_next(j))
      {
        temp = reinterpret_cast <char*>(j->data);
        strLicences += QString(temp) + " ";
      }*/

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
}
