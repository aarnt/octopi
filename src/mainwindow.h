/*
* This file is part of Octopi, an open-source GUI for ArchLinux pacman.
* Copyright (C) 2013  Alexandre Albuquerque Arnt
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QSortFilterProxyModel;
class QStandardItemModel;

const int ctn_PACKAGE_ICON(0);
const int ctn_PACKAGE_NAME(1);
const int ctn_PACKAGE_VERSION(2);
const int ctn_PACKAGE_REPOSITORY(3);

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  QSortFilterProxyModel *m_proxyModelPackages;
  QStandardItemModel *m_modelPackages;
  int m_PackageListOrderedCol;
  Qt::SortOrder m_PackageListSortOrder;

  void initLineEditFilterPackages();
  void initPackageTreeView();
  void initTabInfo();
  void initTabFiles();
  void initActions();

private slots:
  void refreshPackageList();
  void reapplyPackageFilter();
  void headerViewPackageListSortIndicatorClicked(int col, Qt::SortOrder order);

  void refreshTabInfo();
  void refreshTabFiles();
  void changedTabIndex();

};

#endif // MAINWINDOW_H
