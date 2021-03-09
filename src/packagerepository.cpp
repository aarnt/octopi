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

/*
 * This is a data repository where all pacman packages are stored
 * Whenever some data changes, a message is sent to all models that are listening to it
 */

PackageRepository::PackageRepository()
{
}

void PackageRepository::registerDependency(PackageRepository::IDependency &depends)
{
  m_dependingModels.push_back(&depends);
}

struct BeginResetModel
{
  inline void operator()(PackageRepository::IDependency* depends)
  {
    assert(depends != nullptr);
    depends->beginResetRepository();
  }
};

struct TSort
{
  bool operator()(const PackageRepository::PackageData* a, const PackageRepository::PackageData* b) const {
    return a->name < b->name;
  }
};

struct EndResetModel {
  inline void operator()(PackageRepository::IDependency* depends)
  {
    depends->endResetRepository();
  }
};

void PackageRepository::setData(const QList<PackageListData>*const listOfPackages, const QSet<QString>& unrequiredPackages)
{
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());

  // delete items in groups list
  for (QList<Group*>::const_iterator it = m_listOfGroups.constBegin(); it != m_listOfGroups.constEnd(); ++it) {
    if (*it != nullptr) (*it)->invalidateList();
  }
  // delete items in list
  for (TListOfPackages::const_iterator it = m_listOfPackages.constBegin(); it != m_listOfPackages.constEnd(); ++it) {
    if (*it != nullptr) delete *it;
  }
  m_listOfAURPackages.clear();
  m_listOfPackages.clear();

  for (QList<PackageListData>::const_iterator it = listOfPackages->constBegin(); it != listOfPackages->constEnd(); ++it) {
    m_listOfPackages.push_back(new PackageData(*it, !unrequiredPackages.contains(it->name), false));
  }

  std::sort(m_listOfPackages.begin(), m_listOfPackages.end(), TSort());
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), EndResetModel());
}

void PackageRepository::setAURData(const QList<PackageListData>*const listOfForeignPackages,
                                   const QSet<QString>& unrequiredPackages)
{
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());

  //delete AUR items in list
  for (TListOfPackages::iterator it = m_listOfPackages.begin(); it != m_listOfPackages.end(); ++it) {
    if (*it != nullptr && (*it)->managedByAUR) {
      delete *it;
      it = m_listOfPackages.erase(it);
    }
  }
  m_listOfAURPackages.clear();

  for (QList<PackageListData>::const_iterator it = listOfForeignPackages->begin();
       it != listOfForeignPackages->end(); ++it)
  {
    PackageData*const pkg = new PackageData(*it, !unrequiredPackages.contains(it->name), true);
    m_listOfPackages.push_back(pkg);
    m_listOfAURPackages.push_back(pkg);
  }

  std::sort(m_listOfPackages.begin(), m_listOfPackages.end(), TSort());
  std::sort(m_listOfAURPackages.begin(), m_listOfAURPackages.end(), TSort());
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), EndResetModel());
}

/*
 * Iterates over the package list to mark foreign packages
 */
void PackageRepository::setForeignData(QList<PackageListData>*const listOfForeignPackages,
                                           const QStringList& outdatedAURPackages)
{
  Q_UNUSED(outdatedAURPackages)
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());

  //delete AUR items in list
  for (TListOfPackages::iterator it = m_listOfPackages.begin(); it != m_listOfPackages.end(); ++it) {
    if (*it != nullptr && (*it)->managedByAUR)
    {
      delete *it;
      it = m_listOfPackages.erase(it);
      --it;
    }
  }
  m_listOfAURPackages.clear();

  for (QList<PackageListData>::iterator it = listOfForeignPackages->begin();
       it != listOfForeignPackages->end(); ++it)
  {
    PackageData*const pkg = new PackageData(*it, true, true);
    m_listOfPackages.push_back(pkg);
    m_listOfAURPackages.push_back(pkg);
  }

  std::sort(m_listOfPackages.begin(), m_listOfPackages.end(), TSort());
  std::sort(m_listOfAURPackages.begin(), m_listOfAURPackages.end(), TSort());
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), EndResetModel());
}

/*
 * Iterates over the package list to mark outdated packages (returned by "checkupdates")
 */
