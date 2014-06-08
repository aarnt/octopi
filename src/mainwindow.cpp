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

#include "mainwindow.h"
#include "searchlineedit.h"
#include "ui_mainwindow.h"
#include "strconstants.h"
#include "package.h"
#include "uihelper.h"
#include "wmhelper.h"
#include "treeviewpackagesitemdelegate.h"
#include "searchbar.h"
#include "packagecontroller.h"
#include "globals.h"
#include <iostream>

#include <QStandardItemModel>
#include <QString>
#include <QTextBrowser>
#include <QKeyEvent>
#include <QTimer>
#include <QLabel>
#include <QMessageBox>
#include <QComboBox>
#include <QModelIndex>
#include <QDesktopServices>
#include <QFileDialog>
#include <QHash>
#include <QFutureWatcher>

/*
 * MainWindow's constructor: basic UI init
 */
MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent), ui(new Ui::MainWindow), m_packageModel(new PackageModel(m_packageRepo))
{
  m_packageRepo.registerDependency(*m_packageModel);
  m_foundFilesInPkgFileList = new QList<QModelIndex>();
  m_indFoundFilesInPkgFileList = 0;
  m_callSystemUpgrade = false;
  m_callSystemUpgradeNoConfirm = false;
  m_initializationCompleted=false;
  m_systemUpgradeDialog = false;
  m_cic = NULL;
  m_outdatedPackageList = new QStringList();
  m_outdatedYaourtPackageList = new QStringList();
  m_outdatedYaourtPackagesNameVersion = new QHash<QString, QString>();
  m_selectedViewOption = ectn_ALL_PKGS;
  m_selectedRepository = "";
  m_numberOfInstalledPackages = 0;
  ui->setupUi(this);
}

/*
 * MainWindow's destructor
 */
MainWindow::~MainWindow()
{
  //Let's garbage collect transaction files...
  m_unixCommand->removeTemporaryFiles();
  delete ui;
}

/*
 * The show() public SLOT, when the app is being drawn!!!
 * Init member variables and all UI widgets
 */
void MainWindow::show()
{
  if(m_initializationCompleted == false)
  {
    UnixCommand::getIgnorePkg();
    restoreGeometry(SettingsManager::getWindowSize());
    m_commandExecuting=ectn_NONE;
    m_commandQueued=ectn_NONE;
    m_leFilterPackage = new SearchLineEdit(this);

    setWindowTitle(StrConstants::getApplicationName());
    setMinimumSize(QSize(820, 520));

    initTabOutput();
    initTabInfo();
    initTabFiles();
    initTabTransaction();
    initTabHelpUsage();
    initTabNews();
    initLineEditFilterPackages();
    initPackageTreeView();
    loadSettings();

    loadPanelSettings();
    initActions();
    initStatusBar();
    initToolButtonPacman();
    initToolButtonYaourt();
    initAppIcon();
    initMenuBar();
    initToolBar();
    initTabWidgetPropertiesIndex();
    refreshDistroNews(false);
    refreshGroupsWidget();

    QMainWindow::show();

    metaBuildPackageList();
  }
  else
    QMainWindow::show();
}

/*
 * Retrieves a pointer to Output's QTextBrowser object
 */
QTextBrowser *MainWindow::_getOutputTextBrowser()
{
  QTextBrowser *ret=0;
  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");

  if (text)
  {
    ret = text;
  }

  return ret;
}

/*
 * This SLOT is called whenever user clicks a url inside output's textBrowser
 */
void MainWindow::outputTextBrowserAnchorClicked(const QUrl &link)
{
  if (link.toString().contains("goto:"))
  {
    QString pkgName = link.toString().mid(5);

    QModelIndex columnIndex = m_packageModel->index(0, PackageModel::ctn_PACKAGE_NAME_COLUMN,
                                                    QModelIndex());

    QModelIndexList foundItems = m_packageModel->match(columnIndex, Qt::DisplayRole, pkgName, -1, Qt::MatchExactly);
    QModelIndex proxyIndex;

    if (foundItems.count() == 1)
    {
      proxyIndex = foundItems.first();
      if(proxyIndex.isValid())
      {
        ui->tvPackages->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);
        ui->tvPackages->setCurrentIndex(proxyIndex);
        _changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
      }
    }
    if (foundItems.count() == 0 || !proxyIndex.isValid())
    {
      refreshTabInfo(pkgName);
      disconnect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
      _ensureTabVisible(ctn_TABINDEX_INFORMATION);
      connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
    }
  }
  else
  {
    QDesktopServices::openUrl(link);
  }
}

