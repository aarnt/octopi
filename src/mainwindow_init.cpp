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
#include "repoconf.h"
#include <iostream>
#include <cassert>
#include "termwidget.h"

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
#include <QDebug>

/*
 * Loads various application settings configured in ~/.config/octopi/octopi.conf
 */
void MainWindow::loadSettings()
{
  if (ui->tvPackages->model() != nullptr)
  {
    int packageListOrderedCol = SettingsManager::instance()->getPackageListOrderedCol();
    Qt::SortOrder packageListSortOrder = (Qt::SortOrder) SettingsManager::instance()->getPackageListSortOrder();

    ui->tvPackages->header()->setSortIndicator( packageListOrderedCol, packageListSortOrder );
    ui->tvPackages->sortByColumn( packageListOrderedCol, packageListSortOrder );

    /*if (!SettingsManager::isValidSUToolSelected()){
      SettingsManager::setSUTool(ctn_AUTOMATIC);
    }*/
  }
  else assert(false);

  ui->actionUseInstantSearch->setChecked(SettingsManager::isInstantSearchSelected());
}

/*
 * This method only retrieves the App saved panels settings
 */
void MainWindow::loadPanelSettings()
{
  int panelOrganizing = SettingsManager::instance()->getPanelOrganizing();

  switch(panelOrganizing){
    case ectn_MAXIMIZE_PACKAGES:
      maximizePackagesTreeView();
      break;
    case ectn_MAXIMIZE_PROPERTIES:
      maximizePropertiesTabWidget();
      break;
    case ectn_NORMAL:
      ui->splitterHorizontal->restoreState(SettingsManager::instance()->getSplitterHorizontalState());
      break;
  }

  //Do we have to show or hide the Groups panel?
  if (!SettingsManager::getShowGroupsPanel())
  {
    hideGroupsWidget();
  }
}

/*
 * Saves all application settings to ~/.config/octopi/octopi.conf
 */
void MainWindow::saveSettings(SaveSettingsReason saveSettingsReason)
{
  switch(saveSettingsReason){
    case ectn_CONSOLE_FONT_SIZE:
      break;

    case ectn_CURRENTTABINDEX:
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
      if (m_actionShowGroups->isChecked())
        SettingsManager::instance()->setShowGroupsPanel(true); //And also show Groups panel!
      break;

    case ectn_AUR_PACKAGELIST:
      SettingsManager::instance()->setAURPackageListOrderedCol(ui->tvPackages->header()->sortIndicatorSection());
      SettingsManager::instance()->setAURPackageListSortOrder(ui->tvPackages->header()->sortIndicatorOrder());
      break;

    case ectn_PACKAGELIST:
      SettingsManager::instance()->setPackageListOrderedCol(ui->tvPackages->header()->sortIndicatorSection());
      SettingsManager::instance()->setPackageListSortOrder(ui->tvPackages->header()->sortIndicatorOrder());
      break;            

    case ectn_GROUPS:
      QList<int> rl;
      rl = ui->splitterVertical->sizes();

      bool show=false;
      if ( rl[1] != 0 )
      {
        show = true;
      }

      SettingsManager::instance()->setShowGroupsPanel(show);
      break;
  }  
}

/*
 * Save Package treeview column widths to bring them back in the next execution
 */
void MainWindow::savePackageColumnWidths()
{
  int width = ui->tvPackages->columnWidth(PackageModel::ctn_PACKAGE_ICON_COLUMN);
  if (width > 0)
  {
  SettingsManager::setPackageIconColumnWidth(
        ui->tvPackages->columnWidth(PackageModel::ctn_PACKAGE_ICON_COLUMN));
  }

  width = ui->tvPackages->columnWidth(PackageModel::ctn_PACKAGE_NAME_COLUMN);
  if (width > 0)
  {
  SettingsManager::setPackageNameColumnWidth(
        ui->tvPackages->columnWidth(PackageModel::ctn_PACKAGE_NAME_COLUMN));
  }

  width = ui->tvPackages->columnWidth(PackageModel::ctn_PACKAGE_VERSION_COLUMN);
  if (width > 0)
  {
  SettingsManager::setPackageVersionColumnWidth(
        ui->tvPackages->columnWidth(PackageModel::ctn_PACKAGE_VERSION_COLUMN));
  }

  width = ui->tvPackages->columnWidth(PackageModel::ctn_PACKAGE_REPOSITORY_COLUMN);
  if (width > 0)
  {
    SettingsManager::setPackageRepositoryColumnWidth(
        ui->tvPackages->columnWidth(PackageModel::ctn_PACKAGE_REPOSITORY_COLUMN));
  }
}