void PackageRepository::setOutdatedData(const QHash<QString, QString> &outdatedPackages)
{
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());
  QList<PackageListData*> newPkgs;

  for (TListOfPackages::iterator it = m_listOfPackages.begin(); it != m_listOfPackages.end(); ++it)
  {
    if (*it != nullptr && outdatedPackages.contains((*it)->name))
    {
      PackageListData *pld = new PackageListData();
      pld->status=ectn_OUTDATED;
      pld->name=(*it)->name;
      pld->description=(*it)->description;
      pld->outatedVersion=(*it)->version;
      pld->version=outdatedPackages.value((*it)->name);

      if (pld->version == pld->outatedVersion)
        pld->outatedVersion=(*it)->outdatedVersion;

      pld->repository=(*it)->repository;
      pld->downloadSize=(*it)->downloadSize;
      pld->license=(*it)->license;
      pld->installedSize=(*it)->installedSize;
      pld->buildDate=(*it)->buildDate;
      pld->installDate=(*it)->installDate;
      pld->installReason=(*it)->installReason;
      newPkgs.append(pld);

      delete *it;
      it = m_listOfPackages.erase(it);
      --it;
    }
  }

  for (int c=0; c<newPkgs.count(); ++c)
  {
    PackageData*const pkg = new PackageData(*newPkgs.at(c), true, false);
    m_listOfPackages.push_back(pkg);
  }

  std::sort(m_listOfPackages.begin(), m_listOfPackages.end(), TSort());
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), EndResetModel());
}

/*
 * Iterates over the package list to mark outdated foreign packages
 */
void PackageRepository::setAUROutdatedData(QList<PackageListData>*const listOfForeignPackages,
                                           const QStringList& outdatedAURPackages)
{
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());

  //delete AUR items in list
  for (TListOfPackages::iterator it = m_listOfPackages.begin(); it != m_listOfPackages.end(); ++it)
  {
    if (*it != nullptr && ((*it)->status == ectn_FOREIGN || (*it)->status == ectn_FOREIGN_OUTDATED)) {
      delete *it;
      it = m_listOfPackages.erase(it);
      --it;
    }
  }
  m_listOfAURPackages.clear();

  for (QList<PackageListData>::iterator it = listOfForeignPackages->begin();
       it != listOfForeignPackages->end(); ++it)
  {
    if (outdatedAURPackages.contains(it->name))
    {
      it->status = ectn_FOREIGN_OUTDATED;
    }

    PackageData*const pkg = new PackageData(*it, true, true);
    m_listOfPackages.push_back(pkg);
    m_listOfAURPackages.push_back(pkg);
  }

  std::sort(m_listOfPackages.begin(), m_listOfPackages.end(), TSort());
  std::sort(m_listOfAURPackages.begin(), m_listOfAURPackages.end(), TSort());
  std::for_each(m_dependingModels.begin(), m_dependingModels.end(), EndResetModel());
}

/**
 * @brief if the repository groups differ from %listOfGroups they will be reset
 * @param listOfGroups == group names
 */
void PackageRepository::checkAndSetGroups(const QStringList& listOfGroups)
{
  if (!memberListOfGroupsEquals(listOfGroups))
  {
    std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());
    for (QList<Group*>::const_iterator it = m_listOfGroups.constBegin(); it != m_listOfGroups.constEnd(); ++it)
    {
      if (*it != nullptr) delete *it;
    }
    m_listOfGroups.clear();

    for (QStringList::const_iterator it = listOfGroups.begin(); it != listOfGroups.end(); ++it)
    {
      m_listOfGroups.push_back(new Group(*it));
    }
    std::for_each(m_dependingModels.begin(), m_dependingModels.end(), EndResetModel());
  }
}

