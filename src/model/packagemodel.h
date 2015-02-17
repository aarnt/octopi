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

#ifndef OCTOPI_PACKAGEMODEL_H
#define OCTOPI_PACKAGEMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

#include "src/package.h"
#include "src/packagerepository.h"

class PackageModel : public QAbstractItemModel, public PackageRepository::IDependency
{
  Q_OBJECT

public:
  // Column indices for Package's treeview
  static const int ctn_PACKAGE_ICON_COLUMN        = 0;
  static const int ctn_PACKAGE_NAME_COLUMN        = 1;
  static const int ctn_PACKAGE_VERSION_COLUMN     = 2;
  static const int ctn_PACKAGE_REPOSITORY_COLUMN  = 3;
  static const int ctn_PACKAGE_POPULARITY_COLUMN  = 4;
  // Pseudo Column indices for additional filter criterias
  static const int ctn_PACKAGE_DESCRIPTION_FILTER_NO_COLUMN = 5;

public:
  explicit PackageModel(const PackageRepository& repo, QObject* parent = 0);

signals:

public slots:

  // QAbstractItemModel interface
public:
  virtual QModelIndex index(int row, int column, const QModelIndex& parent) const /*override*/;
  virtual QModelIndex parent(const QModelIndex& child) const /*override*/;
  virtual int rowCount(const QModelIndex& parent) const /*override*/;
  virtual int columnCount(const QModelIndex& parent) const /*override*/;
  virtual QVariant data(const QModelIndex& index, int role) const /*override*/;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const /*override*/;
  virtual void sort(int column, Qt::SortOrder order) /*override*/;

  void clear();

  // IDependency interface
public:
  virtual void beginResetRepository() /*override*/;
  virtual void endResetRepository()   /*override*/;

  // Getter
public:
  int getPackageCount() const;
  int getInstalledPackagesCount() const;
  bool isFiltered() const;

  const PackageRepository::PackageData* getData(const QModelIndex& index) const;

  // Setter
public:
  void applyFilter(ViewOptions pkgViewOptions, const QString& repo, const QString& group);
  void applyFilter(bool packagesNotInstalled, const QString& group);
  void applyFilter(const int filterColumn);
  void applyFilter(const QString& filterExp);
  void applyFilter(const int filterColumn, const QString& filterExp);

  void setShowColumnPopularity(bool value);

private:
  const QIcon& getIconFor(const PackageRepository::PackageData& package) const;
  void sort();

private:
  int                                     m_installedPackagesCount;
  bool                                    m_showColumnPopularity;
  const PackageRepository&                m_packageRepo;
  QList<PackageRepository::PackageData*>  m_listOfPackages;             // should be provided sorted by name (by repo)
  QList<PackageRepository::PackageData*>  m_columnSortedlistOfPackages; // sorted by column

  // Filter / Sort attributes
  Qt::SortOrder m_sortOrder;
  int           m_sortColumn;
  bool    m_filterPackagesInstalled;
  bool    m_filterPackagesNotInstalled;
  QString m_filterPackagesNotInThisGroup;
  QString m_filterPackagesNotInThisRepo;
  int     m_filterColumn;
  QRegExp m_filterRegExp;

  // Cache
  QIcon   m_iconNotInstalled;
  QIcon   m_iconInstalled;
  QIcon   m_iconInstalledUnrequired;
  QIcon   m_iconNewer;
  QIcon   m_iconOutdated;
  QIcon   m_iconForeign;
  QIcon   m_iconForeignOutdated;
};

#endif // OCTOPI_PACKAGEMODEL_H
