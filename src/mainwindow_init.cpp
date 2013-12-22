/*
* This file is part of Octopi, an open-source GUI for pacman.
* Copyright (C) 2013 Alexandre Albuquerque Arnt
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

/*
 * This is MainWindow's initialization code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "strconstants.h"
#include "uihelper.h"
#include "settingsmanager.h"
#include "searchlineedit.h"
#include "treeviewpackagesitemdelegate.h"
#include "searchbar.h"
#include "packagecontroller.h"

#include <QLabel>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QTextBrowser>
#include <QResource>
#include <QFile>
#include <QComboBox>
#include <QListView>
#include <QTabBar>
#include <QProgressBar>
#include <QSystemTrayIcon>
#include <QToolButton>
#include <iostream>

/*
 * Loads various application settings configured in ~/.config/octopi/octopi.conf
 */
void MainWindow::loadSettings(){
  m_PackageListOrderedCol = SettingsManager::instance()->getPackageListOrderedCol();
  m_PackageListSortOrder = (Qt::SortOrder) SettingsManager::instance()->getPackageListSortOrder();

  ui->tvPackages->header()->setSortIndicator( m_PackageListOrderedCol, m_PackageListSortOrder );
  ui->tvPackages->sortByColumn( m_PackageListOrderedCol, m_PackageListSortOrder );
}

/*
 * This method only retrieves the App saved panels settings
 */
void MainWindow::loadPanelSettings(){
  int panelOrganizing = SettingsManager::instance()->getPanelOrganizing();
  switch(panelOrganizing){
    case ectn_MAXIMIZE_PACKAGES:
      maximizePackagesTreeView(false);
      break;
    case ectn_MAXIMIZE_PROPERTIES:
      maximizePropertiesTabWidget(false);
      break;
    case ectn_NORMAL:
      ui->splitterHorizontal->restoreState(SettingsManager::instance()->getSplitterHorizontalState());
      break;
  }
}

/*
 * Saves all application settings to ~/.config/octopi/octopi.conf
 */
void MainWindow::saveSettings(int saveSettingsReason){
  switch(saveSettingsReason){
    case ectn_CurrentTabIndex:
      SettingsManager::instance()->setCurrentTabIndex(ui->twProperties->currentIndex());
      break;

    case ectn_MAXIMIZE_PACKAGES:
      SettingsManager::instance()->setPanelOrganizing(ectn_MAXIMIZE_PACKAGES);
      break;

    case ectn_MAXIMIZE_PROPERTIES:
      SettingsManager::instance()->setPanelOrganizing(ectn_MAXIMIZE_PROPERTIES);
      break;

    case ectn_NORMAL:
      SettingsManager::instance()->setPanelOrganizing(ectn_NORMAL);
      SettingsManager::instance()->setSplitterHorizontalState(ui->splitterHorizontal->saveState());
      break;

    case ectn_PackageList:
      SettingsManager::instance()->setPackageListOrderedCol(m_PackageListOrderedCol);
      SettingsManager::instance()->setPackageListSortOrder(m_PackageListSortOrder);
      break;

    case ectn_IconifyOnStart:
      //SettingsManager::instance()->setStartIconified(m_actionIconifyOnStart->isChecked());
      break;
  }
}

/*
 * If we have some outdated packages, let's put octopi in a red face/angry state ;-)
 */
void MainWindow::initAppIcon()
{
  m_outdatedPackageList = Package::getOutdatedPackageList();
  m_outdatedYaourtPackageList = Package::getOutdatedYaourtPackageList();
  m_numberOfOutdatedPackages = m_outdatedPackageList->count();
  refreshAppIcon();
}

/*
 * Whenever user clicks the SystemTrayIcon area...
 */
void MainWindow::execSystemTrayActivated(QSystemTrayIcon::ActivationReason ar)
{
  if (!m_initializationCompleted) return;

  switch (ar)
  {
  case QSystemTrayIcon::Trigger:
  case QSystemTrayIcon::DoubleClick:
    if ( this->isHidden() ){
      //this->restoreGeometry( m_savedGeometry );
      if (this->isMinimized()) this->setWindowState(Qt::WindowNoState);
      this->show();
    }
    else {
      //m_savedGeometry = this->saveGeometry();
      this->hide();
    }
    break;
  default: break;
  }
}

