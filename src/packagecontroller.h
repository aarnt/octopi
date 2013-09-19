/*
 *
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

#ifndef PACKAGECONTROLLER_H
#define PACKAGECONTROLLER_H

#include <QModelIndex>
#include <QStandardItemModel>
#include <QModelIndex>

class PackageController
{
public:
  static QString showFullPathOfItem( const QModelIndex &index );
  static QList<QModelIndex> * findFileEx( const QString& name, const QStandardItemModel *sim);

  static QString retrieveDistroNews(bool searchForLatestNews);
  static QString parseDistroNews();
};

#endif // PACKAGECONTROLLER_H