/*
 * Prints the list of outdated packages to the Output tab.
 */
void MainWindow::outputOutdatedPackageList()
{
  //We cannot output any list if there is a running transaction!
  if (m_commandExecuting != ectn_NONE ||
      isYaourtGroupSelected())
    return;

  m_numberOfOutdatedPackages = m_outdatedPackageList->count();

  if(m_numberOfOutdatedPackages > 0)
  {
    QString html = "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
    QString anchorBegin = "anchorBegin";
    html += "<a id=\"" + anchorBegin + "\"></a>";

    clearTabOutput();

    if(m_outdatedPackageList->count()==1){
      html += "<h3>" + StrConstants::getOneOutdatedPackage() + "</h3>";
    }
    else
    {
      html += "<h3>" +
          StrConstants::getOutdatedPackages().arg(m_outdatedPackageList->count()) + "</h3>";
    }

    html += "<br><table border=\"0\">";
    html += "<tr><th width=\"25%\" align=\"left\">" + StrConstants::getName() +
        "</th><th width=\"18%\" align=\"right\">" +
        StrConstants::getOutdatedVersion() +
        "</th><th width=\"18%\" align=\"right\">" +
        StrConstants::getAvailableVersion() + "</th></tr>";

    for (int c=0; c < m_outdatedPackageList->count(); c++)
    {
      QString pkg = m_outdatedPackageList->at(c);
      const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkg);
      if (package != NULL) {
        html += "<tr><td><a href=\"goto:" + pkg + "\">" + pkg +
            "</td><td align=\"right\"><b><font color=\"#E55451\">" +
            package->outdatedVersion +
            "</b></font></td><td align=\"right\">" +
            package->version + "</td></tr>";
      }
    }

    writeToTabOutput(html);

    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");

    if (text)
    {
      text->scrollToAnchor(anchorBegin);
    }

    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  }
}

/*
 * Prints the list of outdated Yaourt packages to the Output tab.
 */
void MainWindow::outputOutdatedYaourtPackageList()
{
  //We cannot output any list if there is a running transaction!
  if (m_commandExecuting != ectn_NONE) return;

  QString html = "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
  QString anchorBegin = "anchorBegin";
  html += "<a id=\"" + anchorBegin + "\"></a>";

  clearTabOutput();

  if(m_outdatedYaourtPackageList->count()==1){
    html += "<h3>" + StrConstants::getOneOutdatedPackage() + "</h3>";
  }
  else
  {
    html += "<h3>" +
        StrConstants::getOutdatedPackages().arg(m_outdatedYaourtPackageList->count()) + "</h3>";
  }

  html += "<br><table border=\"0\">";
  html += "<tr><th width=\"25%\" align=\"left\">" + StrConstants::getName() +
      "</th><th width=\"18%\" align=\"right\">" +
      StrConstants::getOutdatedVersion() +
      "</th><th width=\"18%\" align=\"right\">" +
      StrConstants::getAvailableVersion() + "</th></tr>";

  for (int c=0; c < m_outdatedYaourtPackageList->count(); c++)
  {
    QString pkg = m_outdatedYaourtPackageList->at(c);
    const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkg);
    if (package != NULL) {
      QString availableVersion = m_outdatedYaourtPackagesNameVersion->value(m_outdatedYaourtPackageList->at(c));
  
      html += "<tr><td><a href=\"goto:" + pkg + "\">" + pkg +
          "</td><td align=\"right\"><b><font color=\"#E55451\">" +
          package->version +
          "</b></font></td><td align=\"right\">" +
          availableVersion + "</td></tr>";
    }
  }

  writeToTabOutput(html);

  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");

  if (text)
  {
    text->scrollToAnchor(anchorBegin);
  }

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
}

/*
 * Removes all text inside the TabOutput editor
 */