/*
 * Inits the Groups combobox, so it can be added in app's toolBar
 */
void MainWindow::initComboBoxGroups()
{
  //This is the twGroups init code
  ui->twGroups->setColumnCount(1);
  ui->twGroups->setHeaderLabel(StrConstants::getGroups());
  ui->twGroups->header()->setSortIndicatorShown(false);
  ui->twGroups->header()->setClickable(false);
  ui->twGroups->header()->setMovable(false);
  ui->twGroups->setFrameShape(QFrame::NoFrame);
  ui->twGroups->setFrameShadow(QFrame::Plain);
  ui->twGroups->setStyleSheet(StrConstants::getTreeViewCSS());
  ui->twGroups->setSelectionMode(QAbstractItemView::SingleSelection);

  connect(ui->twGroups, SIGNAL(itemSelectionChanged()), this, SLOT(onComboGroupsChanged()));
  //connect(ui->twGroups, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(metaBuildPackageList()));
}

/*
 * Whenever user changes the Groups combobox, this method will be triggered...
 */
void MainWindow::onComboGroupsChanged()
{
  if (isAllGroupsSelected())
  {
    ui->actionSearchByName->setChecked(true);
    tvPackagesSearchColumnChanged(ui->actionSearchByName);
  }
}

/*
 * Inits the toolbar, including taking that annoying default view action out of the game
 */
void MainWindow::initToolBar()
{
  initComboBoxGroups();

  ui->mainToolBar->addAction(ui->actionSyncPackages);
  ui->mainToolBar->addAction(ui->actionCommit);
  ui->mainToolBar->addAction(ui->actionRollback);
  m_leFilterPackage->setMinimumHeight(24);
  ui->mainToolBar->addWidget(m_leFilterPackage);
  ui->mainToolBar->addSeparator();
  ui->mainToolBar->addAction(ui->actionShowGroups);

  QWidget * hSpacer = new QWidget(this);
  hSpacer->setMinimumHeight(22);
  hSpacer->setMinimumWidth(6);
  hSpacer->setVisible(true);
  ui->mainToolBar->addWidget(hSpacer);
  ui->mainToolBar->toggleViewAction()->setEnabled(false);
  ui->mainToolBar->toggleViewAction()->setVisible(false);
}

/*
 * The only thing needed here is to create a dynamic label which will contain the package counters
 */
void MainWindow::initStatusBar()
{
  m_lblSelCounter = new QLabel(this);
  m_lblTotalCounters = new QLabel(this);
  m_progressWidget = new QProgressBar(this);

  m_progressWidget->close();
  m_progressWidget->setMaximumWidth(250);

  ui->statusBar->addWidget(m_lblSelCounter);
  ui->statusBar->addWidget(m_lblTotalCounters);
  ui->statusBar->addPermanentWidget(m_progressWidget);

  connect(m_lblTotalCounters, SIGNAL(linkActivated(QString)), this, SLOT(outputOutdatedPackageList()));
}

/*
 * Inits the outdated toolbutton, which warns the user about outdated packages
 */
void MainWindow::initToolButtonPacman()
{
  m_toolButtonPacman = new QToolButton(this);
  m_toolButtonPacman->setIconSize(QSize(16, 16));
  m_toolButtonPacman->setIcon(IconHelper::getIconOutdated());
  m_toolButtonPacman->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  m_toolButtonPacman->setAutoRaise(true);
  m_toolButtonPacman->hide();
  m_menuToolButtonPacman = new QMenu(this);
  m_menuToolButtonPacman->addAction(m_actionInstallPacmanUpdates);
  m_toolButtonPacman->setPopupMode(QToolButton::MenuButtonPopup);
  m_toolButtonPacman->setMenu(m_menuToolButtonPacman);

  connect(m_toolButtonPacman, SIGNAL(clicked()), this, SLOT(outputOutdatedPackageList()));
}

