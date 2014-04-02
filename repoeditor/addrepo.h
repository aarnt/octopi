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
#ifndef ADDREPO_H
#define ADDREPO_H

#include <QDialog>
#include "repoconf.h"
#include "repoentry.h"

namespace Ui {
    class AddRepo;
}

class AddRepo : public QDialog
{
    Q_OBJECT

public:
    explicit AddRepo( QWidget *parent = 0 );
    ~AddRepo();

    RepoEntry entry;

    void setRepoName( const QString & );
    void setRepoLocation( const QString & );
    void setLocationType( int );

    QString getRepoName() const;
    QString getRepoLocation() const;
    int getLocationType() const;

private:
    Ui::AddRepo *ui;

    QString examples;

    void showError( QString error );

private slots:
    void checkAndApply();
    void locationChanged( int );
    void selectLocalRepository();
    void selectServerList();
};

#endif // ADDREPO_H