/*
 * If we have some outdated packages, let's put octopi in a red face/angry state ;-)
 */
void MainWindow::initAppIcon()
{
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
  ui->twGroups->header()->setSectionsClickable(false);
  ui->twGroups->header()->setSectionsMovable(false);
  ui->twGroups->header()->setSectionResizeMode(QHeaderView::Fixed);
  ui->twGroups->setFrameShape(QFrame::NoFrame);
  ui->twGroups->setFrameShadow(QFrame::Plain);
  ui->twGroups->setStyleSheet(StrConstants::getTreeViewCSS());
  ui->twGroups->setSelectionMode(QAbstractItemView::SingleSelection);
  connect(ui->twGroups, SIGNAL(itemSelectionChanged()), this, SLOT(onPackageGroupChanged()));
}

/*
 * Inits the menu options and Repository menuItem in menuBar
 */
void MainWindow::initMenuBar()
{
  ui->menuTools->insertAction(ui->actionCacheCleaner, m_actionMenuMirrorCheck);
  ui->menuTools->addSeparator();
  ui->menuTools->addAction(m_actionMenuOptions);

  QActionGroup *actionGroupPackages = new QActionGroup(this);
  QActionGroup *actionGroupRepositories = new QActionGroup(this);

  ui->menuActions->menuAction()->setVisible(false);

  ui->actionViewAllPackages->setText(StrConstants::getAll());

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

  subMenu->addSeparator();
  QAction * m_actionForeignRepository = subMenu->addAction(StrConstants::getForeignRepositoryName());
  m_actionForeignRepository->setCheckable(true);
  actionGroupRepositories->addAction(m_actionForeignRepository);
  
  actionGroupRepositories->addAction(m_actionRepositoryAll);
  actionGroupRepositories->setExclusive(true);
  m_actionMenuRepository->setMenu(subMenu);

  /*foreach (QAction * act,  ui->menuBar->actions())
  {
    QString text = act->text();
    text = text.remove("&");
    act->setText(qApp->translate("MainWindow", text.toUtf8(), 0));
  }*/

#ifdef OCTOPI_DEV_CODE
  ui->menuFile->insertAction(ui->actionExit, m_actionEditOctopiConf);
#endif
}

/*
 * Inits the toolbar, including taking that annoying default view action out of the game
 */