void MainWindow::clearTabOutput()
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (text)
  {
    text->clear();
  }
}

/*
 * Retrieves the selected package group from the treeWidget
 */
QString MainWindow::getSelectedGroup()
{
  return ui->twGroups->currentItem()->text(0);
}

/*
 * Helper to analyse if <Display All Groups> is selected
 */
bool MainWindow::isAllGroupsSelected()
{
  QModelIndex index = ui->twGroups->currentIndex();
  QString group = ui->twGroups->model()->data(index).toString();

  return isAllGroups(group);
}

bool MainWindow::isAllGroups(const QString& group)
{
  return (group == "<" + StrConstants::getDisplayAllGroups() + ">");
}

/*
 * Helper to analyse if < Yaourt > is selected
 */
bool MainWindow::isYaourtGroupSelected()
{
  QModelIndex index = ui->twGroups->currentIndex();
  QString group = ui->twGroups->model()->data(index).toString();

  return (group == StrConstants::getForeignToolGroup());
}

const PackageRepository::PackageData* MainWindow::getFirstPackageFromRepo(const QString pkgName)
{
  return m_packageRepo.getFirstPackageByName(pkgName);
}

/*
 * Sets a flag to call the System Upgrade action as soom as it's possible
 */
void MainWindow::setCallSystemUpgrade()
{
  m_callSystemUpgrade = true;
}

/*
 * Sets a flag to call the System Upgrade action, without confirmation dialogs!
 */
void MainWindow::setCallSystemUpgradeNoConfirm()
{
  m_callSystemUpgradeNoConfirm = true;
}

/*
 * Sets a flag that holds the remove command to be used in transactions
 */
void MainWindow::setRemoveCommand(const QString &removeCommand)
{
  m_removeCommand = removeCommand;
  m_removeCommand.remove("=");
  m_removeCommand.remove("-");
}

/*
 * Searchs model modelInstalledPackages by a package name and returns if it is already installed
 */
bool MainWindow::isPackageInstalled(const QString &pkgName)
{
  const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkgName);
  return (package != NULL && package->installed());
}

/*
 * User changed the search column used by the SortFilterProxyModel in tvPackages
 */
void MainWindow::tvPackagesSearchColumnChanged(QAction *actionSelected)
{
  //We are in the realm of tradictional NAME search
  if (actionSelected->objectName() == ui->actionSearchByName->objectName())
  {
    m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_NAME_COLUMN);
  }
  //We are talking about slower 'search by description'...
  else
  {
    m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_DESCRIPTION_FILTER_NO_COLUMN);
  }

  QModelIndex mi = m_packageModel->index(0, PackageModel::ctn_PACKAGE_NAME_COLUMN, QModelIndex());
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  changedTabIndex();
  m_leFilterPackage->setFocus();
}

/*
 * Whenever the user selects a different item in View menu,
 * we have to change the model of the Packages treeview...
 */
