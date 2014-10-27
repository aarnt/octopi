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

#include "strconstants.h"
#include "packagerepository.h"

#include <cassert>
#include <iostream>
#include <QSet>

PackageRepository::PackageRepository()
{
}

void PackageRepository::registerDependency(PackageRepository::IDependency &depends)
{
  m_dependingModels.push_back(&depends);
}

struct BeginResetModel {
  inline void operator()(PackageRepository::IDependency* depends) {
    assert(depends != NULL);
    depends->beginResetRepository();
  }
};

struct TSort {
  bool operator()(const PackageRepository::PackageData* a, const PackageRepository::PackageData* b) const {
    return a->name < b->name;
  }
};

struct EndResetModel {
  inline void operator()(PackageRepository::IDependency* depends) {
    depends->endResetRepository();
  }
};

void PackageRepository::setData(const QList<PackageListData>*const listOfPackages, const QSet<QString>& unrequiredPackages)
{
//  std::cout << "received new package list" << std::endl;

  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());

  // delete items in groups list
  for (QList<Group*>::const_iterator it = m_listOfGroups.begin(); it != m_listOfGroups.end(); ++it) {
    if (*it != NULL) (*it)->invalidateList();
  }
  // delete items in list
  for (TListOfPackages::const_iterator it = m_listOfPackages.begin(); it != m_listOfPackages.end(); ++it) {
    if (*it != NULL) delete *it;
  }
  m_listOfAURPackages.clear();
  m_listOfPackages.clear();

  for (QList<PackageListData>::const_iterator it = listOfPackages->begin(); it != listOfPackages->end(); ++it) {
    m_listOfPackages.push_back(new PackageData(*it, unrequiredPackages.contains(it->name) == false, false));
  }

  qSort(m_listOfPackages.begin(), m_listOfPackages.end(), TSort());
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), EndResetModel());
}

void PackageRepository::setAURData(const QList<PackageListData>*const listOfForeignPackages,
                                   const QSet<QString>& unrequiredPackages)
{
  //  std::cout << "received new foreign package list" << std::endl;

    std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());

    // delete AUR items in list
    for (TListOfPackages::iterator it = m_listOfPackages.begin(); it != m_listOfPackages.end(); ++it) {
      if (*it != NULL && (*it)->managedByAUR) {
        delete *it;
        it = m_listOfPackages.erase(it);
      }
    }
    m_listOfAURPackages.clear();

    for (QList<PackageListData>::const_iterator it = listOfForeignPackages->begin();
         it != listOfForeignPackages->end(); ++it)
    {
      PackageData*const pkg = new PackageData(*it, unrequiredPackages.contains(it->name) == false, true);
      m_listOfPackages.push_back(pkg);
      m_listOfAURPackages.push_back(pkg);
    }

    qSort(m_listOfPackages.begin(), m_listOfPackages.end(), TSort());
    qSort(m_listOfAURPackages.begin(), m_listOfAURPackages.end(), TSort());
    std::for_each(m_dependingModels.begin(), m_dependingModels.end(), EndResetModel());
}

/**
 * @brief if the repository groups differ from %listOfGroups they will be reset
 * @param listOfGroups == group names
 */
void PackageRepository::checkAndSetGroups(const QStringList& listOfGroups)
{
  if (memberListOfGroupsEquals(listOfGroups) == false) {
    std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());
    for (QList<Group*>::const_iterator it = m_listOfGroups.begin(); it != m_listOfGroups.end(); ++it) {
      if (*it != NULL) delete *it;
    }
    m_listOfGroups.clear();

    for (QStringList::const_iterator it = listOfGroups.begin(); it != listOfGroups.end(); ++it) {
      m_listOfGroups.push_back(new Group(*it));
    }
    std::for_each(m_dependingModels.begin(), m_dependingModels.end(), EndResetModel());
  }
}

struct TComp {
  bool operator()(const PackageRepository::PackageData* a, const QString& b) const {
    return a->name < b;
  }
  bool operator()(const QString& b, const PackageRepository::PackageData* a) const {
    return b < a->name;
  }
};

/**
 * @brief checks the PackageRepository if the members of %groupName differ from %members and replaces with %members if necessary
 * @param groupName (name of the group)
 * @param members (should be output of Package::getPackagesOfGroup)
 */
void PackageRepository::checkAndSetMembersOfGroup(const QString& groupName, const QStringList& members)
{
  QList<Group*>::const_iterator groupIt = m_listOfGroups.begin();
  for (; groupIt != m_listOfGroups.end(); ++groupIt) {
    if (*groupIt != NULL && (*groupIt)->getName() == groupName) {
      break;
    }
  }
  if (groupIt != m_listOfGroups.end()) {
    Group& group = **groupIt;
//    std::cout << "processing group " << group.getName().toStdString() << std::endl;
    if (group.memberListEquals(members) == false) {

      // invalidate and register all group members if lists are different
      std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());
      group.invalidateList();

      for (QStringList::const_iterator it = members.begin(); it != members.end(); ++it) {
        typedef TListOfPackages::const_iterator TIter;
        std::pair<TIter, TIter> packageIt =  std::equal_range(m_listOfPackages.begin(), m_listOfPackages.end(), *it, TComp());
        for (TIter iter = packageIt.first; iter != packageIt.second; ++iter) {
          if ((*iter)->managedByAUR == false) {
            group.addPackage(**iter);
            break;
          }
        }
      }
      std::for_each(m_dependingModels.begin(), m_dependingModels.end(), EndResetModel());

    }
  }
  else {
    std::cerr << "did not find package group " << groupName.toStdString() << std::endl;
    assert(false);
  }
}

