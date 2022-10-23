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

#include "treeviewpackagesitemdelegate.h"
#include "globals.h"
#include "mainwindow.h"
#include "uihelper.h"

#include <QtGui>
#include <QFutureWatcher>
#include <QTreeWidget>
#include <QToolTip>
#include <iostream>
#include <QtConcurrent/QtConcurrentRun>

QPoint gPoint;

TreeViewPackagesItemDelegate::TreeViewPackagesItemDelegate(QObject *parent):
  QStyledItemDelegate(parent)
{
}

/*
 * Called every time user positions mouse over package's treeview items
 */
bool TreeViewPackagesItemDelegate::helpEvent ( QHelpEvent *event, QAbstractItemView*,
    const QStyleOptionViewItem&, const QModelIndex &index )
{
  if (this->parent()->objectName() == QLatin1String("tvPackages"))
  {
    QTreeView* tvPackages = qobject_cast<QTreeView*>(this->parent());
    PackageModel* sim = qobject_cast<PackageModel*>(tvPackages->model());
    if (sim == nullptr || sim->getPackageCount() == 0) return false;

    const PackageRepository::PackageData*const si = sim->getData(index);

    if (si != nullptr)
    {
      gPoint = tvPackages->mapToGlobal(event->pos());

      const PackageRepository::PackageData*const package = MainWindow::returnMainWindow()->getFirstPackageFromRepo(si->name);
      if (!package) return false;

      QFuture<QString> f;
      PkgDesc pkgDesc;
      pkgDesc.name = si->name;
      pkgDesc.description = package->description;
      pkgDesc.isForeign = (package->status == ectn_FOREIGN || package->status == ectn_FOREIGN_OUTDATED);

      disconnect(&g_fwToolTip, SIGNAL(finished()), this, SLOT(execToolTip()));
      f = QtConcurrent::run(showPackageDescriptionExt, pkgDesc);
      g_fwToolTip.setFuture(f);
      connect(&g_fwToolTip, SIGNAL(finished()), this, SLOT(execToolTip()));
    }
    else return false;
  }
  else if (this->parent()->objectName() == QLatin1String("tvTransaction"))
  {
    QTreeView* tvTransaction = qobject_cast<QTreeView*>(this->parent());
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tvTransaction->model());
    if (sim->rowCount() == 0) return false;

    QStandardItem *si = sim->itemFromIndex(index);

    if (si)
    {
      //If it's really a package in the Transaction treeview...
      QString pkgName=si->text();

      //We have to separate Repository from Package Name, first
      int slash = pkgName.indexOf(QLatin1String("/"));
      if (slash != -1)
      {
        pkgName = pkgName.mid(slash+1);
      }

      const PackageRepository::PackageData*const package = MainWindow::returnMainWindow()->getFirstPackageFromRepo(pkgName);
      if (!package) return false;

      QFuture<QString> f;
      PkgDesc pkgDesc;
      pkgDesc.name = pkgName;
      pkgDesc.description = package->description;
      pkgDesc.isForeign = (package->status == ectn_FOREIGN || package->status == ectn_FOREIGN_OUTDATED);

      if (si->icon().pixmap(22, 22).toImage() ==
          IconHelper::getIconInstallItem().pixmap(22, 22).toImage() ||
          si->icon().pixmap(22, 22).toImage() ==
          IconHelper::getIconRemoveItem().pixmap(22, 22).toImage())
      {
        gPoint = tvTransaction->mapToGlobal(event->pos());
        QFuture<QString> f;
        disconnect(&g_fwToolTip, SIGNAL(finished()), this, SLOT(execToolTip()));
        f = QtConcurrent::run(showPackageDescriptionExt, pkgDesc);
        g_fwToolTip.setFuture(f);
        connect(&g_fwToolTip, SIGNAL(finished()), this, SLOT(execToolTip()));
      }
      else
      {
        QToolTip::hideText();
      }
    }
  }

  return true;
}

/*
 * When the tooltip QFuture method is finished, we show the selected tooltip to the user
 */
void TreeViewPackagesItemDelegate::execToolTip()
{
  if (g_fwToolTip.result().trimmed().isEmpty())
    return;

  gPoint.setX(gPoint.x() + 25);
	gPoint.setY(gPoint.y() + 25);

  QToolTip::showText(gPoint, g_fwToolTip.result());
}