void MainWindow::changePackageListModel(ViewOptions viewOptions, QString selectedRepo)
{  
  m_packageModel->applyFilter(viewOptions, selectedRepo, isAllGroupsSelected() ? "" : getSelectedGroup());

  if (m_leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex cIcon = m_packageModel->index(0, PackageModel::ctn_PACKAGE_ICON_COLUMN, QModelIndex());

  ui->tvPackages->setCurrentIndex(cIcon);
  ui->tvPackages->scrollTo(cIcon, QAbstractItemView::PositionAtTop);

  tvPackagesSelectionChanged(QItemSelection(),QItemSelection());
  changedTabIndex();
}

/*
 * Brings the context menu when the user clicks the right button above the package list
 */
void MainWindow::execContextMenuPackages(QPoint point)
{
/* The following SLOTs will be called:
 *   connect(ui->actionFindFileInPackage, SIGNAL(triggered()), this, SLOT(findFileInPackage()));
 *   connect(ui->actionInstall, SIGNAL(triggered()), this, SLOT(insertIntoInstallPackage()));
 *   connect(ui->actionInstallYaourt, SIGNAL(triggered()), this, SLOT(doInstallYaourtPackage()));
 *   connect(ui->actionInstallGroup, SIGNAL(triggered()), this, SLOT(insertGroupIntoInstallPackage()));
 *   connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(insertIntoRemovePackage()));
 *   connect(ui->actionRemoveGroup, SIGNAL(triggered()), this, SLOT(insertGroupIntoRemovePackage()));
 */
  const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (selectionModel != NULL && selectionModel->selectedRows().count() > 0)
  {
    QMenu* menu = new QMenu(this);
    QModelIndexList selectedRows = selectionModel->selectedRows();
    if (selectedRows.count() == 1) // enable entry "browse installed files" ?
    {
      QModelIndex item = selectedRows.at(0);
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      if (package && package->installed()) {
        menu->addAction(ui->actionFindFileInPackage);
        menu->addSeparator();
      }
    }

    bool allInstallable = true;
    bool allRemovable = true;    
    int numberOfSelPkgs = selectedRows.count();
    int numberOfAUR = 0;

    foreach(QModelIndex item, selectedRows)
    {
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);

      if (package->repository == StrConstants::getForeignRepositoryName())
      {
        allInstallable = false;
        numberOfAUR++;
      }
      if (package->installed() == false)
      {
        allRemovable = false;
      }
    }

    if (allInstallable) // implicitly foreign packages == 0
    {
      if (!isAllGroupsSelected() && !isYaourtGroupSelected()) menu->addAction(ui->actionInstallGroup);
      menu->addAction(ui->actionInstall);

      if (!isAllGroupsSelected() && !isYaourtGroupSelected()) //&& numberOfSelPkgs > 1)
      {
        //Is this group already installed?
        const QList<PackageRepository::PackageData*> packageList = m_packageRepo.getPackageList(getSelectedGroup());
        if (packageList.size() == numberOfSelPkgs)
        {
          //If we select all packages, let's subtract the install action...
          menu->removeAction(ui->actionInstall);
        }
      }
    }
    else if (allInstallable == false && numberOfAUR == numberOfSelPkgs)
    {
      menu->addAction(ui->actionInstallYaourt); // installs directly
    }

    if (allRemovable)
    {
      menu->addAction(ui->actionRemove);

      if (!isAllGroupsSelected() && !isYaourtGroupSelected())
      {
        //Is this group already installed?
        const QList<PackageRepository::PackageData*> packageList = m_packageRepo.getPackageList(getSelectedGroup());
        if (packageList.size() == numberOfSelPkgs)
        {
          //If we select all packages, let's subtract the remove action...
          menu->removeAction(ui->actionRemove);
          menu->addAction(ui->actionRemoveGroup);
        }
      }
    }

    if (menu->actions().count() > 0)
    {
      QPoint pt2 = ui->tvPackages->mapToGlobal(point);
      pt2.setY(pt2.y() + ui->tvPackages->header()->height());
      menu->exec(pt2);
    }
  }
}

/*
 * This SLOT collapses all treeview items
 */
void MainWindow::collapseAllContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if (tv != 0)
    tv->collapseAll();
}

/*
 * This SLOT collapses only the currently selected item
 */
void MainWindow::collapseThisContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if ( tv != 0 )
  {
    tv->repaint(tv->rect());
    QCoreApplication::processEvents();
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tv->model());
    QModelIndex mi = tv->currentIndex();
    if (sim->hasChildren(mi))	_collapseItem(tv, sim, mi);
  }
}

/*
 * This SLOT expands all treeview items
 */
void MainWindow::expandAllContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if ( tv != 0 )
  {
    tv->repaint(tv->rect());
    QCoreApplication::processEvents();
    tv->expandAll();
  }
}

/*
 * This SLOT expands only the currently selected item
 */
void MainWindow::expandThisContentItems(){
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if ( tv != 0 )
  {
    tv->repaint(tv->rect());
    QCoreApplication::processEvents();
    QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tv->model());
    QModelIndex mi = tv->currentIndex();
    if (sim->hasChildren(mi))	_expandItem(tv, sim, &mi);
  }
}

/*
 * This method does the job of collapsing the given item and its children
 */
