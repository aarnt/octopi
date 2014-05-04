/*
Copyright
  2011 Simone Tobia
  2014 Alexandre Albuquerque Arnt (stripped unused parts of the code)

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

#include <QList>
#include <QStringList>
#include "../repoeditor/repoentry.h"

class RepoConf
{
private:
  QList<RepoEntry> entries;
  QStringList preamble;
  QString repoConfFilePath;

  static QString commentString;
  static QRegExp repoMatch;
  static QRegExp detailMatch;
  static bool matchRepo( QString line );
  static bool matchRepoDetails( QString line );

  RepoEntry extractRepo( QString line );
  bool isEmpty( QString line );
  bool loadConf( const QString &eFile );
  void addEntry( const RepoEntry & entry );

public:
  explicit RepoConf();
  QStringList getRepos();
};

#endif // REPOCONF_H
