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
#ifndef REPOEDITOR_H
#define REPOEDITOR_H

#include <QDialog>
#include <QItemSelection>

#include "repoconf.h"
#include "addrepo.h"

namespace Ui {
    class RepoEditor;
}

class RepoEditor : public QDialog
{
    Q_OBJECT

    RepoConf *repoConf;

    AddRepo *addRepoDialog;

public:
    explicit RepoEditor( QWidget *parent = 0 );
    ~RepoEditor();

    QString toString() const {
        return repoConf->toString();
    }

private:
    Ui::RepoEditor *ui;

public slots:
    void moveUp();
    void moveDown();

    void updateMovers( const QItemSelection &, const QItemSelection & );

    void apply();
    void discard();

    void removeEntry();
    void addEntry();
    void editEntry();

    void loadBackup();
};

#endif // REPOEDITOR_H