struct TComp {
  bool operator()(const PackageRepository::PackageData* a, const QString& b) const
  {
    return a->name < b;
  }
  bool operator()(const QString& b, const PackageRepository::PackageData* a) const
  {
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
  QList<Group*>::const_iterator groupIt = m_listOfGroups.constBegin();
  for (; groupIt != m_listOfGroups.constEnd(); ++groupIt) {
    if (*groupIt != nullptr && (*groupIt)->getName() == groupName)
    {
      break;
    }
  }
  if (groupIt != m_listOfGroups.constEnd())
  {
    Group& group = **groupIt;
    if (!group.memberListEquals(members))
    {

      // invalidate and register all group members if lists are different
      std::for_each(m_dependingModels.begin(), m_dependingModels.end(), BeginResetModel());
      group.invalidateList();

      for (QStringList::const_iterator it = members.begin(); it != members.end(); ++it)
      {
        typedef TListOfPackages::const_iterator TIter;
        std::pair<TIter, TIter> packageIt =  std::equal_range(m_listOfPackages.begin(), m_listOfPackages.end(), *it, TComp());
        for (TIter iter = packageIt.first; iter != packageIt.second; ++iter)
        {
          if (!(*iter)->managedByAUR)
          {
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
  if (!group.isEmpty())
  {
    QList<Group*>::const_iterator groupIt = m_listOfGroups.begin();
    for (; groupIt != m_listOfGroups.end(); ++groupIt)
    {
      if (*groupIt != nullptr && (*groupIt)->getName() == group) {
        break;
      }
    }
    if (groupIt != m_listOfGroups.end())
    {
      Group& group = **groupIt;
      const TListOfPackages* list = group.getPackageList();
      if (list != nullptr) return *list;
    }

    // Workaround for AUR filter -> pre-built AUR packageList
    if (group == StrConstants::getForeignToolGroup())
      return m_listOfAURPackages;
  }

  // if no group found or not loaded yet. default to all packages
  return m_listOfPackages;
}

PackageRepository::PackageData* PackageRepository::getFirstPackageByName(const QString &name) const
{
  for (TListOfPackages::const_iterator it = m_listOfPackages.begin(); it != m_listOfPackages.end(); ++it)
  {
    if ((*it)->name == name)
      return *it;
  }
  return nullptr;
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

  QStringList::const_iterator it2 = listOfGroups.constBegin();
  for (QList<Group*>::const_iterator it = m_listOfGroups.constBegin(); it != m_listOfGroups.constEnd(); ++it, ++it2)
  {
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
    version(pkg.version), description(pkg.description), // octopi wants it converted to utf8
    outdatedVersion(pkg.outatedVersion), downloadSize(pkg.downloadSize), installedSize(pkg.installedSize),
    buildDate(pkg.buildDate), installDate(pkg.installDate), license(pkg.license), installReason(pkg.installReason),
    status(pkg.status != ectn_OUTDATED ?
           pkg.status :
           (Package::alpm_pkg_vercmp(pkg.outatedVersion.toLatin1().data(), pkg.version.toLatin1().data()) == 1 ?
             ectn_NEWER : ectn_OUTDATED)),
    popularity(isManagedByAUR ? pkg.popularity : -1),
    popularityString(isManagedByAUR ? QString::number(pkg.popularity) : QString())
{
}

//////// PackageRepository::Group //////////////////////////////

PackageRepository::Group::Group(const QString& grpName)
  : name(grpName), m_listOfPackages(nullptr)
{
}

const QString& PackageRepository::Group::getName()
{
  return name;
}

bool PackageRepository::Group::memberListEquals(const QStringList& packagelist)
{
  if (m_listOfPackages == nullptr || m_listOfPackages->size() != packagelist.size())
    return false;

  QStringList::const_iterator it2 = packagelist.constBegin();
  for (TListOfPackages::const_iterator it = m_listOfPackages->constBegin(); it != m_listOfPackages->constEnd(); ++it, ++it2)
  {
    if ((*it)->name != *it2)
      return false;
  }

  return true;
}

void PackageRepository::Group::addPackage(PackageRepository::PackageData& package)
{
  if (m_listOfPackages == nullptr)
    m_listOfPackages = new TListOfPackages();

  m_listOfPackages->push_back(&package);
}

void PackageRepository::Group::invalidateList()
{
  if (m_listOfPackages == nullptr)
    return;

  m_listOfPackages->clear();
  delete m_listOfPackages;
  m_listOfPackages = nullptr;
}

const PackageRepository::TListOfPackages* PackageRepository::Group::getPackageList() const
{
  return m_listOfPackages;
}