/*
 * Inits the Yaourt toolbutton, which warns the user about outdated foreign packages
 */
void MainWindow::initToolButtonYaourt()
{
  m_toolButtonYaourt = new QToolButton(this);
  m_toolButtonYaourt->setIconSize(QSize(16, 16));
  m_toolButtonYaourt->setIcon(IconHelper::getIconForeignRed());
  m_toolButtonYaourt->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  m_toolButtonYaourt->setAutoRaise(true);
  m_toolButtonYaourt->hide();

  m_menuToolButtonYaourt = new QMenu(this);
  m_menuToolButtonYaourt->addAction(m_actionInstallYaourtUpdates);
  m_toolButtonYaourt->setPopupMode(QToolButton::MenuButtonPopup);
  m_toolButtonYaourt->setMenu(m_menuToolButtonYaourt);
  connect(m_toolButtonYaourt, SIGNAL(clicked()), this, SLOT(outputOutdatedYaourtPackageList()));
}

/*
 * Sets the TabWidget Properties to the given index/tab and change app focus to its child widget
 */
void MainWindow::_changeTabWidgetPropertiesIndex(const int newIndex)
{
  int oldTabIndex = ui->twProperties->currentIndex();
  _ensureTabVisible(newIndex);

  if (newIndex == oldTabIndex)
  {
    if (oldTabIndex == ctn_TABINDEX_INFORMATION)
    {
      refreshTabInfo();
    }
    else if (oldTabIndex == ctn_TABINDEX_FILES)
    {
      refreshTabFiles();
    }

    ui->twProperties->currentWidget()->childAt(1,1)->setFocus();
  }
  else
  {
    //For any other tab... just doing the following is enough
    ui->twProperties->currentWidget()->childAt(1,1)->setFocus();
  }
}

/*
 * Sets the current tabWidget index, based on last user session
 */
void MainWindow::initTabWidgetPropertiesIndex()
{
  ui->twProperties->setCurrentIndex(SettingsManager::getCurrentTabIndex());
}

/*
 * This is the 4th tab (Transaction).
 * It pops up whenever the user selects a remove/install action on a selected package
 */