void MainWindow::initToolBar()
{
  initPackageGroups();

  ui->mainToolBar->addAction(ui->actionCheckUpdates);
  ui->mainToolBar->addAction(ui->actionSystemUpgrade);

  if (m_outdatedStringList->count() > 0)
    ui->actionSystemUpgrade->setEnabled(true);
  else
    ui->actionSystemUpgrade->setEnabled(false);

  ui->mainToolBar->addAction(ui->actionApply);
  ui->mainToolBar->addAction(ui->actionCancel);

  if(m_hasMirrorCheck)
  {
    ui->mainToolBar->addAction(m_actionMenuMirrorCheck);
  }

  if (m_hasAURTool)
  {
    m_separatorForActionAUR = ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(m_actionSwitchToAURTool);
    if (SettingsManager::getAURTool() != ctn_NO_AUR_TOOL)
      m_actionSwitchToAURTool->setToolTip(m_actionSwitchToAURTool->toolTip() + "  (Ctrl+Shift+Y)");
  }

  m_dummyAction = new QAction(this);
  m_dummyAction->setVisible(false);
  ui->mainToolBar->addAction(m_dummyAction);

  m_leFilterPackage->setMinimumHeight(24);
  m_leFilterPackage->setPlaceholderText(m_leFilterPackage->placeholderText() + "  (Ctrl+L)");
  ui->mainToolBar->addWidget(m_leFilterPackage);

  QWidget * hSpacer = new QWidget(this);
  hSpacer->setMinimumHeight(22);
  hSpacer->setMinimumWidth(3);
  hSpacer->setVisible(true);
  ui->mainToolBar->addWidget(hSpacer);
  m_actionShowGroups->setToolTip(m_actionShowGroups->toolTip() + "  (F9)");
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
  m_toolButtonStopTransaction = new QToolButton(this);
  m_toolButtonStopTransaction->setDefaultAction(m_actionStopTransaction);
  m_toolButtonStopTransaction->setVisible(false);
  m_toolButtonStopTransaction->setAutoRaise(true);
  ui->statusBar->addPermanentWidget(m_toolButtonStopTransaction);
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
void MainWindow::changeTabWidgetPropertiesIndex(const int newIndex)
{
  int oldTabIndex = ui->twProperties->currentIndex();
  ensureTabVisible(newIndex);

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
  connect(ui->splitterHorizontal, SIGNAL(splitterMoved(int, int)), this, SLOT(horizontalSplitterMoved(int, int)));
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
  tvTransaction->header()->setSectionsClickable(false);
  tvTransaction->header()->setSectionsMovable(false);
  tvTransaction->header()->setSectionResizeMode(QHeaderView::Fixed);
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

  QString aux(StrConstants::getActions());
  ui->twProperties->removeTab(ctn_TABINDEX_ACTIONS);
  ui->twProperties->insertTab(ctn_TABINDEX_ACTIONS, tabTransaction, QApplication::translate (
                                "MainWindow", aux.toUtf8(), 0/*, QApplication::UnicodeUTF8*/ ));

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
  //if (SettingsManager::isInstantSearchSelected())
  //  connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
  toggleInstantSearch();
}

/*
 * This is the package treeview, it lists the installed [and not installed] packages in the system
 */
void MainWindow::initPackageTreeView()
{
  ui->tvPackages->init();
  ui->tvPackages->setModel(m_packageModel.get());
  ui->tvPackages->resizePackageView();

  connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(tvPackagesSelectionChanged(QItemSelection,QItemSelection)));
  connect(ui->tvPackages, SIGNAL(activated(QModelIndex)), this, SLOT(refreshInfoAndFileTabs()));
  connect(ui->tvPackages, SIGNAL(clicked(QModelIndex)), this, SLOT(refreshInfoAndFileTabs()));
  connect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
          SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));
  connect(ui->tvPackages, SIGNAL(customContextMenuRequested(QPoint)), this,
          SLOT(execContextMenuPackages(QPoint)));
  connect(ui->tvPackages, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickPackageList()));
}

/*
 * Remove all Package TreeView "connect" calls
 */
void MainWindow::removePackageTreeViewConnections()
{
  disconnect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(tvPackagesSelectionChanged(QItemSelection,QItemSelection)));
  disconnect(ui->tvPackages, SIGNAL(activated(QModelIndex)), this, SLOT(refreshInfoAndFileTabs()));
  disconnect(ui->tvPackages, SIGNAL(clicked(QModelIndex)), this, SLOT(refreshInfoAndFileTabs()));
  disconnect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
          SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));
  disconnect(ui->tvPackages, SIGNAL(customContextMenuRequested(QPoint)), this,
          SLOT(execContextMenuPackages(QPoint)));
  disconnect(ui->tvPackages, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClickPackageList()));
}

void MainWindow::resizePackageView()
{
  ui->tvPackages->resizePackageView();
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
  text->setOpenLinks(false);
  connect(text, SIGNAL(anchorClicked(QUrl)), this, SLOT(outputTextBrowserAnchorClicked(QUrl)));
  connect(text, SIGNAL(highlighted(QUrl)), this, SLOT(showAnchorDescription(QUrl)));
  gridLayoutX->addWidget ( text, 0, 0, 1, 1 );

  QString tabName(StrConstants::getTabInfoName());
  ui->twProperties->removeTab(ctn_TABINDEX_INFORMATION);
  ui->twProperties->insertTab(ctn_TABINDEX_INFORMATION, tabInfo, QApplication::translate (
      "MainWindow", tabName.toUtf8(), 0));
  ui->twProperties->setUsesScrollButtons(false);

  SearchBar *searchBar = new SearchBar(this);
  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChangedInTextBrowser(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosedInTextBrowser()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNextInTextBrowser()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPreviousInTextBrowser()));
  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_INFORMATION);
  text->show();
  text->setFocus();
}

/*
 * This is the QTermWidget used to exec AUR/pacman commands.
 */
