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

#include "packagemodel.h"

#include <iostream>
#include <cassert>

#include "src/uihelper.h"
#include "src/strconstants.h"


PackageModel::PackageModel(const PackageRepository& repo, QObject *parent)
: QAbstractItemModel(parent), m_packageRepo(repo), m_sortOrder(Qt::AscendingOrder),
  m_sortColumn(1), m_filterPackagesNotInstalled(false), m_filterPackagesNotInThisGroup(""),
  m_filterColumn(-1), m_filterRegExp("", Qt::CaseInsensitive, QRegExp::RegExp),
  m_iconNotInstalled(IconHelper::getIconNonInstalled()), m_iconInstalled(IconHelper::getIconInstalled()),
  m_iconInstalledUnrequired(IconHelper::getIconUnrequired()),
  m_iconNewer(IconHelper::getIconNewer()), m_iconOutdated(IconHelper::getIconOutdated()),
  m_iconForeign(IconHelper::getIconForeignGreen()), m_iconForeignOutdated(IconHelper::getIconForeignRed())
{
}

QModelIndex PackageModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if (!parent.isValid())
    return createIndex(row, column, (void*)NULL);


  return QModelIndex();
}

QModelIndex PackageModel::parent(const QModelIndex&) const
{
  return QModelIndex();
}

int PackageModel::rowCount(const QModelIndex &parent) const
{
  if (!parent.isValid()) {
    return m_columnSortedlistOfPackages.size();
  }
  else {
    return 0;
  }
}

int PackageModel::columnCount(const QModelIndex &parent) const
{
  if (!parent.isValid()) {
    return 4;
  }
  else return 0;
}

QVariant PackageModel::data(const QModelIndex &index, int role) const
{
  if (index.isValid() && m_columnSortedlistOfPackages.size() > index.row()) {
    switch (role) {
      case Qt::DisplayRole: {
        const PackageRepository::PackageData*const package = getData(index);
        if (package == NULL)
          return QVariant();

        switch (index.column()) {
          case ctn_PACKAGE_ICON_COLUMN:
            break;
          case ctn_PACKAGE_NAME_COLUMN:
            return QVariant(package->name);
          case ctn_PACKAGE_VERSION_COLUMN:
            return QVariant(package->version);
          case ctn_PACKAGE_REPOSITORY_COLUMN:
            return QVariant(package->repository);
          default:
            assert(false);
        }
        break;
      }
      case Qt::DecorationRole: {
        const PackageRepository::PackageData*const package = getData(index);
        if (package == NULL)
          return QVariant();

        if (index.column() == ctn_PACKAGE_ICON_COLUMN) {
          return QVariant(getIconFor(*package));
        }
        break;
      }
      case Qt::ToolTipRole:
        break;
      case Qt::StatusTipRole:
        break;
      default:
        break;
    }
  }
  return QVariant();
}

QVariant PackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
      case ctn_PACKAGE_ICON_COLUMN:
        return QVariant();
      case ctn_PACKAGE_NAME_COLUMN:
        return QVariant(StrConstants::getName());
      case ctn_PACKAGE_VERSION_COLUMN:
        return QVariant(StrConstants::getVersion());
      case ctn_PACKAGE_REPOSITORY_COLUMN:
        return QVariant(StrConstants::getRepository());
      default:
        break;
      }
    }
    return QVariant(section);
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

void PackageModel::sort(int column, Qt::SortOrder order)
{
//  std::cout << "sort column " << column << " in order " << order << std::endl;

  if (column != m_sortColumn || order != m_sortOrder) {
    m_sortColumn = column;
    m_sortOrder  = order;
    emit layoutAboutToBeChanged();
    sort();
    emit layoutChanged();
  }
}

void PackageModel::beginResetRepository()
{
  beginResetModel();
  m_listOfPackages.clear();
  m_columnSortedlistOfPackages.clear();
}

void PackageModel::endResetRepository()
{
  const QList<PackageRepository::PackageData*>& data = m_packageRepo.getPackageList(m_filterPackagesNotInThisGroup);
  m_listOfPackages.reserve(data.size());
  for (QList<PackageRepository::PackageData*>::const_iterator it = data.begin(); it != data.end(); ++it) {
      if (m_filterPackagesNotInstalled == false || (*it)->status != ectn_NON_INSTALLED) {
        if (m_filterRegExp.isEmpty()) {
          m_listOfPackages.push_back(*it);
        }
        else {
          switch (m_filterColumn) {
            case ctn_PACKAGE_NAME_COLUMN:
              if (m_filterRegExp.indexIn((*it)->name) != -1) m_listOfPackages.push_back(*it);
              break;
            case ctn_PACKAGE_DESCRIPTION_FILTER_NO_COLUMN:
              if (m_filterRegExp.indexIn((*it)->description) != -1) m_listOfPackages.push_back(*it);
              break;
            default:
              m_listOfPackages.push_back(*it);
          }
        }
      }
  }
  m_columnSortedlistOfPackages.reserve(data.size());
  m_columnSortedlistOfPackages = m_listOfPackages;
  sort();
  endResetModel();
}

