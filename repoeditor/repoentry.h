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
#ifndef REPOENTRY_H
#define REPOENTRY_H

#include <QString>
#include <QStringList>
#include <QRegExp>

class RepoEntry{

private:
    QString name; //Contains the formatted name (e.g. [main])
    QStringList details;
    QStringList comments;
    QStringList sigLevels;
    QStringList detailsComments;
    QStringList sigLevelsComments;

    bool valid;

    bool active;

public:
    explicit RepoEntry();
    explicit RepoEntry( QString name, bool active = false );

    static QRegExp nameFilter; //Contains the filter to obtain the unformatted repo name (e.g. [main] -> main)
    static QString commentString;
    static QString repoFormat;

    static QString formatRepoName( const QString & name ) {
        QString aux = repoFormat;
        return aux.replace("%repo%",name);
    }

    QString getName() const; //returns the unformatted repo name
    const QStringList & getDetails() const {
        return details;
    }

    void addDetail( const QString & detail );
    void setDetails( const QStringList & details );

    void addSigLevel( const QString & sigLevel );
    void setSigLevels( const QStringList &sigLevels );

    void addComment( const QString & comment );

    void setComments( const QStringList & comments ) {
        this->comments = comments;
    }

    void addComments( const QStringList & comments ) {
        this->comments << comments;
    }

    const QStringList & getComments() const {
        return comments;
    }

    void addDetailsComment( const QString & comment ) {
        detailsComments << comment;
    }

    const QStringList & getDetailsComments() const {
        return detailsComments;
    }

    void setDetailsComments( const QStringList & comments ) {
        detailsComments = comments;
    }

    void addSigLevelsComment( const QString & comment ) {
        sigLevelsComments << comment;
    }

    const QStringList & getSigLevelsComments() const {
        return sigLevelsComments;
    }

    void setSigLevelsComments( const QStringList & comments ) {
        sigLevelsComments = comments;
    }

    const QString & getRealName(){
        return name;
    }

    void setRealName( const QString & realName );
    void setName( const QString & name );

    bool isValid() const {
        return valid;
    }

    bool isActive() const {
        return active;
    }

    bool setActive( bool active );

    QString toString() const;
};

#endif // REPOENTRY_H
