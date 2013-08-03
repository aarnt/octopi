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
#include <iostream>

#include <QSortFilterProxyModel>
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

/*
 * MainWindow's constructor: basic UI init
 */
MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  m_foundFilesInPkgFileList = new QList<QModelIndex>();
  m_indFoundFilesInPkgFileList = 0;
  m_callSystemUpgrade = false;
  m_initializationCompleted=false;
  m_cbGroups = 0;
  m_listOfPackages = 0;
  m_listOfPackagesFromGroup = 0;

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
    loadSettings();
    restoreGeometry(SettingsManager::getWindowSize());

    m_commandExecuting=ectn_NONE;
    m_commandQueued=ectn_NONE;
    m_leFilterPackage = new SearchLineEdit(this);

    setWindowTitle(StrConstants::getApplicationName());
    setMinimumSize(QSize(850, 600));

    if (UnixCommand::getLinuxDistro() == ectn_MANJAROLINUX &&
        !WMHelper::isKDERunning())
    {
      //If we are in Manjaro and not in KDE, let's use a GTK style!
      if (!qApp->style()->inherits("QGtkStyle"))
      {
        qApp->setStyle(new QGtkStyle());
        setCleanLooksStyle(false);
      }
    }

    initStatusBar();
    initTabOutput();
    initTabInfo();
    initTabFiles();
    initTabTransaction();
    initTabHelpUsage();
    initTabNews();
    initLineEditFilterPackages();
    initPackageTreeView();

    qApp->setStyleSheet(StrConstants::getMenuCSS());

    loadPanelSettings();
    initActions();
    initAppIcon();
    initToolBar();
    initTabWidgetPropertiesIndex();
    refreshDistroNews(false);

    refreshComboBoxGroups();
    QMainWindow::show();

    metaBuildPackageList();

  }
  else
    QMainWindow::show();
}

/*
 * Whenever this timer ticks, we need to call the PacmanHelper DBus interface to sync Pacman's dbs
 */
void MainWindow::pacmanHelperTimerTimeout()
{
  m_pacmanHelperTimer->setInterval(1000 * 60 * 60 * 1); //the last number stand for hours

  //If Octopi is executing another task, let it finish it first!
  if(m_commandExecuting != ectn_NONE)
  {
    return;
  }

  /*disconnect(m_pacmanDatabaseSystemWatcher,
             SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  PacmanHelperClient *client =
      new PacmanHelperClient("org.octopi.pacmanhelper", "/", QDBusConnection::systemBus(), 0);
  QObject::connect(client, SIGNAL(syncdbcompleted()), this, SLOT(afterPacmanHelperSyncDatabase()));
  client->syncdb();*/
}

/*
 * Called right after the PacmanHelper syncdb() method has finished!
 */
void MainWindow::afterPacmanHelperSyncDatabase()
{
  //If Octopi is executing another task, let it finish it first!
  if(m_commandExecuting != ectn_NONE)
  {
    return;
  }

  //connect(m_pacmanDatabaseSystemWatcher,
  //        SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  refreshDistroNews();

  int numberOfOutdatedPackages = m_numberOfOutdatedPackages;
  m_outdatedPackageList = Package::getOutdatedPackageList();
  m_numberOfOutdatedPackages = m_outdatedPackageList->count();

  if (numberOfOutdatedPackages != m_numberOfOutdatedPackages){
    metaBuildPackageList();

    if (m_numberOfOutdatedPackages > 0)
    {
      if (m_numberOfOutdatedPackages == 1)
      {
        m_systemTrayIcon->setToolTip(StrConstants::getOneNewUpdate());
      }
      else if (m_numberOfOutdatedPackages > 1)
      {
        m_systemTrayIcon->setToolTip(StrConstants::getNewUpdates().arg(m_numberOfOutdatedPackages));
      }
    }
  }
  else
  {
    if (numberOfOutdatedPackages == 1)
    {
      m_systemTrayIcon->setToolTip(StrConstants::getOneNewUpdate());
    }
    else if (numberOfOutdatedPackages > 1)
    {
      m_systemTrayIcon->setToolTip(StrConstants::getNewUpdates().arg(numberOfOutdatedPackages));
    }
  }
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

    QStandardItemModel * sim = _getCurrentSelectedModel();
    QList<QStandardItem*> foundItems =
        sim->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);

    if (foundItems.count() > 0)
    {
      QStandardItem * si = foundItems.first();
      QModelIndex indexIcon = sim->index(si->row(), ctn_PACKAGE_ICON_COLUMN);
      QModelIndex proxyIndex = m_proxyModelPackages->mapFromSource(indexIcon);
      ui->tvPackages->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);
      ui->tvPackages->setCurrentIndex(proxyIndex);
      _changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
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
  if (m_commandExecuting != ectn_NONE) return;

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
      QString outdatedVersion = getOutdatedPackageVersionByName(m_outdatedPackageList->at(c));
      QString availableVersion = getInstalledPackageVersionByName(m_outdatedPackageList->at(c));

      html += "<tr><td><a href=\"goto:" + pkg + "\">" + pkg +
          "</td><td align=\"right\"><b><font color=\"#E55451\">" +
          outdatedVersion +
          "</b></font></td><td align=\"right\">" +
          availableVersion + "</td></tr>";
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
 * Searchs model modelInstalledPackages by a package name and returns its OUTDATED version
 */