void MainWindow::initTabTransaction()
{
  m_modelTransaction = new QStandardItemModel(this);
  QWidget *tabTransaction = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabTransaction);
  gridLayoutX->setSpacing(0);
  gridLayoutX->setMargin(0);

  QTreeView *tvTransaction = new QTreeView(tabTransaction);
  tvTransaction->setObjectName("tvTransaction");
  tvTransaction->setContextMenuPolicy(Qt::CustomContextMenu);
  tvTransaction->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvTransaction->setDropIndicatorShown(false);
  tvTransaction->setAcceptDrops(false);
  tvTransaction->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tvTransaction->setItemDelegate(new TreeViewPackagesItemDelegate(tvTransaction));
  tvTransaction->header()->setSortIndicatorShown(false);
  tvTransaction->header()->setClickable(false);
  tvTransaction->header()->setMovable(false);
  tvTransaction->setFrameShape(QFrame::NoFrame);
  tvTransaction->setFrameShadow(QFrame::Plain);
  tvTransaction->setStyleSheet(StrConstants::getTreeViewCSS());
  tvTransaction->expandAll();

  m_modelTransaction->setSortRole(0);
  m_modelTransaction->setColumnCount(0);

  QStringList sl;
  m_modelTransaction->setHorizontalHeaderLabels(sl << StrConstants::getPackages());

  QStandardItem *siToBeRemoved = new QStandardItem(IconHelper::getIconToRemove(),
                                                   StrConstants::getTransactionRemoveText());
  QStandardItem *siToBeInstalled = new QStandardItem(IconHelper::getIconToInstall(),
                                                     StrConstants::getTransactionInstallText());

  m_modelTransaction->appendRow(siToBeRemoved);
  m_modelTransaction->appendRow(siToBeInstalled);

  gridLayoutX->addWidget(tvTransaction, 0, 0, 1, 1);

  tvTransaction->setModel(m_modelTransaction);

  QString aux(StrConstants::getTabTransactionName());
  ui->twProperties->removeTab(ctn_TABINDEX_TRANSACTION);
  ui->twProperties->insertTab(ctn_TABINDEX_TRANSACTION, tabTransaction, QApplication::translate (
                                "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ));

  connect(tvTransaction, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(execContextMenuTransaction(QPoint)));
  connect(tvTransaction->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(tvTransactionSelectionChanged(QItemSelection,QItemSelection)));

  connect(tvTransaction->model(), SIGNAL(rowsInserted ( const QModelIndex , int, int )),
          this, SLOT(tvTransactionRowsInserted(QModelIndex,int,int)));
  connect(tvTransaction->model(), SIGNAL(rowsRemoved ( const QModelIndex , int, int )),
          this, SLOT(tvTransactionRowsRemoved(QModelIndex,int,int)));
}

/*
 * This is the LineEdit widget used to filter the package list
 */
void MainWindow::initLineEditFilterPackages(){
  connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
}

/*
 * This is the package treeview, it lists the installed [and not installed] packages in the system
 */
void MainWindow::initPackageTreeView()
{
  m_proxyModelPackages = new QSortFilterProxyModel(this);
  m_modelPackages = new QStandardItemModel(this);
  m_modelPackagesClone = new QStandardItemModel(this);
  m_modelInstalledPackages = new QStandardItemModel(this);
  m_modelPackagesFromGroup = new QStandardItemModel(this);
  m_modelInstalledPackagesFromGroup = new QStandardItemModel(this);
  m_proxyModelPackages->setSourceModel(m_modelPackages);
  m_proxyModelPackages->setFilterKeyColumn(ctn_PACKAGE_NAME_COLUMN);

  ui->tvPackages->setAlternatingRowColors(true);
  ui->tvPackages->setItemDelegate(new TreeViewPackagesItemDelegate(ui->tvPackages));
  ui->tvPackages->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->tvPackages->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->tvPackages->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tvPackages->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  ui->tvPackages->setAllColumnsShowFocus( true );
  ui->tvPackages->setModel(m_proxyModelPackages);
  ui->tvPackages->setSortingEnabled( true );
  ui->tvPackages->sortByColumn(ctn_PACKAGE_NAME_COLUMN, Qt::AscendingOrder);
  ui->tvPackages->setIndentation( 0 );
  ui->tvPackages->header()->setSortIndicatorShown(true);
  ui->tvPackages->header()->setClickable(true);
  ui->tvPackages->header()->setMovable(false);
  ui->tvPackages->header()->setDefaultAlignment( Qt::AlignLeft );
  ui->tvPackages->header()->setResizeMode( QHeaderView::Fixed );
  ui->tvPackages->setStyleSheet(
        StrConstants::getTreeViewCSS());

  connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(tvPackagesSelectionChanged(QItemSelection,QItemSelection)));
  connect(ui->tvPackages, SIGNAL(activated(QModelIndex)), this, SLOT(changedTabIndex()));
  connect(ui->tvPackages, SIGNAL(clicked(QModelIndex)), this, SLOT(changedTabIndex()));
  connect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
          SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));
  connect(ui->tvPackages, SIGNAL(customContextMenuRequested(QPoint)), this,
          SLOT(execContextMenuPackages(QPoint)));
  connect(ui->tvPackages, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickPackageList()));
}

/*
 * This tab has a QTextBrowser which shows information about the selected package
 */
void MainWindow::initTabInfo(){
  QWidget *tabInfo = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout ( tabInfo );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  QTextBrowser *text = new QTextBrowser(tabInfo);
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  text->setOpenExternalLinks(true);
  gridLayoutX->addWidget ( text, 0, 0, 1, 1 );

  QString tabName(StrConstants::getTabInfoName());
  ui->twProperties->removeTab(ctn_TABINDEX_INFORMATION);
  ui->twProperties->insertTab(ctn_TABINDEX_INFORMATION, tabInfo, QApplication::translate (
      "MainWindow", tabName.toUtf8(), 0, QApplication::UnicodeUTF8 ) );

  ui->twProperties->setUsesScrollButtons(false);
  ui->twProperties->setCurrentIndex(ctn_TABINDEX_INFORMATION);
  text->show();
  text->setFocus();
}

