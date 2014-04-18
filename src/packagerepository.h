/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2014 Thomas Binkau
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

#ifndef OCTOPI_PACKAGEREPOSITORY_H
#define OCTOPI_PACKAGEREPOSITORY_H

#include <vector>
#include <QList>

#include "package.h"


class PackageRepository
{
public:
  class IDependency {
  public:
    virtual void beginResetRepository() = 0;
    virtual void endResetRepository() = 0;
  };

  class PackageData {
  public:
    /**
     * @brief PackageData constructor
     * @param package    = parsed data from pacman (e.g.)
     * @param isRequired = false if package is not required by other packages installed, or true otherwise
     */
    PackageData(const PackageListData& package, const bool isRequired, const bool isManagedByYaourt);

    inline bool installed() const {
      return status != ectn_NON_INSTALLED;
    }

  public:
    const bool    required;
    const bool    managedByYaourt; // yaourt packages must not be in any group
    const QString name;
    const QString repository;
    const QString version;
    const QString description;
    const QString outdatedVersion;
    const double downloadSize;
    const PackageStatus status;
  };

  class Group {
  public:
    Group(const QString& name);

    const QString& getName();
    bool memberListEquals(const QStringList& packagelist);
    void addPackage(PackageData& package);
    void invalidateList();

    const QList<PackageData*>* getPackageList() const;

  private:
    QString name;
    QList<PackageData*>* m_listOfPackages; // WEAK ptr PackageData*
  };

public:
  PackageRepository();

  void registerDependency(IDependency& depends);
  void setData(const QList<PackageListData>*const listOfPackages, const QSet<QString>& unrequiredPackages);
  void setAURData(const QList<PackageListData>*const listOfForeignPackages, const QSet<QString>& unrequiredPackages);
  void checkAndSetGroups(const QStringList& listOfGroups);
  void checkAndSetMembersOfGroup(const QString& group, const QStringList& members);

  const QList<PackageData*>& getPackageList() const;
  const QList<PackageData*>& getPackageList(const QString& group) const;
  const PackageData*         getFirstPackageByName(const QString name) const;

private:
  std::vector<IDependency*> m_dependingModels;
  QList<PackageData*>       m_listOfPackages;       // sorted qlist of all packages
  QList<PackageData*>       m_listOfYaourtPackages; // sorted qlist of all yaourt packages
  QList<Group*>             m_listOfGroups;         // sorted list of all pacman package groups
  bool memberListOfGroupsEquals(const QStringList& listOfGroups);
};

#endif // OCTOPI_PACKAGEREPOSITORY_H