QString MainWindow::getOutdatedPackageVersionByName(const QString &pkgName)
{
  QList<QStandardItem *> foundItems =
      m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);
  QString res;

  if (foundItems.count() > 0)
  {
    QStandardItem * si = foundItems.first();
    QStandardItem * siIcon = m_modelInstalledPackages->item(si->row(), ctn_PACKAGE_ICON_COLUMN);

    int mark = siIcon->text().indexOf('^');
    if (mark >= 0)
    {
      res = siIcon->text().right(siIcon->text().size()-mark-1);
    }
  }

  return res;
}

/*
 * Searchs model modelInstalledPackages by a package name and returns its AVAILABLE version
 */
QString MainWindow::getInstalledPackageVersionByName(const QString &pkgName)
{
  QList<QStandardItem *> foundItems =
      m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);
  QString res;

  if (foundItems.count() > 0)
  {
    QStandardItem * si = foundItems.first();
    QStandardItem * aux = m_modelInstalledPackages->item(si->row(), ctn_PACKAGE_VERSION_COLUMN);
    res = aux->text();
  }

  return res;
}

/*
 * Returns the QStandardItem available in the row that pkgName is found
 */
QStandardItem *MainWindow::getAvailablePackage(const QString &pkgName, const int index)
{
  QStandardItemModel *sim;

  if (m_cbGroups->currentText() == StrConstants::getYaourtGroup()
      && m_modelPackages &&
      m_modelPackages->rowCount() > 0)
  {
    sim = m_modelPackages;
  }
  else if (m_modelPackagesClone && m_modelPackagesClone->rowCount() > 0)
  {
    sim = m_modelPackagesClone;
  }
  else return 0;

  QList<QStandardItem *> foundItems =
      sim->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);
  QStandardItem *res=0;

  if (foundItems.count() > 0)
  {
    QStandardItem * si = foundItems.first();
    QStandardItem * aux = sim->item(si->row(), index);
    res = aux->clone();
  }

  return res;
}

/*
 * Sets a flag to call the System Upgrade action as soom as it's possible
 */
void MainWindow::setCallSystemUpgrade()
{
  m_callSystemUpgrade = true;
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
  QList<QStandardItem *> foundItems =
      m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);

  return (foundItems.count() > 0);
}

/*
 * User changed the search column used by the SortFilterProxyModel in tvPackages
 */
