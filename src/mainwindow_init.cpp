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
#include "repoconf.h"

#include <QLabel>
#include <QStandardItemModel>
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
#include <cassert>

/*
 * Loads various application settings configured in ~/.config/octopi/octopi.conf
 */
void MainWindow::loadSettings(){
  if (ui->tvPackages->model() != NULL)
  {
    int packageListOrderedCol = SettingsManager::instance()->getPackageListOrderedCol();
    Qt::SortOrder packageListSortOrder = (Qt::SortOrder) SettingsManager::instance()->getPackageListSortOrder();

    ui->tvPackages->header()->setSortIndicator( packageListOrderedCol, packageListSortOrder );
    ui->tvPackages->sortByColumn( packageListOrderedCol, packageListSortOrder );
  }
  else assert(false);
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
      SettingsManager::instance()->setPackageListOrderedCol(ui->tvPackages->header()->sortIndicatorSection());
      SettingsManager::instance()->setPackageListSortOrder(ui->tvPackages->header()->sortIndicatorOrder());
      break;
  }
}

/*
 * If we have some outdated packages, let's put octopi in a red face/angry state ;-)
 */
void MainWindow::initAppIcon()
{
  m_outdatedPackageList = Package::getOutdatedPackageList();

  if (UnixCommand::hasTheExecutable(StrConstants::getForeignRepositoryToolName()))
  {
    m_outdatedAURPackageList = Package::getOutdatedAURPackageList();
  }

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
      if (this->isMinimized()) this->setWindowState(Qt::WindowNoState);
      this->show();
    }
    else {
      this->hide();
    }
    break;

  default: break;
  }
}

/*
 * Inits the Groups combobox, so it can be added in app's toolBar
 */
void MainWindow::initPackageGroups()
{
  //This is the twGroups init code
  ui->twGroups->setColumnCount(1);
  ui->twGroups->setHeaderLabel(StrConstants::getGroups());
  ui->twGroups->header()->setSortIndicatorShown(false);

  #if QT_VERSION < 0x050000
    ui->twGroups->header()->setClickable(false);
    ui->twGroups->header()->setMovable(false);
    ui->twGroups->header()->setResizeMode(QHeaderView::Fixed);
  #else
    ui->twGroups->header()->setSectionsClickable(false);
    ui->twGroups->header()->setSectionsMovable(false);
    ui->twGroups->header()->setSectionResizeMode(QHeaderView::Fixed);
  #endif

  ui->twGroups->setFrameShape(QFrame::NoFrame);
  ui->twGroups->setFrameShadow(QFrame::Plain);
  ui->twGroups->setStyleSheet(StrConstants::getTreeViewCSS());
  ui->twGroups->setSelectionMode(QAbstractItemView::SingleSelection);

  connect(ui->twGroups, SIGNAL(itemSelectionChanged()), this, SLOT(onPackageGroupChanged()));
}

/*
 * Whenever user changes the Groups combobox, this method will be triggered...
 */
void MainWindow::onPackageGroupChanged()
{
  if (isAllGroupsSelected())
  {
    ui->actionSearchByName->setChecked(true);
    tvPackagesSearchColumnChanged(ui->actionSearchByName);
  }
}

/*
 * Inits the Repository menuItem in menuBar
 */
void MainWindow::initMenuBar()
{
  QActionGroup *actionGroupPackages = new QActionGroup(this);
  QActionGroup *actionGroupRepositories = new QActionGroup(this);

  ui->actionViewAllPackages->setText(StrConstants::getAll());
  ui->actionViewAllPackages->setChecked(true);
  actionGroupPackages->addAction(ui->actionViewAllPackages);
  actionGroupPackages->addAction(ui->actionViewInstalledPackages);
  actionGroupPackages->addAction(ui->actionViewNonInstalledPackages);
  actionGroupPackages->setExclusive(true);

  m_actionMenuRepository = ui->menuView->addAction(StrConstants::getRepository());
  QMenu *subMenu = new QMenu(ui->menuView);
  connect(subMenu, SIGNAL(triggered(QAction*)), this, SLOT(selectedRepositoryMenu(QAction*)));

  m_actionRepositoryAll = subMenu->addAction(StrConstants::getAll());
  m_actionRepositoryAll->setCheckable(true);
  m_actionRepositoryAll->setChecked(true);
  subMenu->addSeparator();

  RepoConf *repoConf = new RepoConf();
  QStringList repos = repoConf->getRepos();

  foreach(QString repo, repos)
  {
    QAction * createdAction = subMenu->addAction(repo);
    createdAction->setCheckable(true);
    actionGroupRepositories->addAction(createdAction);
  }

  QAction * actionForeignRepo = subMenu->addAction(StrConstants::getForeignRepositoryName());
  actionForeignRepo->setCheckable(true);
  actionGroupRepositories->addAction(m_actionRepositoryAll);
  actionGroupRepositories->addAction(actionForeignRepo);
  actionGroupRepositories->setExclusive(true);

  m_actionMenuRepository->setMenu(subMenu);
}