/*
 * This is the files treeview, which shows the directory structure of ONLY installed packages's files.
 */
void MainWindow::initTabFiles()
{
  QWidget *tabPkgFileList = new QWidget(this);
  QGridLayout *gridLayoutX = new QGridLayout ( tabPkgFileList );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );
  QStandardItemModel *modelPkgFileList = new QStandardItemModel(this);
  QTreeView *tvPkgFileList = new QTreeView(tabPkgFileList);
  tvPkgFileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvPkgFileList->setDropIndicatorShown(false);
  tvPkgFileList->setAcceptDrops(false);
  tvPkgFileList->header()->setSortIndicatorShown(false);
  tvPkgFileList->header()->setClickable(false);
  tvPkgFileList->header()->setMovable(false);
  tvPkgFileList->setFrameShape(QFrame::NoFrame);
  tvPkgFileList->setFrameShadow(QFrame::Plain);
  tvPkgFileList->setObjectName("tvPkgFileList");
  tvPkgFileList->setStyleSheet(StrConstants::getTreeViewCSS());

  modelPkgFileList->setSortRole(0);
  modelPkgFileList->setColumnCount(0);
  gridLayoutX->addWidget(tvPkgFileList, 0, 0, 1, 1);
  tvPkgFileList->setModel(modelPkgFileList);

  QString aux(StrConstants::getTabFilesName());
  ui->twProperties->removeTab(ctn_TABINDEX_FILES);
  ui->twProperties->insertTab(ctn_TABINDEX_FILES, tabPkgFileList, QApplication::translate (
                                                  "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );

  tvPkgFileList->setContextMenuPolicy(Qt::CustomContextMenu);

  SearchBar *searchBar = new SearchBar(this);

  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChangedEx(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosedEx()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNextEx()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPreviousEx()));

  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);

  connect(tvPkgFileList, SIGNAL(customContextMenuRequested(QPoint)),
          this, SLOT(execContextMenuPkgFileList(QPoint)));
  connect(tvPkgFileList, SIGNAL(doubleClicked (const QModelIndex&)),
          this, SLOT(openFile()));
}

/*
 * This is the TextEdit output tab, which shows the output of pacman commands.
 */
void MainWindow::initTabOutput()
{
  QWidget *tabOutput = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout(tabOutput);
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  QTextBrowser *text = new QTextBrowser(tabOutput);
  text->setObjectName("textOutputEdit");
  text->setReadOnly(true);
  text->setOpenLinks(false);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);

  connect(text, SIGNAL(anchorClicked(QUrl)), this, SLOT(outputTextBrowserAnchorClicked(QUrl)));

  gridLayoutX->addWidget (text, 0, 0, 1, 1);  

  QString aux(StrConstants::getTabOutputName());
  ui->twProperties->removeTab(ctn_TABINDEX_OUTPUT);
  ui->twProperties->insertTab(ctn_TABINDEX_OUTPUT, tabOutput, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  text->show();
  text->setFocus();
}

/*
 * Initialize QAction objects
 */