const PackageRepository::TListOfPackages& PackageRepository::getPackageList() const
{
  return m_listOfPackages;
}

const QList<PackageRepository::PackageData*>& PackageRepository::getPackageList(const QString& group) const
{
//  std::cout << "get package list for group " << group.toStdString() << std::endl;

  if (!group.isEmpty()) {
    QList<Group*>::const_iterator groupIt = m_listOfGroups.begin();
    for (; groupIt != m_listOfGroups.end(); ++groupIt) {
      if (*groupIt != NULL && (*groupIt)->getName() == group) {
        break;
      }
    }
    if (groupIt != m_listOfGroups.end()) {
      Group& group = **groupIt;
      const TListOfPackages* list = group.getPackageList();
      if (list != NULL) return *list;
    }

    // Workaround for AUR filter -> pre-built AUR packageList
    if (group == StrConstants::getForeignToolGroup())
      return m_listOfAURPackages;
  }

  // if no group found or not loaded yet. default to all packages
  return m_listOfPackages;
}

PackageRepository::PackageData* PackageRepository::getFirstPackageByName(const QString name) const
{
  for (TListOfPackages::const_iterator it = m_listOfPackages.begin(); it != m_listOfPackages.end(); ++it) {
    if ((*it)->name == name)
      return *it;
  }
  return NULL;
}

/**
 * @brief checks if the repository groups are up to date
 * @param listOfGroups == group-names
 * @return true if the groups in this repository are the same as the %listOfGroups
 */
bool PackageRepository::memberListOfGroupsEquals(const QStringList& listOfGroups)
{
  if (m_listOfGroups.size() != listOfGroups.size())
    return false;

  QStringList::const_iterator it2 = listOfGroups.begin();
  for (QList<Group*>::const_iterator it = m_listOfGroups.begin(); it != m_listOfGroups.end(); ++it, ++it2) {
    if ((*it)->getName() != *it2)
      return false;
  }

  return true;
}

//////// PackageRepository::PackageData //////////////////////////////

/**
 * @brief conversion from pkg will default the repository to the foreign repo name
 */
PackageRepository::PackageData::PackageData(const PackageListData& pkg, const bool isRequired, const bool isManagedByAUR)
  : required(isRequired), managedByAUR(isManagedByAUR), name(pkg.name),
    repository(pkg.repository.isEmpty() ? StrConstants::getForeignRepositoryName() : pkg.repository),
    version(pkg.version), description(pkg.description.toLatin1()), // octopi wants it converted to utf8
    outdatedVersion(pkg.outatedVersion), downloadSize(pkg.downloadSize),
    status(pkg.status != ectn_OUTDATED ?
           pkg.status :
           (Package::rpmvercmp(pkg.outatedVersion.toLatin1().data(), pkg.version.toLatin1().data()) == 1 ?
             ectn_NEWER : ectn_OUTDATED)),
    popularity(isManagedByAUR ? pkg.popularity : -1),
    popularityString(isManagedByAUR ? QString::number(pkg.popularity) + " " + StrConstants::getVotes() : QString())
{
}

//////// PackageRepository::Group //////////////////////////////

PackageRepository::Group::Group(const QString& grpName)
  : name(grpName), m_listOfPackages(NULL)
{
}

const QString& PackageRepository::Group::getName()
{
  return name;
}

bool PackageRepository::Group::memberListEquals(const QStringList& packagelist)
{
  if (m_listOfPackages == NULL || m_listOfPackages->size() != packagelist.size())
    return false;

  QStringList::const_iterator it2 = packagelist.begin();
  for (TListOfPackages::const_iterator it = m_listOfPackages->begin(); it != m_listOfPackages->end(); ++it, ++it2) {
    if ((*it)->name != *it2)
      return false;
  }

  return true;
}

void PackageRepository::Group::addPackage(PackageRepository::PackageData& package)
{
  if (m_listOfPackages == NULL)
    m_listOfPackages = new TListOfPackages();

  m_listOfPackages->push_back(&package);
}

void PackageRepository::Group::invalidateList()
{
//  std::cout << "invalidate group " << name.toStdString() << std::endl;

  if (m_listOfPackages == NULL)
    return;

  delete m_listOfPackages;
  m_listOfPackages = NULL;
}

const PackageRepository::TListOfPackages* PackageRepository::Group::getPackageList() const
{
  return m_listOfPackages;
}