void MainWindow::_collapseItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex mi){
  for (int i=0; i<sim->rowCount(mi); i++)
  {
    if (sim->hasChildren(mi))
    {
      QCoreApplication::processEvents();
      tv->collapse(mi);
      QModelIndex mi2 = mi.child(i, 0);
      _collapseItem(tv, sim, mi2);
    }
  }
}

/*
 * This method does the job of expanding the given item and its children
 */
void MainWindow::_expandItem(QTreeView* tv, QStandardItemModel* sim, QModelIndex* mi){
  for (int i=0; i<sim->rowCount(*mi); i++){
    if (sim->hasChildren(*mi)){
      tv->expand(*mi);
      QModelIndex mi2 = mi->child(i, 0);
      _expandItem(tv, sim, &mi2);
    }
  }
}

/*
 * Brings the context menu when the user clicks the right button
 * above the Files treeview in "Files" Tab
 */
void MainWindow::execContextMenuPkgFileList(QPoint point)
{
  QTreeView *tvPkgFileList =
      ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");

  if (tvPkgFileList == 0)
  {
    return;
  }

  QModelIndex mi = tvPkgFileList->currentIndex();
  QString selectedPath = PackageController::showFullPathOfItem(mi);
  QMenu menu(this);
  QStandardItemModel *sim = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
  QStandardItem *si = sim->itemFromIndex(mi);
  
  if (si == 0) return;
  if (si->hasChildren() && (!tvPkgFileList->isExpanded(mi)))
    menu.addAction(ui->actionExpandItem);

  if (si->hasChildren() && (tvPkgFileList->isExpanded(mi)))
    menu.addAction(ui->actionCollapseItem);

  if (menu.actions().count() > 0)
    menu.addSeparator();

  menu.addAction(ui->actionCollapseAllItems);
  menu.addAction(ui->actionExpandAllItems);
  menu.addSeparator();

  QDir d;
  QFile f(selectedPath);

  if (si->icon().pixmap(QSize(22,22)).toImage() ==
      IconHelper::getIconFolder().pixmap(QSize(22,22)).toImage())
  {
    if (d.exists(selectedPath))
    {
      menu.addAction(ui->actionOpenDirectory);
      menu.addAction(ui->actionOpenTerminal);
    }
  }
  else if (f.exists())
  {
    menu.addAction(ui->actionOpenFile);
  }
  if (!UnixCommand::isRootRunning() && f.exists() && UnixCommand::isTextFile(selectedPath))
  {
    menu.addAction(ui->actionEditFile);
  }

  QPoint pt2 = tvPkgFileList->mapToGlobal(point);
  pt2.setY(pt2.y() + tvPkgFileList->header()->height());
  menu.exec(pt2);
}

/*
 * Brings the context menu when the user clicks the right button
 * above the Transaction treeview in "Transaction" Tab
 */
void MainWindow::execContextMenuTransaction(QPoint point)
{
  QTreeView *tvTransaction = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvTransaction");
  if (!tvTransaction) return;

  if ((tvTransaction->currentIndex() == getRemoveTransactionParentItem()->index() &&
      tvTransaction->model()->hasChildren(tvTransaction->currentIndex())) ||
      (tvTransaction->currentIndex() == getInstallTransactionParentItem()->index() &&
            tvTransaction->model()->hasChildren(tvTransaction->currentIndex())))
  {
    QMenu menu(this);
    menu.addAction(ui->actionRemoveTransactionItems);
    QPoint pt2 = tvTransaction->mapToGlobal(point);
    pt2.setY(pt2.y() + tvTransaction->header()->height());
    menu.exec(pt2);
  }
  else if (tvTransaction->currentIndex() != getRemoveTransactionParentItem()->index() &&
           tvTransaction->currentIndex() != getInstallTransactionParentItem()->index() &&
           tvTransaction->currentIndex().isValid())
  {
    QMenu menu(this);

    if (tvTransaction->selectionModel()->selectedIndexes().count() == 1)
    {
      ui->actionRemoveTransactionItem->setText(StrConstants::getRemoveItem());
    }
    else
    {
      ui->actionRemoveTransactionItem->setText(StrConstants::getRemoveItems());
    }

    menu.addAction(ui->actionRemoveTransactionItem);
    QPoint pt2 = tvTransaction->mapToGlobal(point);
    pt2.setY(pt2.y() + tvTransaction->header()->height());
    menu.exec(pt2);
  }
}

