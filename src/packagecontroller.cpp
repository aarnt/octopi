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

#include <iostream>
#include "packagecontroller.h"
#include "package.h"

#include <QDirIterator>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QCoreApplication>

/*
 * This is a controller class that provides search services, using Package methods.
 */

/*
 * Returns the full path of a tree view item (normaly a file in a directory tree)
 */
QString PackageController::showFullPathOfItem( const QModelIndex &index ){
  QString str;
  if (!index.isValid()) return str;

  const QStandardItemModel *sim = qobject_cast<const QStandardItemModel*>( index.model() );
  QStringList sl;
  QModelIndex nindex;
  sl << sim->itemFromIndex( index )->text();

  nindex = index;
  while (1){
    nindex = sim->parent( nindex );
    if ( nindex != sim->invisibleRootItem()->index() ) sl << sim->itemFromIndex( nindex )->text();
    else break;
  }
  str = QDir::separator() + str;

  for ( int i=sl.count()-1; i>=0; i-- ){
    if ( i < sl.count()-1 ) str += QDir::separator();
    str += sl[i];
  }

  return str;
}

/*
 * Given a filename 'name', searches for it inside a QStandard item model
 * Result is a list containing all QModelIndex occurencies
 */
QList<QModelIndex> * PackageController::findFileEx( const QString& name, const QStandardItemModel *sim)
{
  QList<QModelIndex> * res = new QList<QModelIndex>();
  QList<QStandardItem *> foundItems;

  if (name.isEmpty() || sim->rowCount() == 0)
  {
    return res;
  }

  foundItems = sim->findItems(Package::parseSearchString(name), Qt::MatchRegExp|Qt::MatchRecursive);
  foreach(QStandardItem *item, foundItems)
  {
    if (item->accessibleDescription().contains("directory")) continue;

    res->append(item->index());
  }

  return res;
}
