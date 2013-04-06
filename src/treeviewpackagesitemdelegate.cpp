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
#include "package.h"
#include "mainwindow.h"
#include "strconstants.h"
#include "uihelper.h"
#include <QtGui>
#include <QFutureWatcher>
#include <QTreeWidget>
#include <iostream>

QPoint gPoint;
QFutureWatcher<QString> fw;
using namespace QtConcurrent;

QString showPackageInfo(QString pkgName){
  QString description =
      MainWindow::returnMainWindow()->getAvailablePackage(pkgName, ctn_PACKAGE_DESCRIPTION_COLUMN)->text();

  int space = description.indexOf(" ");
  return description.mid(space+1);
}

TreeViewPackagesItemDelegate::TreeViewPackagesItemDelegate(QObject *parent): QStyledItemDelegate(parent){  
}

bool TreeViewPackagesItemDelegate::helpEvent ( QHelpEvent *event, QAbstractItemView*,
		const QStyleOptionViewItem&, const QModelIndex &index ){

  if (this->parent()->objectName() == "tvPackages")
  {
    QTreeView* tvPackages = qobject_cast<QTreeView*>(this->parent());
    QSortFilterProxyModel *sfp = qobject_cast<QSortFilterProxyModel*>(tvPackages->model());
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(sfp->sourceModel());

    if (sim->rowCount() == 0) return false;

    QModelIndex ind = sfp->mapToSource(index);
    QStandardItem *si = sim->itemFromIndex(ind);

    if (si)
    {
      //If the user's mouse is not positioned above the name column, let's give him a little help...
      if (si->column() != ctn_PACKAGE_NAME_COLUMN)
      {
        QModelIndex miName = sim->index(si->row(), ctn_PACKAGE_NAME_COLUMN);
        si = sim->itemFromIndex(miName);
      }

      QPoint p;
      gPoint = tvPackages->mapToGlobal(event->pos());
      QFuture<QString> f;

      f = run(showPackageInfo, si->text());
      fw.setFuture(f);
      connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
    }
    else return false;
  }
  else if (this->parent()->objectName() == "tvTransaction")
  {
    QTreeView* tvTransaction = qobject_cast<QTreeView*>(this->parent());
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tvTransaction->model());

    if (sim->rowCount() == 0) return false;

    QStandardItem *si = sim->itemFromIndex(index);

    if (si)
    {
      if (si->icon().isNull()) //If it's really a package in the Transaction treeview...
      {
        QStandardItemModel *modelPackages = MainWindow::returnMainWindow()->getModelPackages();
        QList<QStandardItem*> foundItems = modelPackages->findItems(si->text(), Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);

        if (foundItems.count() > 0)
        {
          QStandardItem *siFound = foundItems.at(0);

          QPoint p;
          gPoint = tvTransaction->mapToGlobal(event->pos());
          QFuture<QString> f;

          f = run(showPackageInfo, siFound->text());
          fw.setFuture(f);
          connect(&fw, SIGNAL(finished()), this, SLOT(execToolTip()));
        }
      }
      else
      {
        QToolTip::hideText();
      }
    }
  }

  return true;
}

void TreeViewPackagesItemDelegate::execToolTip(){
  if (fw.result().isEmpty()) return;

  gPoint.setX(gPoint.x() + 25);
	gPoint.setY(gPoint.y() + 25);

  QToolTip::showText(gPoint, fw.result());
}