/*
 * Returns true if tabWidget height is greater than 0. Otherwise, returns false.
 */
bool MainWindow::_isPropertiesTabWidgetVisible()
{
  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

  return (rl[1] > 0);
}

/*
 * Returns true if tvPackages height is greater than 0. Otherwise, returns false.
 */
bool MainWindow::_isPackageTreeViewVisible()
{
  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

  return (rl[0] > 0);
}

/*
 * Selects the very first item in the tvPkgFileList treeview
 */
void MainWindow::_selectFirstItemOfPkgFileList()
{
  QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
  if(tvPkgFileList)
  {
    tvPkgFileList->setFocus();
    QModelIndex maux = tvPkgFileList->model()->index(0, 0);
    tvPkgFileList->setCurrentIndex(maux);
  }
}

/*
 * Searches for the SearchBar at the tab Files. If it is found, closes it.
 */
void MainWindow::_closeTabFilesSearchBar()
{
  SearchBar *searchBar = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<SearchBar*>("searchbar");
  if (searchBar)
  {
    if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    {
      searchBar->close();
    }
  }
}

/*
 * Extracts the base file name from an absolute file name.
 */
QString MainWindow::_extractBaseFileName(const QString &fileName)
{
  QString baseFileName(fileName);

  if (fileName.endsWith('/'))
  {
    baseFileName.remove(baseFileName.size()-1, 1);
  }

  return baseFileName.right(baseFileName.size() - baseFileName.lastIndexOf('/') -1);
}

/*
 * Whenever user double clicks the package list items, app shows the contents of the selected package
 */
void MainWindow::onDoubleClickPackageList()
{
  const PackageRepository::PackageData* package = m_packageModel->getData(ui->tvPackages->currentIndex());
  if (package != NULL && package->installed())
  {
    refreshTabFiles(false, true);
  }
  else
  {
    refreshTabInfo(false, true);
  }
}

/*
 * When the user changes the current selected tab, we must take care of data refresh.
 */
void MainWindow::changedTabIndex()
{
  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION)
    refreshTabInfo();
  else if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    refreshTabFiles();

  if(m_initializationCompleted)
    saveSettings(ectn_CurrentTabIndex);
}

/*
 * Clears the information showed on the current tab (Info or Files).
 */
void MainWindow::invalidateTabs()
{
  if(ui->twProperties->currentIndex() == ctn_TABINDEX_INFORMATION) //This is TabInfo
  {
    refreshTabInfo(true);
    return;
  }
  else if(ui->twProperties->currentIndex() == ctn_TABINDEX_FILES) //This is TabFiles
  {
    refreshTabFiles(true);
    return;
  }
}

/*
 * Shows or hides the group's widget
 */
void MainWindow::hideGroupsWidget(bool pSaveSettings)
{
  static int tvPackagesWidth = ui->tvPackages->width();

  QList<int> l, rl;
  rl = ui->splitterVertical->sizes();

  if ( rl[1] != 0 )
  {
    ui->splitterVertical->setSizes( l << tvPackagesWidth << 0);

    if(pSaveSettings)
      saveSettings(ectn_GROUPS);
  }
  else
  {
    //First we test if the UI doesnt have the lower pane maximized
    QList<int> splitterHoriz;
    splitterHoriz = ui->splitterHorizontal->sizes();
    if ( splitterHoriz[1] != 0 )
    {
      QList<int> savedSizes;
      savedSizes << 200 << 235;
      ui->splitterHorizontal->setSizes(savedSizes);
    }

    ui->splitterVertical->setSizes( l << tvPackagesWidth << ui->twGroups->maximumWidth() );
    ui->tvPackages->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_NORMAL);
  }
}

/*
 * Maximizes/de-maximizes the upper pane (tvPackages)
 */
