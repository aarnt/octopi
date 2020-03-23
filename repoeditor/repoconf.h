/*
Copyright 2011 Simone Tobia

This file is part of AppSet.

AppSet is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

AppSet is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with AppSet; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef REPOCONF_H
#define REPOCONF_H

#include <QAbstractTableModel>
#include <QList>
#include <QStringList>
#include "repoentry.h"

class RepoConf : public QAbstractTableModel
{
  Q_OBJECT

private:
  QList<RepoEntry> entries;
  QStringList preamble;

  QString repoConfFilePath;

  bool isEmpty( QString line );

  RepoEntry extractRepo( QString line );

public:
  explicit RepoConf();

  static QString commentString;
  static QRegularExpression repoMatch;
  static QRegularExpression detailMatch;
  static QRegularExpression sigLevelMatch;

  static bool matchRepo( QString line );
  static bool matchRepoDetails( QString line );
  static bool matchRepoSigLevel( QString line );

  QString toString() const;

  RepoEntry at( int i ) const {
    return ( ( i <= entries.count() ) ? entries.at(i) : RepoEntry() );
  }

  int count() const {
    return entries.count();
  }

  bool detailsExists() const;

  void addEntry( const RepoEntry & entry );

  bool exists( const QString & name );

  const QString & getConfPath() const {
    return repoConfFilePath;
  }

  bool hasAnyChanges();

  //Table Model
  int rowCount( const QModelIndex & ) const {
    return count();
  }

  int columnCount( const QModelIndex & ) const {
    return detailsExists()?3:2;
  }

  QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
  QVariant headerData( int section, Qt::Orientation orientation, int role ) const;

  Qt::ItemFlags flags( const QModelIndex & ) const {
    return Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable;
  }

  bool setData( const QModelIndex &index, const QVariant &value, int role );

public slots:    
  bool loadConf( const QString &eFile );

  void reload(); //Reloads entries from configured file
  bool saveChanges( const QString & backup = QLatin1String("") );

  void moveUp( int i ) {
    if( entries.count() && i > 0 ) {
      entries.swapItemsAt( i, i - 1 );
      emit dataChanged( QModelIndex(), QModelIndex() );
    }
  }

  void moveDown( int i ){
    if( entries.count() && i < entries.count() - 1 ) {
      entries.swapItemsAt( i, i + 1 );
      emit dataChanged( QModelIndex(), QModelIndex() );
    }
  }

  bool removeRows(int row, int count, const QModelIndex &);
};

#endif // REPOCONF_H
