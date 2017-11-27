/*
Copyright 2015 Michaël Lhomme

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

#ifndef CACHECLEANER_H
#define CACHECLEANER_H

#include <QMainWindow>

#include "packagegroupmodel.h"

namespace Ui {
class CacheCleaner;
}

/*
 * Main CacheCleaner window
 */
class CacheCleaner : public QMainWindow
{
    Q_OBJECT

private:
    Ui::CacheCleaner *ui;
    PackageGroupModel *m_installed;
    PackageGroupModel *m_uninstalled;

public:
    explicit CacheCleaner(QWidget *parent = 0);
    ~CacheCleaner();

protected:
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *);
};

#endif // CACHECLEANER_H