void MainWindow::maximizePackagesTreeView(bool pSaveSettings)
{
  QList<int> savedSizes;
  savedSizes << 200 << 235;

  QList<int> l, rl;
  rl = ui->splitterHorizontal->sizes();

  if ( rl[1] != 0 )
  {
    ui->splitterHorizontal->setSizes( l << ui->tvPackages->maximumHeight() << 0);
    if(!ui->tvPackages->hasFocus())
      ui->tvPackages->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_MAXIMIZE_PACKAGES);
  }
  else
  {
    ui->splitterHorizontal->setSizes(savedSizes);
    ui->tvPackages->scrollTo(ui->tvPackages->currentIndex());
    ui->tvPackages->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_NORMAL);
  }
}

/*
 * Maximizes/de-maximizes the lower pane (tabwidget)
 */
void MainWindow::maximizePropertiesTabWidget(bool pSaveSettings)
{
  QList<int> savedSizes;
  savedSizes << 200 << 235;

  QList<int> l, rl;
  rl = ui->splitterHorizontal->sizes();

  if ( rl[0] != 0 )
  {
    ui->splitterHorizontal->setSizes( l << 0 << ui->twProperties->maximumHeight());
    ui->twProperties->currentWidget()->childAt(1,1)->setFocus();

    if(pSaveSettings)
      saveSettings(ectn_MAXIMIZE_PROPERTIES);
  }
  else
  {
    ui->splitterHorizontal->setSizes(savedSizes);
    ui->tvPackages->scrollTo(ui->tvPackages->currentIndex());
    ui->tvPackages->setFocus();

    if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
    {
      QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
      if (tv)
        tv->scrollTo(tv->currentIndex());
    }

    if(pSaveSettings)
      saveSettings(ectn_NORMAL);
  }
}

/*
 * Whenever a user clicks on the Sort indicator of the package treeview, we keep values to mantain his choices
 */
void MainWindow::headerViewPackageListSortIndicatorClicked( int col, Qt::SortOrder order )
{
  // prevent infinite loop
  disconnect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
             SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));
  // sort
  ui->tvPackages->sortByColumn(col, order);

  // reconnect
  connect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
          SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));

  saveSettings(ectn_PackageList);
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void MainWindow::_positionTextEditCursorAtEnd()
{
  QTextBrowser *textEdit =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");

  if (textEdit)
  {
    QTextCursor tc = textEdit->textCursor();
    tc.clearSelection();
    tc.movePosition(QTextCursor::End);
    textEdit->setTextCursor(tc);
  }
}

/*
 * Ensures the given index tab is visible
 */
void MainWindow::_ensureTabVisible(const int index)
{
  QList<int> rl = ui->splitterHorizontal->sizes();

  if(rl[1] <= 50)
  {
    rl.clear();
    rl << 200 << 235;
    ui->splitterHorizontal->setSizes(rl);

    saveSettings(ectn_NORMAL);
  }

  ui->twProperties->setCurrentIndex(index);
}

/*
 * Helper method to find the given "findText" in the Output TextEdit
 */
bool MainWindow::_textInTabOutput(const QString& findText)
{
  bool res;
  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (text)
  {
    _positionTextEditCursorAtEnd();
    res = text->find(findText, QTextDocument::FindBackward | QTextDocument::FindWholeWords);
    _positionTextEditCursorAtEnd();
  }

  return res;
}

/*
 * Helper method to find the "Synching repo..." strings
 */
bool MainWindow::_IsSyncingRepoInTabOutput()
{
  bool res;
  QTextBrowser *text =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (text)
  {
    _positionTextEditCursorAtEnd();
    //We have to find at least two times, as "Synching" string will always be the first text in output
    res = text->find(StrConstants::getSyncing(), QTextDocument::FindBackward | QTextDocument::FindWholeWords);
    res = text->find(StrConstants::getSyncing(), QTextDocument::FindBackward | QTextDocument::FindWholeWords);

    if (!res)
    {
      _positionTextEditCursorAtEnd();
      res = text->find("downloading", QTextDocument::FindBackward | QTextDocument::FindWholeWords);
    }

    _positionTextEditCursorAtEnd();
  }

  return res;
}

/*
 * Helper method that opens an existing file using the available program/DE.
 */
void MainWindow::openFile()
{
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if (tv)
  {
    QString path = PackageController::showFullPathOfItem(tv->currentIndex());
    QFileInfo file(path);
    if (file.isFile())
    {
      WMHelper::openFile(path);
    }
  }
}