void MainWindow::tvPackagesSearchColumnChanged(QAction *actionSelected)
{
  //We are in the realm of tradictional NAME search
  if (actionSelected->objectName() == ui->actionSearchByName->objectName())
  {
    m_proxyModelPackages->setFilterKeyColumn(ctn_PACKAGE_NAME_COLUMN);
  }
  //We are talking about slower 'search by description'...
  else
  {
    m_proxyModelPackages->setFilterKeyColumn(ctn_PACKAGE_DESCRIPTION_COLUMN);
  }

  QModelIndex cIcon = m_proxyModelPackages->index(0, ctn_PACKAGE_ICON_COLUMN);
  QModelIndex cName = m_proxyModelPackages->index(0, ctn_PACKAGE_NAME_COLUMN);
  QModelIndex cVersion = m_proxyModelPackages->index(0, ctn_PACKAGE_VERSION_COLUMN);
  QModelIndex cRepository = m_proxyModelPackages->index(0, ctn_PACKAGE_REPOSITORY_COLUMN);

  ui->tvPackages->setCurrentIndex(cIcon);

  ui->tvPackages->scrollTo(cIcon, QAbstractItemView::PositionAtTop);

  if (m_leFilterPackage->text() == "")
  {
    ui->tvPackages->selectionModel()->setCurrentIndex(cIcon, QItemSelectionModel::Select);
  }
  else
  {
    ui->tvPackages->selectionModel()->setCurrentIndex(cIcon, QItemSelectionModel::SelectCurrent);
    ui->tvPackages->selectionModel()->setCurrentIndex(cName, QItemSelectionModel::Select);
    ui->tvPackages->selectionModel()->setCurrentIndex(cVersion, QItemSelectionModel::Select);
    ui->tvPackages->selectionModel()->setCurrentIndex(cRepository, QItemSelectionModel::Select);
  }

  QModelIndex mi = m_proxyModelPackages->index(0, ctn_PACKAGE_NAME_COLUMN);
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  changedTabIndex();
  m_leFilterPackage->setFocus();
}

/*
 * Whenever the user changes the checkbox menu to show non installed packages,
 * we have to change the model from the Packages treeview...
 */
void MainWindow::changePackageListModel()
{
  QStringList sl;

  if (ui->actionNonInstalledPackages->isChecked())
  {
    if(m_cbGroups->currentIndex() == 0 || m_cbGroups->currentText() == StrConstants::getYaourtGroup())
    {
      m_modelPackages->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());     
      m_proxyModelPackages->setSourceModel(m_modelPackages);
    }
    else
    {
      m_modelPackagesFromGroup->setHorizontalHeaderLabels(
            sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());
      m_proxyModelPackages->setSourceModel(m_modelPackagesFromGroup);
    }
  }
  else
  {
    if(m_cbGroups->currentIndex() == 0 || m_cbGroups->currentText() == StrConstants::getYaourtGroup())
    {
      m_modelInstalledPackages->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());
      m_proxyModelPackages->setSourceModel(m_modelInstalledPackages);
    }
    else
    {
      m_modelInstalledPackagesFromGroup->setHorizontalHeaderLabels(
            sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());
      m_proxyModelPackages->setSourceModel(m_modelInstalledPackagesFromGroup);
    }
  }

  if (m_leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex cIcon = m_proxyModelPackages->index(0, ctn_PACKAGE_ICON_COLUMN);
  QModelIndex cName = m_proxyModelPackages->index(0, ctn_PACKAGE_NAME_COLUMN);
  QModelIndex cVersion = m_proxyModelPackages->index(0, ctn_PACKAGE_VERSION_COLUMN);
  QModelIndex cRepository = m_proxyModelPackages->index(0, ctn_PACKAGE_REPOSITORY_COLUMN);

  ui->tvPackages->setCurrentIndex(cIcon);

  ui->tvPackages->scrollTo(cIcon, QAbstractItemView::PositionAtTop);

  if (m_leFilterPackage->text() == "")
  {
    ui->tvPackages->selectionModel()->setCurrentIndex(cIcon, QItemSelectionModel::Select);
  }
  else
  {
    ui->tvPackages->selectionModel()->setCurrentIndex(cIcon, QItemSelectionModel::SelectCurrent);
    ui->tvPackages->selectionModel()->setCurrentIndex(cName, QItemSelectionModel::Select);
    ui->tvPackages->selectionModel()->setCurrentIndex(cVersion, QItemSelectionModel::Select);
    ui->tvPackages->selectionModel()->setCurrentIndex(cRepository, QItemSelectionModel::Select);
  }

  changedTabIndex();
}

/*
 * Returns the current selected StandardItemModel, based on groups and installed packages switches
 */
QStandardItemModel *MainWindow::_getCurrentSelectedModel()
{
  QStandardItemModel *sim=0;

  if(ui->actionNonInstalledPackages->isChecked())
  {
    if(m_cbGroups->currentIndex() == 0 || m_cbGroups->currentText() == StrConstants::getYaourtGroup())
      sim = m_modelPackages;
    else
      sim = m_modelPackagesFromGroup;
  }
  else
  {
    if(m_cbGroups->currentIndex() == 0 || m_cbGroups->currentText() == StrConstants::getYaourtGroup())
      sim = m_modelInstalledPackages;
    else
      sim = m_modelInstalledPackagesFromGroup;
  }

  return sim;
}

