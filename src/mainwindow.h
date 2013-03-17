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

#include <QApplication>
#include <QMainWindow>
#include <QUrl>
#include "unixcommand.h"

class QSortFilterProxyModel;
class QStandardItemModel;
class QStandardItem;
class QModelIndex;
class QTimer;
class QFileSystemWatcher;
class QLabel;
class QComboBox;

//Column indices for Package's treeview
const int ctn_PACKAGE_ICON_COLUMN(0);
const int ctn_PACKAGE_NAME_COLUMN(1);
const int ctn_PACKAGE_VERSION_COLUMN(2);
const int ctn_PACKAGE_REPOSITORY_COLUMN(3);

//Tab indices for Properties' tabview
const int ctn_TABINDEX_INFORMATION(0);
const int ctn_TABINDEX_FILES(1);
const int ctn_TABINDEX_TRANSACTION(2);
const int ctn_TABINDEX_OUTPUT(3);
const int ctn_TABINDEX_NEWS(4);
const int ctn_TABINDEX_HELPABOUT(5);

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

  static MainWindow* returnMainWindow()
  {
    static MainWindow *w=0;
    if (w != 0) return w;
    foreach (QWidget *widget, QApplication::topLevelWidgets())
    {
      if (widget->objectName() == "MainWindow")
        w = (MainWindow*) widget;
    }
    return w;
  }

  QStandardItemModel *getModelPackages(){ return m_modelPackages; }

private:
  Ui::MainWindow *ui;

  UnixCommand *m_unixCommand;

  bool m_initializationCompleted;

  QSortFilterProxyModel *m_proxyModelPackages;
  QFileSystemWatcher *m_pacmanDatabaseSystemWatcher;

  //This model provides the list of ALL packages (installed + non-installed)
  QStandardItemModel *m_modelPackages;

  //This model provides the list of ONLY installed packages
  QStandardItemModel *m_modelInstalledPackages;

  //This model provides the list of ALL packages from the selected group
  QStandardItemModel *m_modelPackagesFromGroup;

  //This model provides the list of ONLY installed packages from the selected group
  QStandardItemModel *m_modelInstalledPackagesFromGroup;

  //This model provides the list of pending actions of a transaction
  QStandardItemModel *m_modelTransaction;

  //This member holds the target list retrieved by the pacman command which will be executed
  QStringList *m_targets;

  //This member holds the current command being executed by Octopi
  CommandExecuting m_commandExecuting;
  CommandExecuting m_commandQueued;

  QLabel *m_lblGroups;
  QComboBox *m_cbGroups;

  int m_PackageListOrderedCol;
  Qt::SortOrder m_PackageListSortOrder;
  QTimer *timer;
  QStringList *m_outdatedPackageList;
  QLabel *m_lblCounters;    
  QByteArray m_horizontalSplit;

  int m_numberOfInstalledPackages;
  int m_numberOfAvailablePackages;
  int m_numberOfOutdatedPackages;

  void loadSettings();
  void loadPanelSettings();
  void saveSettings(int);

  void initAppIcon();
  void refreshAppIcon();
  void initComboBoxGroups();
  void refreshComboBoxGroups();
  void initToolBar();
  void initStatusBar();
  void initLineEditFilterPackages();

  QString getOutdatedPackageVersionByName(const QString &pkgName);
  QString getInstalledPackageVersionByName(const QString &pkgName);

  QStandardItem *getAvailablePackage(const QString &pkgName, const int index);

  bool isPackageInstalled(const QString &pkgName);
  void initPackageTreeView();

  void _changeTabWidgetPropertiesIndex(const int newIndex);
  void initTabWidgetPropertiesIndex();
  void initTabInfo();

  //Tab Files related methods
  QString getSelectedDirectory();
  QString showFullPathOfObject(const QModelIndex & index);

  void initTabFiles();
  void initActions();
  void refreshStatusBar();

  //Tab Transaction related methods
  void initTabTransaction();
  QStandardItem * getRemoveTransactionParentItem();
  QStandardItem * getInstallTransactionParentItem();
  void insertRemovePackageIntoTransaction(const QString &pkgName);
  void insertInstallPackageIntoTransaction(const QString &pkgName);
  void removePackagesFromRemoveTransaction();
  void removePackagesFromInstallTransaction();

  int getNumberOfTobeRemovedPackages();
  QString getTobeRemovedPackages();
  QString getTobeInstalledPackages();

  //Tab Output related methods
  bool _isThereAPendingTransaction();
  void _positionTextEditCursorAtEnd();
  bool _textInTabOutput(const QString& findText);
  bool _searchForKeyVerbs(const QString& msg);
  void _treatProcessOutput(const QString &pMsg);
  void _ensureTabVisible(const int index);
  bool _isPropertiesTabWidgetVisible();
  void writeToTabOutput(const QString &msg);
  void initTabOutput();
  void clearTabOutput();

  QString retrieveArchNews(bool searchForLatestNews = true);
  QString parseArchNews();
  void initTabNews();

  void initTabHelpAbout();

public slots:
  void show();

protected:
  void closeEvent(QCloseEvent *event);
  void keyPressEvent(QKeyEvent* ke);

private slots:

  //TreeView methods
  void buildPackagesFromGroupList(const QString &groupName);
  void buildPackageList();
  void headerViewPackageListSortIndicatorClicked(int col, Qt::SortOrder order);
  void changePackageListModel();
  void execContextMenuPackages(QPoint point);

  //SearchLineEdit methods
  void reapplyPackageFilter();

  //TabWidget methods
  void refreshTabInfo(bool clearContents=false);
  void refreshTabFiles(bool clearContents=false, bool neverQuit=false);
  void onDoubleClickPackageList();
  void changedTabIndex();
  void invalidateTabs(); //This method clears the current information showed on tab.

  void doSyncDatabase();
  void doSystemUpgrade(bool syncDatabase = false);
  void doRemove();
  void doInstall();
  void doCleanCache();

  void disableTransactionActions();
  void enableTransactionActions();
  void toggleTransactionActions(const bool value);
  void doCommitTransaction();
  void doRollbackTransaction();

  void actionsProcessStarted();
  void actionsProcessFinished(int exitCode, QProcess::ExitStatus);
  void actionsProcessReadOutput();
  void actionsProcessRaisedError();

  void insertIntoRemovePackage();
  void insertIntoInstallPackage();
  void insertGroupIntoRemovePackage();
  void insertGroupIntoInstallPackage();

  void maximizePackagesTreeView(bool pSaveSettings = true);
  void maximizePropertiesTabWidget(bool pSaveSettings = true);
  void outputOutdatedPackageList();

  void onTabNewsSourceChanged(QUrl newSource);

  void onHelpAbout();
  void onPressDelete();
  void changeTransactionActionsState();
  void clearTransactionTreeView();

  void openFile(const QModelIndex& mi);

  void refreshArchNews(bool searchForLatestNews = true);
};

#endif // MAINWINDOW_H