void MainWindow::initActions()
{
  m_actionInstallPacmanUpdates = new QAction(this);
  m_actionInstallPacmanUpdates->setIcon(IconHelper::getIconToInstall());
  m_actionInstallPacmanUpdates->setText(ui->actionInstall->text());
  m_actionInstallPacmanUpdates->setIconVisibleInMenu(true);
  connect(m_actionInstallPacmanUpdates, SIGNAL(triggered()), this, SLOT(doSystemUpgrade()));

  m_actionInstallYaourtUpdates = new QAction(this);
  m_actionInstallYaourtUpdates->setIcon(IconHelper::getIconToInstall());
  m_actionInstallYaourtUpdates->setText(ui->actionInstall->text());
  m_actionInstallYaourtUpdates->setIconVisibleInMenu(true);
  connect(m_actionInstallYaourtUpdates, SIGNAL(triggered()), this, SLOT(doYaourtUpgrade()));

  toggleTransactionActions(true);

  ui->actionShowGroups->setText(StrConstants::getGroups());
  ui->actionShowGroups->setCheckable(true);
  ui->actionShowGroups->setChecked(true);
  connect(ui->actionShowGroups, SIGNAL(triggered()), this, SLOT(hideGroupsWidget()));

  QActionGroup *actionGroup = new QActionGroup(this);
  actionGroup->addAction(ui->actionSearchByDescription);
  actionGroup->addAction(ui->actionSearchByName);
  ui->actionSearchByName->setChecked(true);
  actionGroup->setExclusive(true);

  connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(tvPackagesSearchColumnChanged(QAction*)));

  ui->actionInstallLocalPackage->setIcon(IconHelper::getIconFolder());

  ui->actionOpenDirectory->setIcon(IconHelper::getIconFolder());

  connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(invalidateTabs()));

  connect(ui->actionInstallLocalPackage, SIGNAL(triggered()), this, SLOT(installLocalPackage()));

  connect(ui->actionRemoveTransactionItem, SIGNAL(triggered()), this, SLOT(onPressDelete()));
  connect(ui->actionRemoveTransactionItems, SIGNAL(triggered()), this, SLOT(onPressDelete()));
  connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
  connect(ui->actionNonInstalledPackages, SIGNAL(changed()), this, SLOT(changePackageListModel()));
  connect(ui->actionSyncPackages, SIGNAL(triggered()), this, SLOT(doSyncDatabase()));
  connect(ui->actionSystemUpgrade, SIGNAL(triggered()), this, SLOT(doSystemUpgrade()));
  connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(insertIntoRemovePackage()));
  connect(ui->actionInstall, SIGNAL(triggered()), this, SLOT(insertIntoInstallPackage()));
  connect(ui->actionInstallYaourt, SIGNAL(triggered()), this, SLOT(doInstallYaourtPackage()));
  connect(ui->actionFindFileInPackage, SIGNAL(triggered()), this, SLOT(findFileInPackage()));
  connect(ui->actionRemoveGroup, SIGNAL(triggered()), this, SLOT(insertGroupIntoRemovePackage()));
  connect(ui->actionInstallGroup, SIGNAL(triggered()), this, SLOT(insertGroupIntoInstallPackage()));
  connect(ui->actionCommit, SIGNAL(triggered()), this, SLOT(doCommitTransaction()));
  connect(ui->actionRollback, SIGNAL(triggered()), this, SLOT(doRollbackTransaction()));
  connect(ui->actionGetNews, SIGNAL(triggered()), this, SLOT(refreshDistroNews()));
  connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
  connect(ui->actionHelpUsage, SIGNAL(triggered()), this, SLOT(onHelpUsage()));
  connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(onHelpAbout()));

  //Actions from tvPkgFileList context menu
  connect(ui->actionCollapseAllItems, SIGNAL(triggered()), this, SLOT(collapseAllContentItems()));
  connect(ui->actionExpandAllItems, SIGNAL(triggered()), this, SLOT(expandAllContentItems()));
  connect(ui->actionCollapseItem, SIGNAL(triggered()), this, SLOT(collapseThisContentItems()));
  connect(ui->actionExpandItem, SIGNAL(triggered()), this, SLOT(expandThisContentItems()));
  connect(ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
  connect(ui->actionEditFile, SIGNAL(triggered()), this, SLOT(editFile()));
  connect(ui->actionOpenDirectory, SIGNAL(triggered()), this, SLOT(openDirectory()));
  connect(ui->actionOpenTerminal, SIGNAL(triggered()), this, SLOT(openTerminal()));
  connect(ui->actionOpenRootTerminal, SIGNAL(triggered()), this, SLOT(openRootTerminal()));

  if (WMHelper::isXFCERunning())
  {
    //Loop through all actions and set their icons (if any) visible to menus.
    foreach(QAction* ac, this->findChildren<QAction*>(QRegExp("(m_a|a)ction\\S*")))
    {
      if (ac) ac->setIconVisibleInMenu(true);
    }
  }
}