void MainWindow::initTabTerminal()
{
  QWidget *tabTerminal = new QWidget(this);
  QGridLayout *gridLayoutX = new QGridLayout(tabTerminal);
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  m_console = new TermWidget(this);
  connect(m_console, SIGNAL(finished()), this, SLOT(initTabTerminal()));
  connect(m_console, SIGNAL(onKeyQuit()), this, SLOT(close()));
  connect(m_console, SIGNAL(onKeyF11()), this, SLOT(maximizeTerminalTab()));

  gridLayoutX->addWidget(m_console, 0, 0, 1, 1);
  ui->twProperties->removeTab(ctn_TABINDEX_TERMINAL);
  QString aux(StrConstants::getTabTerminal());
  ui->twProperties->insertTab(ctn_TABINDEX_TERMINAL, tabTerminal, QApplication::translate (
                                                  "MainWindow", aux.toUtf8(), 0) );
  ui->twProperties->setCurrentIndex(ctn_TABINDEX_TERMINAL);
  m_console->setFocus();
}

/*
 * Removes tab with the QTermWidget
 */
void MainWindow::removeTabTerminal()
{
  ui->twProperties->removeTab(ctn_TABINDEX_TERMINAL);
  delete m_console;
  m_console = nullptr;
}

/*
 * Enables/Disables Tab Terminal if selected terminal is QTermWidget5
 */
void MainWindow::onTerminalChanged()
{
  if (SettingsManager::getTerminal() == ctn_QTERMWIDGET)
    initTabTerminal();
  else
    removeTabTerminal();
}

/*
 * Executes the given command in the QTermWidget5
 */
void MainWindow::onExecCommandInTabTerminal(QString command)
{
  ensureTabVisible(ctn_TABINDEX_TERMINAL);

  disconnect(m_console, SIGNAL(onPressAnyKeyToContinue()), this, SLOT(onPressAnyKeyToContinue()));
  disconnect(m_console, SIGNAL(onCancelControlKey()), this, SLOT(onCancelControlKey()));
  connect(m_console, SIGNAL(onPressAnyKeyToContinue()), this, SLOT(onPressAnyKeyToContinue()));
  connect(m_console, SIGNAL(onCancelControlKey()), this, SLOT(onCancelControlKey()));

  m_console->enter();
  m_console->execute("clear");
  m_console->execute(command);
  m_console->setFocus();
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
  tvPkgFileList->header()->setSectionsClickable(false);
  tvPkgFileList->header()->setSectionsMovable(false);
  tvPkgFileList->header()->setSectionResizeMode(QHeaderView::Fixed);
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
                                                  "MainWindow", aux.toUtf8(), 0/*, QApplication::UnicodeUTF8*/ ) );
  tvPkgFileList->setContextMenuPolicy(Qt::CustomContextMenu);
  SearchBar *searchBar = new SearchBar(this);
  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChangedInTreeView(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosedInTreeView()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNextInTreeView()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPreviousInTreeView()));

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
  text->setObjectName("textBrowser");
  text->setReadOnly(true);
  text->setOpenLinks(false);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);

  connect(text, SIGNAL(anchorClicked(QUrl)), this, SLOT(outputTextBrowserAnchorClicked(QUrl)));
  connect(text, SIGNAL(highlighted(QUrl)), this, SLOT(showAnchorDescription(QUrl)));
  gridLayoutX->addWidget (text, 0, 0, 1, 1);  

  QString aux(StrConstants::getTabOutputName());
  ui->twProperties->removeTab(ctn_TABINDEX_OUTPUT);
  ui->twProperties->insertTab(ctn_TABINDEX_OUTPUT, tabOutput, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0/*, QApplication::UnicodeUTF8*/ ) );

  SearchBar *searchBar = new SearchBar(this);
  connect(searchBar, SIGNAL(textChanged(QString)), this, SLOT(searchBarTextChangedInTextBrowser(QString)));
  connect(searchBar, SIGNAL(closed()), this, SLOT(searchBarClosedInTextBrowser()));
  connect(searchBar, SIGNAL(findNext()), this, SLOT(searchBarFindNextInTextBrowser()));
  connect(searchBar, SIGNAL(findPrevious()), this, SLOT(searchBarFindPreviousInTextBrowser()));
  gridLayoutX->addWidget(searchBar, 1, 0, 1, 1);

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  text->show();
  text->setFocus();
}

/*
 * Initialize QAction objects
 */