int PackageModel::getPackageCount() const
{
  return m_listOfPackages.size();
}

const PackageRepository::PackageData* PackageModel::getData(const QModelIndex& index) const
{
  if (index.isValid() && index.row() < m_columnSortedlistOfPackages.size()) {
    switch (m_sortOrder) {
    case Qt::AscendingOrder:
      return m_columnSortedlistOfPackages.at(index.row());
    case Qt::DescendingOrder:
      return m_columnSortedlistOfPackages.at(m_listOfPackages.size() - index.row() - 1);
    }
  }
  return NULL;
}

void PackageModel::applyFilter(bool packagesNotInstalled, const QString& group)
{
//  std::cout << "apply new group filter " << (packagesNotInstalled ? "true" : "false") << ", " << group.toStdString() << std::endl;

  m_filterPackagesNotInstalled   = packagesNotInstalled;
  m_filterPackagesNotInThisGroup = group;
  beginResetRepository();
  endResetRepository();
}

void PackageModel::applyFilter(const int filterColumn)
{
  applyFilter(filterColumn, m_filterRegExp.pattern());
}

void PackageModel::applyFilter(const QString& filterExp)
{
  applyFilter(m_filterColumn, filterExp);
}

void PackageModel::applyFilter(const int filterColumn, const QString& filterExp)
{
  assert(filterExp.isNull() == false);
//  std::cout << "apply new column filter " << filterColumn << ", " << filterExp.toStdString() << std::endl;

  m_filterColumn = filterColumn;
  m_filterRegExp.setPattern(filterExp);
  beginResetRepository();
  endResetRepository();
}

const QIcon& PackageModel::getIconFor(const PackageRepository::PackageData& package) const
{
  switch (package.status)
  {
    case ectn_FOREIGN:
      return m_iconForeign;
    case ectn_FOREIGN_OUTDATED:
      return m_iconForeignOutdated;
    case ectn_OUTDATED:
    {
      //TODO: potential refactoring for performance if necessary
      if (Package::rpmvercmp(package.outdatedVersion.toLatin1().data(), package.version.toLatin1().data()) == 1) {
        return m_iconNewer;
      }
      else {
        return m_iconOutdated;
      }
      assert(false);
      return m_iconOutdated;
    }
    case ectn_INSTALLED:
      // Does no other package depend on this package ? (unrequired package list)
      if (package.required)
      {
        return m_iconInstalled;
      }
      else
      {
        return m_iconInstalledUnrequired;
      }
      break;
    case ectn_NON_INSTALLED:
//      if (package.required == false) std::cout << "not installed not required" << std::endl; // doesn't happen with pacman
      return m_iconNotInstalled;
    default:
      assert(false);
  }
}


struct TSort0 {
  bool operator()(const PackageRepository::PackageData* a, const PackageRepository::PackageData* b) const {
    return a->status < b->status;
  }
};

struct TSort2 {
  bool operator()(const PackageRepository::PackageData* a, const PackageRepository::PackageData* b) const {
    return Package::rpmvercmp(a->version.toLatin1().data(), b->version.toLatin1().data()) < 0;
  }
};

struct TSort3 {
  bool operator()(const PackageRepository::PackageData* a, const PackageRepository::PackageData* b) const {
    return a->repository < b->repository;
  }
};

void PackageModel::sort()
{
  switch (m_sortColumn) {
  case ctn_PACKAGE_NAME_COLUMN:
    m_columnSortedlistOfPackages = m_listOfPackages;
    return;
  case ctn_PACKAGE_ICON_COLUMN:
    qSort(m_columnSortedlistOfPackages.begin(), m_columnSortedlistOfPackages.end(), TSort0());
    return;
  case ctn_PACKAGE_VERSION_COLUMN:
    qSort(m_columnSortedlistOfPackages.begin(), m_columnSortedlistOfPackages.end(), TSort2());
    return;
  case ctn_PACKAGE_REPOSITORY_COLUMN:
    qSort(m_columnSortedlistOfPackages.begin(), m_columnSortedlistOfPackages.end(), TSort3());
    return;
  default:
    return;
  }
}