/*
 * Inits the toolbar, including taking that annoying default view action out of the game
 */
void MainWindow::initToolBar()
{
  initPackageGroups();

  ui->mainToolBar->addAction(ui->actionSyncPackages);
  ui->mainToolBar->addAction(ui->actionSystemUpgrade);

  if (m_outdatedPackageList->count() > 0)
    ui->actionSystemUpgrade->setEnabled(true);
  else
    ui->actionSystemUpgrade->setEnabled(false);

  ui->mainToolBar->addAction(ui->actionCommit);
  ui->mainToolBar->addAction(ui->actionRollback);

  if(UnixCommand::hasTheExecutable(ctn_MIRROR_CHECK_APP))
  {
    ui->mainToolBar->addAction(m_actionMirrorCheck);
  }

  m_leFilterPackage->setMinimumHeight(24);
  ui->mainToolBar->addWidget(m_leFilterPackage);

  QWidget * hSpacer = new QWidget(this);
  hSpacer->setMinimumHeight(22);
  hSpacer->setMinimumWidth(3);
  hSpacer->setVisible(true);
  ui->mainToolBar->addWidget(hSpacer);
  ui->mainToolBar->addAction(m_actionShowGroups);
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
 * Inits the AUR toolbutton, which warns the user about outdated foreign packages
 */
void MainWindow::initToolButtonAUR()
{
  m_toolButtonAUR = new QToolButton(this);
  m_toolButtonAUR->setIconSize(QSize(16, 16));
  m_toolButtonAUR->setIcon(IconHelper::getIconForeignRed());
  m_toolButtonAUR->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  m_toolButtonAUR->setAutoRaise(true);
  m_toolButtonAUR->hide();

  m_menuToolButtonAUR = new QMenu(this);
  m_menuToolButtonAUR->addAction(m_actionInstallAURUpdates);
  m_toolButtonAUR->setPopupMode(QToolButton::MenuButtonPopup);
  m_toolButtonAUR->setMenu(m_menuToolButtonAUR);
  connect(m_toolButtonAUR, SIGNAL(clicked()), this, SLOT(outputOutdatedAURPackageList()));
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

#if QT_VERSION < 0x050000
  tvTransaction->header()->setClickable(false);
  tvTransaction->header()->setMovable(false);
  tvTransaction->header()->setResizeMode(QHeaderView::Fixed);
#else
  tvTransaction->header()->setSectionsClickable(false);
  tvTransaction->header()->setSectionsMovable(false);
  tvTransaction->header()->setSectionResizeMode(QHeaderView::Fixed);
#endif

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

#if QT_VERSION < 0x050000
  ui->twProperties->insertTab(ctn_TABINDEX_TRANSACTION, tabTransaction, QApplication::translate (
                                "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ));
#else
  ui->twProperties->insertTab(ctn_TABINDEX_TRANSACTION, tabTransaction, QApplication::translate (
                                "MainWindow", aux.toUtf8(), 0/*, QApplication::UnicodeUTF8*/ ));
#endif

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
  ui->tvPackages->setAlternatingRowColors(true);
  ui->tvPackages->setItemDelegate(new TreeViewPackagesItemDelegate(ui->tvPackages));
  ui->tvPackages->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->tvPackages->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->tvPackages->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tvPackages->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  ui->tvPackages->setAllColumnsShowFocus( true );
  ui->tvPackages->setModel(m_packageModel.get());
  ui->tvPackages->setSortingEnabled( true );
  ui->tvPackages->setIndentation( 0 );
  ui->tvPackages->header()->setSortIndicatorShown(true);

#if QT_VERSION < 0x050000
  ui->tvPackages->header()->setClickable(true);
  ui->tvPackages->header()->setMovable(false);
  ui->tvPackages->header()->setResizeMode( QHeaderView::Interactive );
#else
  ui->tvPackages->header()->setSectionsClickable(true);
  ui->tvPackages->header()->setSectionsMovable(false);
  ui->tvPackages->header()->setSectionResizeMode(QHeaderView::Interactive);
#endif

  ui->tvPackages->header()->setDefaultAlignment( Qt::AlignLeft );
  ui->tvPackages->setStyleSheet(StrConstants::getTreeViewCSS());

  resizePackageView();

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

void MainWindow::resizePackageView()
{
  ui->tvPackages->setColumnWidth(PackageModel::ctn_PACKAGE_ICON_COLUMN, 24);
  ui->tvPackages->setColumnWidth(PackageModel::ctn_PACKAGE_NAME_COLUMN, 400); //500
  ui->tvPackages->setColumnWidth(PackageModel::ctn_PACKAGE_VERSION_COLUMN, 160);
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

#if QT_VERSION < 0x050000
  ui->twProperties->insertTab(ctn_TABINDEX_INFORMATION, tabInfo, QApplication::translate (
      "MainWindow", tabName.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
#else
  ui->twProperties->insertTab(ctn_TABINDEX_INFORMATION, tabInfo, QApplication::translate (
      "MainWindow", tabName.toUtf8(), 0/*, QApplication::UnicodeUTF8*/ ) );
#endif

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

#if QT_VERSION < 0x050000
  tvPkgFileList->header()->setClickable(false);
  tvPkgFileList->header()->setMovable(false);
  tvPkgFileList->header()->setResizeMode(QHeaderView::Fixed);
#else
  tvPkgFileList->header()->setSectionsClickable(false);
  tvPkgFileList->header()->setSectionsMovable(false);
  tvPkgFileList->header()->setSectionResizeMode(QHeaderView::Fixed);
#endif

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

#if QT_VERSION < 0x050000
  ui->twProperties->insertTab(ctn_TABINDEX_FILES, tabPkgFileList, QApplication::translate (
                                                  "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
#else
  ui->twProperties->insertTab(ctn_TABINDEX_FILES, tabPkgFileList, QApplication::translate (
                                                  "MainWindow", aux.toUtf8(), 0/*, QApplication::UnicodeUTF8*/ ) );
#endif

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

#if QT_VERSION < 0x050000
  ui->twProperties->insertTab(ctn_TABINDEX_OUTPUT, tabOutput, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
#else
  ui->twProperties->insertTab(ctn_TABINDEX_OUTPUT, tabOutput, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0/*, QApplication::UnicodeUTF8*/ ) );
#endif

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  text->show();
  text->setFocus();
}

/*
 * Initialize QAction objects
 */
void MainWindow::initActions()
{
  if(UnixCommand::hasTheExecutable(ctn_MIRROR_CHECK_APP))
  {
    m_actionMirrorCheck = new QAction(this);
    m_actionMirrorCheck->setShortcut(QKeySequence(Qt::ControlModifier|Qt::ShiftModifier|Qt::Key_M));
    m_actionMirrorCheck->setText("Mirror-Check");
    m_actionMirrorCheck->setIcon(IconHelper::getIconMirrorCheck());
    connect(m_actionMirrorCheck, SIGNAL(triggered()), this, SLOT(doMirrorCheck()));
  }

  m_actionInstallPacmanUpdates = new QAction(this);
  m_actionInstallPacmanUpdates->setIcon(IconHelper::getIconToInstall());
  m_actionInstallPacmanUpdates->setText(ui->actionInstall->text());
  m_actionInstallPacmanUpdates->setIconVisibleInMenu(true);
  connect(m_actionInstallPacmanUpdates, SIGNAL(triggered()), this, SLOT(doSystemUpgrade()));

  m_actionInstallAURUpdates = new QAction(this);
  m_actionInstallAURUpdates->setIcon(IconHelper::getIconToInstall());
  m_actionInstallAURUpdates->setText(ui->actionInstall->text());
  m_actionInstallAURUpdates->setIconVisibleInMenu(true);
  connect(m_actionInstallAURUpdates, SIGNAL(triggered()), this, SLOT(doAURUpgrade()));

  toggleTransactionActions(true);

  m_actionShowGroups = new QAction(this);
  m_actionShowGroups->setIcon(IconHelper::getIconShowGroups());
  m_actionShowGroups->setText(StrConstants::getGroups());
  m_actionShowGroups->setCheckable(true);
  m_actionShowGroups->setChecked(true);
  m_actionShowGroups->setShortcut(QKeySequence(Qt::Key_F9));
  connect(m_actionShowGroups, SIGNAL(triggered()), this, SLOT(hideGroupsWidget()));

  QActionGroup *actionGroup = new QActionGroup(this);
  actionGroup->addAction(ui->actionSearchByDescription);
  actionGroup->addAction(ui->actionSearchByName);
  actionGroup->addAction(ui->actionSearchByFile);
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
  connect(ui->actionSyncPackages, SIGNAL(triggered()), this, SLOT(doSyncDatabase()));
  connect(ui->actionSystemUpgrade, SIGNAL(triggered()), this, SLOT(doSystemUpgrade()));
  connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(insertIntoRemovePackage()));
  connect(ui->actionInstall, SIGNAL(triggered()), this, SLOT(insertIntoInstallPackage()));
  connect(ui->actionInstallAUR, SIGNAL(triggered()), this, SLOT(doInstallAURPackage()));
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

  // Use theme icons for QActions
  ui->actionSyncPackages->setIcon(IconHelper::getIconSyncPackages());
  ui->actionCommit->setIcon(IconHelper::getIconCommit());
  ui->actionRollback->setIcon(IconHelper::getIconRollback());
  ui->actionExit->setIcon(IconHelper::getIconExit());
  ui->actionSystemUpgrade->setIcon(IconHelper::getIconSystemUpgrade());
  ui->actionInstall->setIcon(IconHelper::getIconInstallItem());
  ui->actionRemove->setIcon(IconHelper::getIconRemoveItem());
  ui->actionGetNews->setIcon(IconHelper::getIconGetNews());
  ui->actionRemoveGroup->setIcon(IconHelper::getIconRemoveItem());
  ui->actionInstallGroup->setIcon(IconHelper::getIconInstallItem());
  ui->actionCollapseItem->setIcon(IconHelper::getIconCollapse());
  ui->actionExpandItem->setIcon(IconHelper::getIconExpand());
  ui->actionCollapseAllItems->setIcon(IconHelper::getIconCollapse());
  ui->actionExpandAllItems->setIcon(IconHelper::getIconExpand());
  ui->actionOpenFile->setIcon(IconHelper::getIconBinary());
  ui->actionEditFile->setIcon(IconHelper::getIconEditFile());
  ui->actionOpenDirectory->setIcon(IconHelper::getIconFolder());
  ui->actionOpenTerminal->setIcon(IconHelper::getIconTerminal());
  ui->actionRemoveTransactionItem->setIcon(IconHelper::getIconClose());
  ui->actionRemoveTransactionItems->setIcon(IconHelper::getIconClose());
  ui->actionFindFileInPackage->setIcon(IconHelper::getIconFindFileInPackage());
  ui->actionOpenRootTerminal->setIcon(IconHelper::getIconTerminal());
  ui->actionInstallAUR->setIcon(IconHelper::getIconInstallItem());

  //Actions for the View menu
  connect(ui->actionViewAllPackages, SIGNAL(triggered()), this, SLOT(selectedAllPackagesMenu()));
  connect(ui->actionViewInstalledPackages, SIGNAL(triggered()), this, SLOT(selectedInstalledPackagesMenu()));
  connect(ui->actionViewNonInstalledPackages, SIGNAL(triggered()), this, SLOT(selectedNonInstalledPackagesMenu()));

  if (WMHelper::isKDERunning() && UnixCommand::getLinuxDistro() != ectn_KAOS)
  {
    ui->actionHelpAbout->setIcon(IconHelper::getIconHelpAbout());
    ui->actionHelpUsage->setIcon(IconHelper::getIconHelpUsage());
    ui->actionInstallLocalPackage->setIcon(IconHelper::getIconInstallLocalPackage());
  }

  // Populate Tools menu
  ui->menuTools->menuAction()->setVisible(false);

  if(UnixCommand::hasTheExecutable("plv"))
  {
    ui->menuTools->menuAction()->setVisible(true);
    ui->actionPacmanLogViewer->setIcon(QIcon::fromTheme("plv"));
    connect(ui->actionPacmanLogViewer, SIGNAL(triggered()), this, SLOT(launchPLV()));
  }
  else
    ui->actionPacmanLogViewer->setVisible(false);

  if(UnixCommand::hasTheExecutable("octopi-repoeditor") && UnixCommand::getLinuxDistro() != ectn_KAOS)
  {
    ui->menuTools->menuAction()->setVisible(true);
    connect(ui->actionRepositoryEditor, SIGNAL(triggered()), this, SLOT(launchRepoEditor()));
  }
  else
    ui->actionRepositoryEditor->setVisible(false);

  if (WMHelper::isXFCERunning())
  {
    //Loop through all actions and set their icons (if any) visible to menus.
    foreach(QAction* ac, this->findChildren<QAction*>(QRegExp("(m_a|a)ction\\S*")))
    {
      if (ac) ac->setIconVisibleInMenu(true);
    }
  }
}