void MainWindow::initActions()
{
  m_hasSLocate = UnixCommand::hasTheExecutable("slocate");
  m_hasMirrorCheck = UnixCommand::hasTheExecutable(ctn_MIRROR_CHECK_APP);
  m_actionSysInfo = new QAction(this);
  m_actionMenuMirrorCheck = new QAction(this);
  m_actionMenuOptions = new QAction(this);
  m_actionPackageInfo = new QAction(this);
  m_actionPackageInfo->setText(StrConstants::getTabInfoName());

  m_actionAURVote = new QAction(this);
  m_actionAURVote->setText(StrConstants::getVote());
  m_actionAURVote->setIcon(IconHelper::getIconVote());
  connect(m_actionAURVote, SIGNAL(triggered()), this, SLOT(onAURVote()));

  m_actionAURUnvote = new QAction(this);
  m_actionAURUnvote->setText(StrConstants::getUnvote());
  m_actionAURUnvote->setIcon(IconHelper::getIconUnvote());
  connect(m_actionAURUnvote, SIGNAL(triggered()), this, SLOT(onAURUnvote()));

  if(m_hasMirrorCheck)
  {
    m_actionMenuMirrorCheck->setShortcut(QKeySequence(Qt::ControlModifier|Qt::ShiftModifier|Qt::Key_M));
    m_actionMenuMirrorCheck->setText("Mirror-Check");
    m_actionMenuMirrorCheck->setIcon(IconHelper::getIconMirrorCheck());
    connect(m_actionMenuMirrorCheck, SIGNAL(triggered()), this, SLOT(doMirrorCheck()));
  }  

  //ui->actionSyncPackages->setVisible(false);
  ui->actionOpenRootTerminal->setVisible(false);

  m_actionMenuOptions->setText(StrConstants::getOptions());
  connect(m_actionMenuOptions, SIGNAL(triggered()), this, SLOT(onOptions()));

  m_actionStopTransaction = new QAction(this);
  m_actionStopTransaction->setIcon(IconHelper::getIconStop());
  m_actionStopTransaction->setText(StrConstants::getStop());
  connect(m_actionStopTransaction, SIGNAL(triggered()), this, SLOT(stopTransaction()));

  m_actionSwitchToAURTool = new QAction(this);
  m_actionSwitchToAURTool->setIcon(IconHelper::getIconForeignGreen());
  if (SettingsManager::getAURTool() != ctn_NO_AUR_TOOL)
  {
    m_actionSwitchToAURTool->setToolTip(StrConstants::getUseAURTool());
    m_actionSwitchToAURTool->setCheckable(true);
    m_actionSwitchToAURTool->setChecked(false);
  }

  //m_actionSwitchToAURTool->setCheckable(true);
  //m_actionSwitchToAURTool->setChecked(false);
  m_actionSwitchToAURTool->setEnabled(false);
  connect(m_actionSwitchToAURTool, SIGNAL(triggered()), this, SLOT(AURToolSelected()));

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

  m_actionShowGroups = new QAction(this);
  m_actionShowGroups->setIcon(IconHelper::getIconShowGroups());
  m_actionShowGroups->setText(StrConstants::getGroups());
  m_actionShowGroups->setCheckable(true);
  m_actionShowGroups->setChecked(true);
  m_actionShowGroups->setShortcut(QKeySequence(Qt::Key_F9));
  connect(m_actionShowGroups, SIGNAL(triggered()), this, SLOT(hideGroupsWidget()));

  m_actionEditOctopiConf = new QAction(this);
  m_actionEditOctopiConf->setText("octopi.conf...");
  m_actionEditOctopiConf->setIcon(IconHelper::getIconBinary());
  connect(m_actionEditOctopiConf, SIGNAL(triggered()), this, SLOT(editOctopiConf()));

  m_actionCopyFullPath = new QAction(this);
  m_actionCopyFullPath->setText(StrConstants::getCopyFullPath());
  m_actionCopyFullPath->setIcon(IconHelper::getIconEditCopy());
  connect(m_actionCopyFullPath, SIGNAL(triggered()), this, SLOT(copyFullPathToClipboard()));

  QActionGroup *actionGroup = new QActionGroup(this);
  actionGroup->addAction(ui->actionSearchByDescription);
  actionGroup->addAction(ui->actionSearchByName);
  actionGroup->addAction(ui->actionSearchByFile);
  ui->actionSearchByName->setChecked(true);
  actionGroup->setExclusive(true);
  connect(actionGroup, SIGNAL(triggered(QAction*)), this, SLOT(tvPackagesSearchColumnChanged(QAction*)));

  ui->actionInstallLocalPackage->setIcon(IconHelper::getIconFolder());
  ui->actionOpenDirectory->setIcon(IconHelper::getIconFolder());

  connect(ui->actionUseInstantSearch, SIGNAL(triggered(bool)), this, SLOT(toggleInstantSearch()));
  connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(invalidateTabs()));
  connect(ui->actionInstallLocalPackage, SIGNAL(triggered()), this, SLOT(installLocalPackage()));
  connect(ui->actionRemoveTransactionItem, SIGNAL(triggered()), this, SLOT(onPressDelete()));
  connect(ui->actionRemoveTransactionItems, SIGNAL(triggered()), this, SLOT(onPressDelete()));
  connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
  connect(ui->actionCheckUpdates, SIGNAL(triggered()), this, SLOT(doCheckUpdates()));
  connect(ui->actionSystemUpgrade, SIGNAL(triggered()), this, SLOT(doSystemUpgrade()));
  connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(insertIntoRemovePackage()));
  connect(ui->actionInstall, SIGNAL(triggered()), this, SLOT(insertIntoInstallPackage()));
  connect(ui->actionInstallAUR, SIGNAL(triggered()), this, SLOT(doInstallAURPackage()));
  connect(ui->actionFindFileInPackage, SIGNAL(triggered()), this, SLOT(findFileInPackage()));
  connect(ui->actionRemoveGroup, SIGNAL(triggered()), this, SLOT(insertGroupIntoRemovePackage()));
  connect(ui->actionInstallGroup, SIGNAL(triggered()), this, SLOT(insertGroupIntoInstallPackage()));
  connect(ui->actionApply, SIGNAL(triggered()), this, SLOT(commitTransaction()));
  connect(ui->actionCancel, SIGNAL(triggered()), this, SLOT(cancelTransaction()));
  connect(ui->actionGetNews, SIGNAL(triggered()), this, SLOT(refreshDistroNews()));
  connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
  connect(ui->actionHelpUsage, SIGNAL(triggered()), this, SLOT(onHelpUsage()));
  connect(ui->actionHelpAbout, SIGNAL(triggered()), this, SLOT(onHelpAbout()));
  connect(ui->actionDonate, SIGNAL(triggered(bool)), this, SLOT(onHelpDonate()));
  connect(m_actionPackageInfo, SIGNAL(triggered()), this, SLOT(showPackageInfo()));

  // Actions from tvPkgFileList context menu
  connect(ui->actionCollapseAllItems, SIGNAL(triggered()), this, SLOT(collapseAllContentItems()));
  connect(ui->actionExpandAllItems, SIGNAL(triggered()), this, SLOT(expandAllContentItems()));
  connect(ui->actionCollapseItem, SIGNAL(triggered()), this, SLOT(collapseThisContentItems()));
  connect(ui->actionExpandItem, SIGNAL(triggered()), this, SLOT(expandThisContentItems()));
  connect(ui->actionOpenFile, SIGNAL(triggered()), this, SLOT(openFile()));
  connect(ui->actionEditFile, SIGNAL(triggered()), this, SLOT(editFile()));
  connect(ui->actionOpenDirectory, SIGNAL(triggered()), this, SLOT(openDirectory()));
  connect(ui->actionOpenTerminal, SIGNAL(triggered()), this, SLOT(openTerminal()));

  // Use theme icons for QActions
  ui->actionCheckUpdates->setIcon(IconHelper::getIconCheckUpdates());
  ui->actionApply->setIcon(IconHelper::getIconCommit());
  ui->actionCancel->setIcon(IconHelper::getIconRollback());
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
  //ui->actionOpenRootTerminal->setIcon(IconHelper::getIconTerminal());
  ui->actionInstallAUR->setIcon(IconHelper::getIconInstallItem());

  // Actions for the View menu
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
  ui->menuTools->menuAction()->setVisible(true);
  refreshMenuTools();

  if (WMHelper::isXFCERunning())
  {
    //Loop through all actions and set their icons (if any) visible to menus.
    foreach(QAction* ac, this->findChildren<QAction*>(QRegularExpression("(m_a|a)ction\\S*")))
    {
      if (ac) ac->setIconVisibleInMenu(true);
    }
  }

  QString text;
  foreach(QAction* ac, this->findChildren<QAction*>(QRegularExpression("(m_a|a)ction\\S*")))
  {
    //text = ac->text().remove("&");
    //ac->setText(qApp->translate("MainWindow", text.toUtf8(), 0));

    if (!ac->shortcut().isEmpty())
    {
      ac->setToolTip(ac->toolTip() + "  (" + ac->shortcut().toString() + ")");
    }
  }

  toggleTransactionActions(true);
}