/*
 * Brings the context menu when the user clicks the right button above the package list
 */
void MainWindow::execContextMenuPackages(QPoint point)
{
  if(ui->tvPackages->selectionModel()->selectedRows().count() > 0)
  {
    QStandardItemModel * sim = _getCurrentSelectedModel();
    QMenu *menu = new QMenu(this);

    if (ui->tvPackages->selectionModel()->selectedRows().count() == 1)
    {
      QModelIndex item = ui->tvPackages->selectionModel()->selectedRows().at(0);
      QModelIndex mi = m_proxyModelPackages->mapToSource(item);
      QStandardItem *si = sim->item(mi.row(), ctn_PACKAGE_ICON_COLUMN);

      if ((si->icon().pixmap(QSize(22,22)).toImage()) !=
          IconHelper::getIconNonInstalled().pixmap(QSize(22,22)).toImage())
      {
        menu->addAction(ui->actionFindFileInPackage);
        menu->addSeparator();
      }
    }

    bool allSameType = true;
    bool allInstallable = true;
    bool allRemovable = true;

    foreach(QModelIndex item, ui->tvPackages->selectionModel()->selectedRows())
    {
      QModelIndex mi = m_proxyModelPackages->mapToSource(item);
      QStandardItem *si = sim->item(mi.row(), ctn_PACKAGE_ICON_COLUMN);

      if((si->icon().pixmap(QSize(22,22)).toImage()) ==
         IconHelper::getIconForeign().pixmap(QSize(22,22)).toImage())
      {
        allInstallable = false;
      }
      else if((si->icon().pixmap(QSize(22,22)).toImage()) ==
              IconHelper::getIconNonInstalled().pixmap(QSize(22,22)).toImage())
      {
        allRemovable = false;
      }
    }

    if (allSameType)
    {
      if(allInstallable)
      {
        menu->addAction(ui->actionInstall);

        if (m_cbGroups->currentIndex() != 0 && m_cbGroups->currentText() != StrConstants::getYaourtGroup())
        {
          menu->addAction(ui->actionInstallGroup);
        }
      }

      if(allRemovable)
      {
        menu->addAction(ui->actionRemove);

        if (m_cbGroups->currentIndex() != 0 && m_cbGroups->currentText() != StrConstants::getYaourtGroup())
        {
          //Is this group already installed?
          if (m_modelInstalledPackagesFromGroup->rowCount() == m_modelPackagesFromGroup->rowCount())
          {
            menu->addAction(ui->actionRemoveGroup);
          }
        }
      }
    }

    if(menu->actions().count() > 0)
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
  if ( tv != 0 ){
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
  if ( tv != 0 ){
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
  if ( tv != 0 ){
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
  for (int i=0; i<sim->rowCount(mi); i++){
    if (sim->hasChildren(mi)){
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
 * Whenever user double clicks the package list items, app shows the contents of the selected package
 */
void MainWindow::onDoubleClickPackageList()
{
  QStandardItemModel *sim=_getCurrentSelectedModel();
  QModelIndex mi = m_proxyModelPackages->mapToSource(ui->tvPackages->currentIndex());
  QStandardItem *siName = sim->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);

  if (isPackageInstalled(siName->text()))
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
  m_PackageListOrderedCol = col;
  m_PackageListSortOrder = order;

  saveSettings(ectn_PackageList);
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void MainWindow::_positionTextEditCursorAtEnd()
{
  QTextBrowser *textEdit =
      ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (textEdit){
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
  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

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
  m_unixCommand->openRootTerminal();
}

/*
 * Open a file chooser dialog for the user to select local packages to install (pacman -U)
 */
void MainWindow::installLocalPackage()
{
  m_packagesToInstallList =
      QFileDialog::getOpenFileNames(this, StrConstants::getFileChooserTitle(),
                                         QDir::homePath(), "*pkg.tar.xz");

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
  QString newMessage = StrConstants::getSelectedPackages().arg(
        QString::number(m_proxyModelPackages->rowCount())).
      arg(QString::number(ui->tvPackages->selectionModel()->selectedRows().count()));

  m_lblSelCounter->setText(newMessage);
}
