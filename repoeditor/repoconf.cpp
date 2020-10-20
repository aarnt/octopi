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

#include "repoconf.h"
#include "../src/unixcommand.h"

#include <QApplication>
#include <QStyle>
#include <QFont>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QTemporaryFile>

QString RepoConf::commentString = QLatin1String("");
QRegularExpression RepoConf::repoMatch = QRegularExpression();
QRegularExpression RepoConf::detailMatch = QRegularExpression();
QRegularExpression RepoConf::sigLevelMatch = QRegularExpression();

RepoConf::RepoConf()
{
  repoConfFilePath = QLatin1String("/etc/pacman.conf");
  repoMatch = QRegularExpression(QStringLiteral("^\\[(?!(options|repo-name|\\[|\\s))"));
  detailMatch = QRegularExpression(QStringLiteral("^(Server|Include)\\s*=\\s*.+"));
  sigLevelMatch = QRegularExpression(QStringLiteral("^(SigLevel)\\s*=\\s*.+"));
  RepoEntry::nameFilter = QRegularExpression(QStringLiteral("(\\s+|\\[|\\])"));
  commentString = QLatin1String("#");
  RepoEntry::repoFormat = QLatin1String("[%repo%]");

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

bool RepoConf::matchRepoSigLevel( QString line )
{
  return line.remove( commentString ).trimmed().contains( sigLevelMatch );
}

RepoEntry RepoConf::extractRepo( QString line )
{
  line = line.trimmed();
  bool active = line.mid( 0, commentString.length() ) != commentString;
  return RepoEntry( line.remove( commentString ), active );
}

bool RepoConf::exists( const QString &name )
{
  bool found=false;
  for( int i=0; !found && i < entries.count(); ++i )
    if( ( ( RepoEntry & )entries.at( i ) ).getName() == name)
      found = true;
  return found;
}

/*
 * Let's compare actual "/etc/pacman.conf" to the UI to see if user changed anything
 */
bool RepoConf::hasAnyChanges()
{
  bool res=false;
  QFile confFile(QStringLiteral("/etc/pacman.conf"));
  QTextStream confFileStream( &confFile );

  if (!confFile.open(QIODevice::ReadOnly))
  {
    res=false;
  }
  else
  {
    QString contents = QString::fromUtf8(confFile.readAll().trimmed());
    QByteArray file=contents.toLatin1();
    QByteArray repoeditor=toString().toLatin1();
    if (file != repoeditor)
    {
      int cFile=file.count('\n');
      int cRepoeditor=repoeditor.count('\n');
      if (file.size() != (repoeditor.size() - (cRepoeditor-cFile))) return true;
    }
  }

  return res;
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

  beginResetModel();
  entries.clear();

  QStringList comments;
  while( !confFileStream.atEnd() )
  {
    QString line = confFileStream.readLine();
    bool commented = !isEmpty( line ) &&
        line.trimmed().mid( 0, commentString.length() ) == commentString;

    if( !preamble && isEmpty( line ) )
    {
      actual.setDetailsComments( comments );
      comments.clear();
    }
    else if( matchRepo( line ) )
    {
      if( preamble )
        preamble = false;

      if( actual.isValid() )
        addEntry( actual );

      actual = extractRepo( line );
      actual.setComments( comments );
      comments.clear();
    }
    else if( !preamble && matchRepoDetails( line ) )
    {
      if( actual.isActive() && commented )
        actual.addDetailsComment( line );    
      else
        actual.addDetail( line.remove( commentString ) );
    }
    else if( !preamble && matchRepoSigLevel( line ) )
    {
      if( actual.isActive() && commented )
        actual.addSigLevelsComment( line );
      else
        actual.addSigLevel( line.remove( commentString ) );
    }

    else if( !preamble && commented )
      comments << line;

    else if( preamble )
    {
      if( isEmpty(line) || !commented )
      {
        this->preamble << comments << line;
        comments.clear();
      }
      else if( commented )
        comments << line;
    }
  }

  if( actual.isValid() )
    addEntry(actual);

  endResetModel();

  return true;
}

void RepoConf::addEntry( const RepoEntry & entry )
{
  beginInsertRows( QModelIndex(), entries.count(), entries.count() );
  entries.push_back( entry );
  endInsertRows();
}

void RepoConf::reload()
{
  loadConf( repoConfFilePath );
}

/*
 * Saves changes in pacman.conf using the available SU tool
 */
bool RepoConf::saveChanges( const QString & backup )
{
  QTemporaryFile tempFile;
  UnixCommand *unixC = new UnixCommand(this);
  QString command;

  if( !backup.isEmpty() ) {
    QMessageBox mbexists( QMessageBox::Warning,
                          tr( "Backup error" ),
                          tr( "Backup file already exists." ) + QStringLiteral( "\n" ) + tr( "Do you want to overwrite it?" ),
                          QMessageBox::Yes | QMessageBox::No );

    //First we test if backup file already exists. If so, we remove it!
    if( QFile::exists( backup ) && mbexists.exec() == QMessageBox::Yes ) {
      command = QLatin1String("rm ") + backup;
    }

    //Then we create a backup, with the user defined name
    if (!command.isEmpty()) command += QLatin1String("; ");
    command += QLatin1String("cp /etc/pacman.conf ") + backup;
  }

  if (!tempFile.open())
    return false;

  tempFile.write( toString().toLatin1() );
  tempFile.close();

  //Last, we copy the tempfile to the repoconf path
  if (!command.isEmpty()) command += QLatin1String("; ");

  command +=
      QLatin1String("cp ") + tempFile.fileName() +
      QLatin1String(" /etc/pacman.conf;") +
      QLatin1String(" chown root /etc/pacman.conf;") +
      QLatin1String(" chgrp root /etc/pacman.conf;") +
      QLatin1String("chmod 644 /etc/pacman.conf");

  unixC->execCommand(command);

  reload();
  return true;
}

bool RepoConf::removeRows( int row, int count, const QModelIndex & )
{
  beginRemoveRows( QModelIndex(), row, row + count - 1);

  for( int i = 0; i < count; ++i ) {
    entries.removeAt( row + i );
  }

  endRemoveRows();

  return true;
}

QString RepoConf::toString() const
{
  QStringList ret;

  for( int i = 0; i < entries.count(); ++i ) {
    ret << ( ( RepoEntry & )entries.at( i ) ).toString();
  }

  return preamble.join( QStringLiteral("\n") ).trimmed() + QStringLiteral( "\n\n" ) + ret.join( QStringLiteral("\n\n") );
}

bool RepoConf::detailsExists() const
{
  bool found = false;

  for( int i = 0; !found && i < entries.count(); ++i )
    found = !( ( const RepoEntry & )entries.at( i ) ).getDetails().isEmpty();

  return found;
}

QVariant RepoConf::headerData( int section, Qt::Orientation orientation, int role ) const
{
  QStringList headers;

  headers << tr( "Active" ) << tr( "Repository" ) << tr( "Options" );

  switch( role ) {
  case Qt::DisplayRole:
    if( orientation == Qt::Horizontal ) {
      return headers.at( section );
    } else {
      return section + 1;
    }
    break;
  }

  return QVariant();
}

QVariant RepoConf::data( const QModelIndex &index, int role ) const
{
  switch( role ){
  case Qt::DisplayRole :
  case Qt::EditRole :
    switch( index.column() ) {
    case 0 :
      return ( ( RepoEntry & )entries.at( index.row() ) ).isActive();
    case 1 :
      return ( ( RepoEntry & )entries.at( index.row() ) ).getName();
    case 2 :
      return ( ( RepoEntry & )entries.at( index.row() ) ).getDetails().join( QStringLiteral("\n") );
    }

    break;
  case Qt::FontRole :
    switch( index.column() ) {
    case 1 :
      QFont font( qApp->font() );
      font.setBold( true );
      return font;
    }
    break;
  case Qt::ToolTipRole :
    switch( index.column() ) {
    case 2 :
      return ( ( ( ( RepoEntry & )entries.at( index.row() ) ).getDetailsComments().isEmpty() )
               ? ( ( ( RepoEntry & )entries.at( index.row() ) ).getComments().join( QStringLiteral("\n") ) )
               : ( ( ( RepoEntry & )entries.at( index.row() ) ).getDetailsComments().join( QStringLiteral("\n") ) ) );
    default :
      return ( ( RepoEntry & )entries.at( index.row() ) ).getComments().join( QStringLiteral("\n") );
    }
    break;
  }

  return QVariant();
}

bool RepoConf::setData( const QModelIndex &index, const QVariant &value, int role )
{
  switch( role ) {
  case Qt::DisplayRole :
  case Qt::EditRole :
    switch( index.column() ) {
    case 0 :
      ( ( RepoEntry & )entries.at( index.row() ) ).setActive( value.toBool() );
      break;
    case 1 :
      if( !matchRepo( RepoEntry::formatRepoName( value.toString() ) ) )
        return false;
      ( ( RepoEntry & )entries.at( index.row() ) ).setName( value.toString() );
      break;
    case 2:
      QStringList list = value.toString().trimmed().split( QStringLiteral("\n") );
      for( int i = 0; i < list.count(); ++i) {
        list[i] = list.at( i ).trimmed();
        if( !matchRepoDetails( list.at(i) ) )
          return false;
      }
      ( ( RepoEntry & )entries.at( index.row() ) ).setDetails( list );
      break;
    }

    emit dataChanged( index, index );
    return true;
  }

  return false;
}