/*
 * Helper method that edits an existing file using the available program/DE.
 */
void MainWindow::editFile()
{
  QTreeView *tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
  if (tv)
  {
    QString path = PackageController::showFullPathOfItem(tv->currentIndex());
    WMHelper::editFile(path);
  }
}

/*
 * Helper method that opens a terminal in the selected directory, using the available program/DE.
 */
void MainWindow::openTerminal()
{
  QString dir = getSelectedDirectory();
  if (!dir.isEmpty())
  {
    WMHelper::openTerminal(dir);
  }
}

/*
 * Helper method that opens an existing directory using the available program/DE.
 */
void MainWindow::openDirectory(){
  QString dir = getSelectedDirectory();
  if (!dir.isEmpty())
  {
    WMHelper::openDirectory(dir);
  }
}

/*
 * Helper method which opens a root terminal
 */
void MainWindow::openRootTerminal()
{
  //If there are no means to run the actions, we must warn!
  if (!_isSUAvailable()) return;

  m_unixCommand->openRootTerminal();
}

/*
 * Open a file chooser dialog for the user to select local packages to install (pacman -U)
 */
void MainWindow::installLocalPackage()
{
  if (!_isSUAvailable()) return;

  m_packagesToInstallList =
      QFileDialog::getOpenFileNames(this,
                                    StrConstants::getFileChooserTitle(),
                                    QDir::homePath(),
                                    StrConstants::getPackages() + " (*.pkg.tar*)");

  if (m_packagesToInstallList.count() > 0)
    doInstallLocalPackages();
}

/*
 * Brings the user to the tab Files and position cursor inside searchBar
 * so he can find any file the selected package may have
 */
void MainWindow::findFileInPackage()
{
  refreshTabFiles(false, true);

  QTreeView *tb = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
  SearchBar *searchBar = ui->twProperties->currentWidget()->findChild<SearchBar*>("searchbar");

  if (tb && tb->model()->rowCount() > 0 && searchBar)
  {
    if (searchBar)
    {
      searchBar->clear();
      searchBar->show();
    }
  }
}

/*
 * Returns the current selected directory of the FileList treeview in FilesTab
 * In case nothing is selected, returns an empty string
 */
QString MainWindow::getSelectedDirectory()
{
  QString targetDir;

  if (_isPropertiesTabWidgetVisible() && ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
  {
    QTreeView *t = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvPkgFileList");
    if(t && t->currentIndex().isValid())
    {
      QString itemPath = PackageController::showFullPathOfItem(t->currentIndex());
      QFileInfo fi(itemPath);

      if (fi.isDir())
        targetDir = itemPath;
      else targetDir = fi.path();
    }
  }

  return targetDir;
}

/*
 * Changes the number of selected items in tvPackages: YY in XX(YY) Packages
 */
void MainWindow::tvPackagesSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  const QItemSelectionModel*const selection = ui->tvPackages->selectionModel();
  const int selected = selection != NULL ? selection->selectedRows().count() : 0;

  QString newMessage = StrConstants::getSelectedPackages().arg(
        QString::number(m_packageModel->getPackageCount())).
      arg(QString::number(selected));

  QString text;
  int numberOfInstalledPackages = m_packageModel->getInstalledPackagesCount();

  if(numberOfInstalledPackages > 0)
  {
    text = "| " + StrConstants::getNumberInstalledPackages().arg(numberOfInstalledPackages);
  }
  else if (m_leFilterPackage->text().isEmpty() && !m_packageModel->isFiltered())
  {
    text = "| " + StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages);
  }
  else
  {
    text = "| " + StrConstants::getNumberInstalledPackages().arg(0);
  }

  m_lblTotalCounters->setText(text);
  m_lblSelCounter->setText(newMessage);
}

/*
 *  Launch Pacman Log Viewer
 */
void MainWindow::launchPLV()
{
  QProcess *proc = new QProcess();
  proc->startDetached("plv");
}

/*
 * Launch Repo Editor
 */
void MainWindow::launchRepoEditor()
{
  m_unixCommand = new UnixCommand(this);
  m_unixCommand->executeCommand(QLatin1String("octopi-repoeditor"));
}
