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
#include "unixcommand.h"

class QSortFilterProxyModel;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;
class QTimer;
class QFileSystemWatcher;
class QLabel;

const int ctn_COLUMN_PACKAGE_ICON(0);
const int ctn_PACKAGE_NAME_COLUMN(1);
const int ctn_PACKAGE_VERSION_COLUMN(2);
const int ctn_PACKAGE_REPOSITORY_COLUMN(3);

const int ctn_TABINDEX_INFORMATION(0);
const int ctn_TABINDEX_FILES(1);
const int ctn_TABINDEX_TRANSACTION(2);
const int ctn_TABINDEX_OUTPUT(3);

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
  UnixCommand *m_unixCommand;

  QSortFilterProxyModel *m_proxyModelPackages;
  QFileSystemWatcher *m_pacmanDatabaseSystemWatcher;

  //This model provides the list of ALL packages (installed + non-installed)
  QStandardItemModel *m_modelPackages;

  //This model privides the list of ONLY installed packages
  QStandardItemModel *m_modelInstalledPackages;

  //This model provides the list of pending actions of a transaction
  QStandardItemModel *m_modelTransaction;

  //This member holds the target list retrieved by the pacman command which will be executed
  QStringList *m_targets;

  //Holds the current index of the target being processing
  int m_currentTarget;

  //This member holds the current command being executed by Octopi
  CommandExecuting m_commandExecuting;
  CommandExecuting m_commandQueued;

  int m_PackageListOrderedCol;
  Qt::SortOrder m_PackageListSortOrder;
  QTimer *timer;
  QStringList *m_outdatedPackageList;

  QLabel * m_lblCounters;

  int m_numberOfInstalledPackages;
  int m_numberOfAvailablePackages;
  int m_numberOfOutdatedPackages;

  void initAppIcon();
  void refreshAppIcon();
  void initToolBar();
  void initStatusBar();
  void initLineEditFilterPackages();

  QString getInstalledPackageVersionByName(const QString &pkgName);
  bool isPackageInstalled(const QString &pkgName);
  void initPackageTreeView();

  void initTabInfo();
  void initTabFiles();
  void initActions();
  void refreshStatusBar();

  //Tab Output related methods
  void initTabOutput();
  void clearTabOutput();

  void _positionTextEditCursorAtEnd();
  bool _textInTabOutput(const QString& findText);
  void _treatProcessOutput(const QString &pMsg);
  void writeToTabOutput(const QString &msg);

  //Tab Transaction related methods
  void initTabTransaction();
  QStandardItem * getRemoveTransactionParentItem();
  QStandardItem * getInstallTransactionParentItem();
  void insertRemovePackageIntoTransaction(const QString &pkgName);
  void insertInstallPackageIntoTransaction(const QString &pkgName);
  void removePackagesFromRemoveTransaction();
  void removePackagesFromInstallTransaction();

  QString getToBeRemovedPackages();
  QString getToBeInstalledPackages();

  QString showFullPathOfObject(const QModelIndex & index);

protected:
  void keyPressEvent(QKeyEvent* ke);

private slots:

  //TreeView methods
  void buildPackageList();
  void headerViewPackageListSortIndicatorClicked(int col, Qt::SortOrder order);
  void changePackageListModel();
  void execContextMenuPackages(QPoint point);

  //SearchLineEdit methods
  void reapplyPackageFilter();

  //TabWidget methods
  void refreshTabInfo(bool clearContents=false);
  void refreshTabFiles(bool clearContents=false);
  void changedTabIndex();
  void invalidateTabs(); //This method clears the current information showed on tab.

  void doSyncDatabase();
  void doSystemUpgrade(bool syncDatabase = false);
  void doRemove();
  void doInstall();

  void disableTransactionActions();
  void enableTransactionActions();
  void toggleTransactionActions(const bool value);
  void commitTransaction();
  void rollbackTransaction();

  void actionsProcessStarted();
  void actionsProcessFinished(int exitCode, QProcess::ExitStatus);
  void actionsProcessReadOutput();
  void actionsProcessRaisedError();

  void insertIntoRemovePackage();
  void insertIntoInstallPackage();

  void maximizeTabWidget();
  void outputOutdatedPackageList();

  void onPressDelete();
  void changeTransactionActionsState();
  void clearTransactionTreeView();

  void openFile(const QModelIndex& mi);

};

#endif // MAINWINDOW_H
