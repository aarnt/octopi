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

#include "repoconf.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

/*
 * Gets all the repos available in system's Pacman configuration
 */

QString RepoConf::commentString = "";
QRegularExpression RepoConf::repoMatch     = QRegularExpression();
QRegularExpression RepoConf::detailMatch   = QRegularExpression();

RepoConf::RepoConf()
{
  repoConfFilePath = "/etc/pacman.conf";
  repoMatch = QRegularExpression("^\\[(?!(options|repo-name|\\[|\\s))");
  detailMatch = QRegularExpression("^(Server|Include)\\s*=\\s*.+");
  RepoEntry::nameFilter = QRegularExpression("(\\s+|\\[|\\])");
  commentString = "#";
  RepoEntry::repoFormat = "[%repo%]";

  loadConf( repoConfFilePath );
}

bool RepoConf::isEmpty(QString line)
{
  return line.remove( commentString ).trimmed().isEmpty();
}

bool RepoConf::matchRepo( QString line ){
  return line.remove( commentString ).trimmed().contains( repoMatch );
}

bool RepoConf::matchRepoDetails( QString line )
{
  return line.remove( commentString ).trimmed().contains( detailMatch );
}

RepoEntry RepoConf::extractRepo( QString line )
{
  line = line.trimmed();
  bool active = line.mid( 0, commentString.length() ) != commentString;
  return RepoEntry( line.remove( commentString ), active );
}

bool RepoConf::loadConf( const QString &eFile )
{
  QFile confFile( eFile );
  QTextStream confFileStream( &confFile );
  RepoEntry actual;
  bool preamble = true;

  if( !confFile.exists() || !confFile.open( QIODevice::ReadOnly ) ) {
    return false;
  }

  RepoEntry::commentString = RepoConf::commentString;

  entries.clear();

  QStringList comments;
  while( !confFileStream.atEnd() ) {
    QString line = confFileStream.readLine();
    bool commented = !isEmpty( line ) &&
        line.trimmed().mid( 0, commentString.length() ) == commentString;
    if( !preamble && isEmpty( line ) ) {
      actual.setDetailsComments( comments );
      comments.clear();
    } else if( matchRepo( line ) ) {
      if( preamble )
        preamble = false;
      if( actual.isValid() )
        addEntry( actual );
      actual = extractRepo( line );
      actual.setComments( comments );
      comments.clear();
    } else if( !preamble && matchRepoDetails( line ) ) {
      if( actual.isActive() && commented )
        actual.addDetailsComment( line );
      else actual.addDetail( line.remove( commentString ) );
    } else if( !preamble && commented )
      comments << line;
    else if( preamble ) {
      if( isEmpty(line) || !commented ) {
        this->preamble << comments << line;
        comments.clear();
      } else if( commented )
        comments << line;
    }
  }

  if( actual.isValid() )
    addEntry(actual);

  return true;
}

void RepoConf::addEntry( const RepoEntry & entry )
{
  entries.push_back( entry );
}

QStringList RepoConf::getRepos(){
  QStringList res;

  for (int c=0; c<entries.count(); c++){
    if (entries.at(c).isActive())
      res.append(entries.at(c).getName());
  }

  return res;
}
