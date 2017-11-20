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
#include "repoentry.h"

QRegularExpression RepoEntry::nameFilter = QRegularExpression( "" );
QString RepoEntry::commentString = "";
QString RepoEntry::repoFormat = "";

RepoEntry::RepoEntry()
    : valid( false )
    , active( false )
{
}

RepoEntry::RepoEntry( QString name, bool active )
    : name( name )
    , valid( true )
    , active( active )
{
}

QString RepoEntry::getName() const
{
    if( !nameFilter.isValid() || !isValid() )
        return "";

    QString ret = name;

    return ret.remove( nameFilter );
}

void RepoEntry::addDetail( const QString & detail )
{
    details << detail;
}

void RepoEntry::setDetails( const QStringList &details )
{
  this->details = details;
}

void RepoEntry::addSigLevel(const QString &sigLevel)
{
    sigLevels << sigLevel;
}

void RepoEntry::setSigLevels(const QStringList &sigLevels)
{
    this->sigLevels = sigLevels;
}

void RepoEntry::addComment( const QString &comment )
{
    this->comments << comment;
}

void RepoEntry::setName( const QString &name )
{
    this->name = formatRepoName( name );
    valid = true;
}

QString RepoEntry::toString() const
{
    QStringList list,list2,list3,retList;

    if( !isValid() )
        return "";

    if( comments.isEmpty() )
        list << ( active ? name : commentString + name );
    else
        list << comments.join( "\n" ) << name;

    if( sigLevelsComments.isEmpty() ) {
        if( !active && !sigLevels.isEmpty() ) {
            QStringList sigLevelsCopy = sigLevels;
            sigLevelsCopy[0] = commentString + sigLevels.at( 0 );
            list3 << sigLevelsCopy;
        } else
            list3 << sigLevels;
    } else
        list3 << sigLevelsComments.join( "\n" ) << details;

    if( detailsComments.isEmpty() ) {
        if( !active && !details.isEmpty() ) {
            QStringList detailsCopy = details;
            detailsCopy[0] = commentString + details.at( 0 );
            list2 << detailsCopy;
        } else
            list2 << details;
    } else
        list2 << detailsComments.join( "\n" ) << details;

    retList <<  list.join( QString( "\n" ) + ( active ? "" : commentString ) );
    retList << list3.join( QString( "\n" ) + ( active ? "" : commentString ) );
    retList << list2.join( QString( "\n" ) + ( active ? "" : commentString ) );

    return retList.join( "\n" );
}

void RepoEntry::setRealName( const QString &realName )
{
    name = realName;
    valid = true;
}

bool RepoEntry::setActive( bool active )
{
    if( !valid )
        return false;

    this->active = active;

    return true;
}
