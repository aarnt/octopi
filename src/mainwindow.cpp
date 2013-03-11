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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "strconstants.h"
#include "package.h"
#include "uihelper.h"
#include "wmhelper.h"
#include "treeviewpackagesitemdelegate.h"

#include <QDebug>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QString>
#include <QTextBrowser>
#include <QKeyEvent>
#include <QProgressDialog>
#include <QTimer>
#include <QLabel>
#include <QMessageBox>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  m_PackageListOrderedCol=1;
  m_PackageListSortOrder=Qt::AscendingOrder;
  m_currentTarget=0;
  m_commandExecuting=ectn_NONE;
  m_commandQueued=ectn_NONE;

  setWindowTitle(StrConstants::getApplicationName());
  setMinimumSize(QSize(850, 600));

  initStatusBar();
  initTabOutput();
  initTabInfo();
  initTabFiles();
  initTabTransaction();
  initLineEditFilterPackages();
  initPackageTreeView();

  initActions();
  initAppIcon();
  initToolBar();

  //Let's watch for changes in the Pacman db dir!
  m_pacmanDatabaseSystemWatcher = new QFileSystemWatcher(QStringList() << ctn_PACMAN_DATABASE_DIR, this);
  connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

  /* This timer is needed to beautify GUI initialization... */
  timer = new QTimer();
  connect(timer, SIGNAL(timeout()), this, SLOT(buildPackageList()));
  timer->start(40);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::refreshAppIcon()
{
  if(m_outdatedPackageList->count() > 0)
  {
    setWindowIcon(QIcon(":/resources/images/octopi_red.png"));
  }
  else
  {
    setWindowIcon(QIcon(":/resources/images/octopi_yellow.png"));
  }
}

/*
 * If we have some outdated packages, let's put octopi in a red face/angry state ;-)
 */
void MainWindow::initAppIcon()
{
  m_outdatedPackageList = Package::getOutdatedPackageList();
  m_numberOfOutdatedPackages = m_outdatedPackageList->count();
  refreshAppIcon();
}

void MainWindow::initToolBar()
{
  ui->mainToolBar->addAction(ui->actionSyncPackages);
  ui->mainToolBar->addAction(ui->actionCommit);
  ui->mainToolBar->addAction(ui->actionRollback);
  ui->mainToolBar->addSeparator();
  ui->mainToolBar->addAction(ui->actionExit);
}

void MainWindow::initStatusBar()
{
  m_lblCounters = new QLabel(this);
}

/*
 * Prints the list of outdated packages to the Output tab.
 */
void MainWindow::outputOutdatedPackageList()
{
  m_numberOfOutdatedPackages = m_outdatedPackageList->count();

  if(m_numberOfOutdatedPackages > 0)
  {
    clearTabOutput();

    if(m_outdatedPackageList->count()==1){
      writeToTabOutput("<b>" + StrConstants::getOneOutdatedPackage() + "</b><br>");
    }
    else
    {
      writeToTabOutput("<b>" +
                       StrConstants::getOutdatedPackages().arg(m_outdatedPackageList->count()) + "</b><br>");
    }

    for (int c=0; c < m_outdatedPackageList->count(); c++)
    {
      QString pkg = m_outdatedPackageList->at(c);
      pkg = pkg.leftJustified(40, QChar(' '));
      writeToTabOutput("<pre><font color=\"red\">" + pkg + "</font> " +
                       StrConstants::getNewVersionAvailable().arg(getInstalledPackageVersionByName(m_outdatedPackageList->at(c))) + "</pre>");
    }

    writeToTabOutput("<br>");
    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  }
}

/*
 * This is the 4th and LAST tab.
 * It pops up whenever the user selects a remove/install action on a selected package
 */
void MainWindow::initTabTransaction()
{
  m_modelTransaction = new QStandardItemModel(this);
  QWidget *tabTransaction = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout ( tabTransaction );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  QTreeView *tvTransaction = new QTreeView(tabTransaction);
  tvTransaction->setObjectName("tvTransaction");
  tvTransaction->setEditTriggers(QAbstractItemView::NoEditTriggers);
  tvTransaction->setDropIndicatorShown(false);
  tvTransaction->setAcceptDrops(false);
  tvTransaction->header()->setSortIndicatorShown(false);
  tvTransaction->header()->setClickable(false);
  tvTransaction->header()->setMovable(false);
  tvTransaction->setFrameShape(QFrame::NoFrame);
  tvTransaction->setFrameShadow(QFrame::Plain);
  tvTransaction->setStyleSheet(StrConstants::getTreeViewCSS(SettingsManager::getPkgListFontSize()));
  tvTransaction->expandAll();

  m_modelTransaction->setSortRole(0);
  m_modelTransaction->setColumnCount(0);

  QStringList sl;
  m_modelTransaction->setHorizontalHeaderLabels(sl << "Package");

  QStandardItem *siToBeRemoved = new QStandardItem(IconHelper::getIconToRemove(), StrConstants::getTodoRemoveText());
  QStandardItem *siToBeInstalled = new QStandardItem(IconHelper::getIconToInstall(), StrConstants::getTodoInstallText());

  m_modelTransaction->appendRow(siToBeRemoved);
  m_modelTransaction->appendRow(siToBeInstalled);

  gridLayoutX->addWidget ( tvTransaction, 0, 0, 1, 1 );

  tvTransaction->setModel(m_modelTransaction);

  QString aux(tr("Transaction"));

  ui->twProperties->removeTab(ctn_TABINDEX_TRANSACTION);
  ui->twProperties->insertTab(ctn_TABINDEX_TRANSACTION, tabTransaction, QApplication::translate (
                                "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ));
}

QStandardItem * MainWindow::getRemoveTransactionParentItem()
{
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvTransaction->model());
  QStandardItem *si;

  if(sim)
  {
    si = sim->item(0, 0);
  }

  return si;
}

QStandardItem * MainWindow::getInstallTransactionParentItem()
{
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvTransaction->model());
  QStandardItem *si;

  if(sim)
  {
    si = sim->item(1, 0);
  }

  return si;
}

void MainWindow::insertRemovePackageInTransaction(const QString &pkgName)
{
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItem * si = getRemoveTransactionParentItem();
  QStandardItem * siRemove = new QStandardItem(pkgName);
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(si->model());

  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);
  if (foundItems.size() == 0) si->appendRow(siRemove);
  ui->twProperties->setCurrentIndex(ctn_TABINDEX_TRANSACTION);
  tvTransaction->expandAll();

  changeTransactionActionsState();
}

void MainWindow::insertInstallPackageInTransaction(const QString &pkgName)
{
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItem * si = getInstallTransactionParentItem();
  QStandardItem * siInstall = new QStandardItem(pkgName);
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(si->model());

  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);
  if (foundItems.size() == 0) si->appendRow(siInstall);
  ui->twProperties->setCurrentIndex(ctn_TABINDEX_TRANSACTION);
  tvTransaction->expandAll();

  changeTransactionActionsState();
}

void MainWindow::removePackagesFromRemoveTransaction()
{
  QStandardItem * siRemove = getRemoveTransactionParentItem();
  siRemove->removeRows(0, siRemove->rowCount());
  changeTransactionActionsState();
}

void MainWindow::removePackagesFromInstallTransaction()
{
  QStandardItem * siInstall = getInstallTransactionParentItem();
  siInstall->removeRows(0, siInstall->rowCount());
  changeTransactionActionsState();
}

/*
 * Retrieve the list of all packages scheduled to be removed
 */
QString MainWindow::getToBeRemovedPackages()
{
  QStandardItem * siRemoval = getRemoveTransactionParentItem();
  QString res;

  for(int c=0; c < siRemoval->rowCount(); c++)
  {
    res += siRemoval->child(c)->text() + " ";
  }

  res = res.trimmed();
  return res;
}

/*
 * Retrieve the list of all packages scheduled to be installed
 */
QString MainWindow::getToBeInstalledPackages()
{
  QStandardItem * siInstall = getInstallTransactionParentItem();
  QString res;

  for(int c=0; c < siInstall->rowCount(); c++)
  {
    res += siInstall->child(c)->text() + " ";
  }

  res = res.trimmed();
  return res;
}


void MainWindow::initLineEditFilterPackages(){
  connect(ui->leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
}

/*
 * This is the package treeview, it lists the installed [and not installed] packages in the system.
 */
void MainWindow::initPackageTreeView()
{
  m_proxyModelPackages = new QSortFilterProxyModel(this);
  m_modelPackages = new QStandardItemModel(this);
  m_modelInstalledPackages = new QStandardItemModel(this);
  m_proxyModelPackages->setSourceModel(m_modelPackages);
  m_proxyModelPackages->setFilterKeyColumn(1);
  ui->tvPackages->setItemDelegate(new TreeViewPackagesItemDelegate(ui->tvPackages));
  ui->tvPackages->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->tvPackages->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->tvPackages->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->tvPackages->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
  ui->tvPackages->setAllColumnsShowFocus( true );
  ui->tvPackages->setModel(m_proxyModelPackages);
  ui->tvPackages->setSortingEnabled( true );
  ui->tvPackages->sortByColumn( 1, Qt::AscendingOrder);
  ui->tvPackages->setIndentation( 0 );
  ui->tvPackages->header()->setSortIndicatorShown(true);
  ui->tvPackages->header()->setClickable(true);
  ui->tvPackages->header()->setMovable(false);
  ui->tvPackages->header()->setDefaultAlignment( Qt::AlignLeft );
  ui->tvPackages->header()->setResizeMode( QHeaderView::Fixed );
  ui->tvPackages->setStyleSheet(
        StrConstants::getTreeViewCSS(SettingsManager::getPackagesInDirFontSize()));

  //Prepare it for drag operations
  //tvPackage->setDragEnabled(true);
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

  QString aux(tr("Info"));
  //QString translated_about = QApplication::translate ( "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 );

  ui->twProperties->removeTab(ctn_TABINDEX_INFORMATION);
  /*int tindex =*/ ui->twProperties->insertTab(ctn_TABINDEX_INFORMATION, tabInfo, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  //ui->twProperties->setTabText(ui->twProperties->indexOf(tabInfo), QApplication::translate(
  //    "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

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
  tvPkgFileList->setStyleSheet(StrConstants::getTreeViewCSS(SettingsManager::getPkgListFontSize()));

  modelPkgFileList->setSortRole(0);
  modelPkgFileList->setColumnCount(0);
  gridLayoutX->addWidget ( tvPkgFileList, 0, 0, 1, 1 );

  tvPkgFileList->setModel(modelPkgFileList);

  QString aux(tr("Files"));
  ui->twProperties->removeTab(ctn_TABINDEX_FILES);
  ui->twProperties->insertTab(ctn_TABINDEX_FILES, tabPkgFileList, QApplication::translate (
                                                  "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );

  /*twTODO->setTabText(twTODO->indexOf(tabPkgFileList), QApplication::translate(
      "MainWindow", tabName.toUtf8(), 0, QApplication::UnicodeUTF8));*/

  tvPkgFileList->setContextMenuPolicy(Qt::CustomContextMenu);

  //connect(tvPkgFileList, SIGNAL(customContextMenuRequested(QPoint)),
  //        this, SLOT(execContextMenuPkgFileList(QPoint)));
  //connect(tvPkgFileList, SIGNAL(clicked (const QModelIndex&)),
  //        this, SLOT(showFullPathOfObject(const QModelIndex&)));

  connect(tvPkgFileList, SIGNAL(doubleClicked (const QModelIndex&)),
          this, SLOT(openFile(const QModelIndex&)));
  //connect(tvPkgFileList, SIGNAL(activated(const QModelIndex)),
  //        this, SLOT(openFileOrDirectory(QModelIndex)));

  //connect(tvPkgFileList, SIGNAL(pressed (const QModelIndex&)),
  //        tvPkgFileList, SIGNAL(clicked (const QModelIndex&)));
}

/*
 * This is the TextEdit output pane, which shows the output of pacman commands.
 */
void MainWindow::initTabOutput()
{
  QWidget *tabOutput = new QWidget();
  QGridLayout *gridLayoutX = new QGridLayout ( tabOutput );
  gridLayoutX->setSpacing ( 0 );
  gridLayoutX->setMargin ( 0 );

  QTextEdit *text = new QTextEdit(tabOutput);
  text->setObjectName("textOutputEdit");
  text->setReadOnly(true);
  text->setFrameShape(QFrame::NoFrame);
  text->setFrameShadow(QFrame::Plain);
  gridLayoutX->addWidget ( text, 0, 0, 1, 1 );

  QString aux(tr("Output"));
  //QString translated_about = QApplication::translate ( "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 );

  ui->twProperties->removeTab(ctn_TABINDEX_OUTPUT);
  ui->twProperties->insertTab(ctn_TABINDEX_OUTPUT, tabOutput, QApplication::translate (
      "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8 ) );
  //ui->twProperties->setTabText(ui->twProperties->indexOf(tabInfo), QApplication::translate(
  //    "MainWindow", aux.toUtf8(), 0, QApplication::UnicodeUTF8));

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
  text->show();
  text->setFocus();
}

void MainWindow::clearTabOutput()
{
  QTextEdit *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextEdit*>("textOutputEdit");
  if (text)
  {
    text->clear();
  }
}

/*
 * This method searchs model modelInstalledPackages by a package name and returns it's version
 */
QString MainWindow::getInstalledPackageVersionByName(const QString &pkgName)
{
  QList<QStandardItem *> foundItems = m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);
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
 * This method searchs model modelInstalledPackages by a package name and returns if it is already installed
 */
bool MainWindow::isPackageInstalled(const QString &pkgName)
{
  QList<QStandardItem *> foundItems = m_modelInstalledPackages->findItems(pkgName, Qt::MatchExactly, ctn_PACKAGE_NAME_COLUMN);

  return (foundItems.count() > 0);
}

/*
 * Populates the list of available packages (installed [+ non-installed])
 */
void MainWindow::buildPackageList()
{
  disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

  static bool firstTime = true;
  timer->stop();

  CPUIntensiveComputing cic;  
  m_modelPackages->clear();
  m_modelInstalledPackages->clear();
  QStringList sl;

  if(!firstTime) //If it's not the starting of the app...
  {
    //Let's get outdatedPackages list again!
    m_outdatedPackageList = Package::getOutdatedPackageList();
    m_numberOfOutdatedPackages = m_outdatedPackageList->count();
  }

  QStringList *unrequiredPackageList = Package::getUnrequiredPackageList();
  QList<PackageListData> *list = Package::getPackageList();
  QList<PackageListData> *listForeign = Package::getForeignPackageList();
  QList<PackageListData>::const_iterator itForeign = listForeign->begin();

  while (itForeign != listForeign->end())
  {
    list->append(*itForeign);
    itForeign++;
  }

  QStandardItem *parentItem = m_modelPackages->invisibleRootItem();
  QStandardItem *parentItemInstalledPackages = m_modelInstalledPackages->invisibleRootItem();

  QList<PackageListData>::const_iterator it = list->begin();
  QList<QStandardItem*> lIcons, lNames, lVersions, lRepositories;
  QList<QStandardItem*> lIcons2, lNames2, lVersions2, lRepositories2;

  QProgressDialog progress(StrConstants::getBuildingPackageList(), "", 0, list->count(), this);
  progress.setValue(0);
  progress.setMinimumDuration(10);
  progress.setCancelButton(0);
  progress.setWindowModality(Qt::WindowModal);

  int counter=0;
  while(it != list->end())
  {
    PackageListData pld = *it;
    if (pld.status == ectn_NON_INSTALLED && !ui->actionNonInstalledPackages->isChecked()){
      it++;
      continue;
    }

    //If this is an installed package, it can be also outdated!
    switch (pld.status)
    {
    case ectn_FOREIGN:
      lIcons << new QStandardItem(IconHelper::getIconForeign(), "_Foreign");
      break;

    case ectn_OUTDATED:
      lIcons << new QStandardItem(IconHelper::getIconOutdated(), "_OutDated^"+pld.outatedVersion);
      break;
    case ectn_INSTALLED:
      //Is this package unrequired too?
      if (unrequiredPackageList->contains(pld.name))
      {
        lIcons << new QStandardItem(IconHelper::getIconUnrequired(), "_Unrequired");
      }
      else
      {
        lIcons << new QStandardItem(IconHelper::getIconInstalled(), "_Installed");
      }

      break;
    case ectn_NON_INSTALLED:
      lIcons << new QStandardItem(IconHelper::getIconNonInstalled(), "_NonInstalled");
      break;
    default:;
    }

    lNames << new QStandardItem(pld.name);
    lVersions << new QStandardItem(pld.version);
    lRepositories << new QStandardItem(pld.repository);

    //If this is an INSTALLED package, we add it to the model view of installed packages!
    if (pld.status != ectn_NON_INSTALLED)
    {
      lIcons2 << lIcons.last()->clone();
      lNames2 << lNames.last()->clone();
      lVersions2 << lVersions.last()->clone();
      lRepositories2 << lRepositories.last()->clone();
    }

    counter++;
    progress.setValue(counter);
    qApp->processEvents();
    it++;
  }

  parentItem->insertColumn(0, lIcons);
  parentItem->insertColumn(1, lNames);
  parentItem->insertColumn(2, lVersions);
  parentItem->insertColumn(3, lRepositories);

  parentItemInstalledPackages->insertColumn(0, lIcons2);
  parentItemInstalledPackages->insertColumn(1, lNames2);
  parentItemInstalledPackages->insertColumn(2, lVersions2);
  parentItemInstalledPackages->insertColumn(3, lRepositories2);

  ui->tvPackages->setColumnWidth(0, 24);
  ui->tvPackages->setColumnWidth(1, 500);
  ui->tvPackages->setColumnWidth(2, 160);
  ui->tvPackages->sortByColumn(m_PackageListOrderedCol, m_PackageListSortOrder);

  m_modelPackages->setHorizontalHeaderLabels(
        sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());

  if (ui->leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);    

  list->clear();
  refreshTabInfo();
  ui->tvPackages->setFocus();

  //Refresh counters
  m_numberOfInstalledPackages = m_modelInstalledPackages->invisibleRootItem()->rowCount();
  m_numberOfAvailablePackages = m_modelPackages->invisibleRootItem()->rowCount() - m_numberOfInstalledPackages;

  //Refresh statusbar widget
  refreshStatusBar();
  firstTime = false;

  //Refresh application icon
  refreshAppIcon();

  connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));
}

/*
 * This method prints the values of the package counters at the right of the statusBar
 */
void MainWindow::refreshStatusBar()
{
  QString text;

  if(m_numberOfOutdatedPackages > 0)
  {
    text = StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages) +
        " | <font color=\"red\"><a href=\"dummy\" style=\"color:\'red\'\">" +
        StrConstants::getNumberOutdatedPackages().arg(m_numberOfOutdatedPackages) + "</a></font> | " +
        StrConstants::getNumberAvailablePackages().arg(m_numberOfAvailablePackages);
  }
  else
  {
    text = StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages) +
        " | " + StrConstants::getNumberAvailablePackages().arg(m_numberOfAvailablePackages);
  }

  m_lblCounters->setText(text);
  ui->statusBar->addPermanentWidget(m_lblCounters);
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
    m_modelPackages->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());
    m_proxyModelPackages->setSourceModel(m_modelPackages);
  }
  else
  {
    m_modelInstalledPackages->setHorizontalHeaderLabels(
          sl << "" << StrConstants::getName() << StrConstants::getVersion() << StrConstants::getRepository());
    m_proxyModelPackages->setSourceModel(m_modelInstalledPackages);
  }

  if (ui->leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex maux = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->selectionModel()->setCurrentIndex(maux, QItemSelectionModel::Select);

  changedTabIndex();
}

/*
 * Brings the context menu when the user clicks the right button above the package list
 */
void MainWindow::execContextMenuPackages(QPoint point)
{
  if(ui->tvPackages->selectionModel()->selectedRows().count() > 0)
  {
    //QIcon lastType;
    QStandardItemModel * sim;
    bool allSameType = true;
    bool allInstallable = true;
    bool allRemovable = true;

    if(ui->actionNonInstalledPackages->isChecked())
    {
      sim = m_modelPackages;
    }
    else
    {
      sim = m_modelInstalledPackages;
    }

    foreach(QModelIndex item, ui->tvPackages->selectionModel()->selectedRows())
    {
      QModelIndex mi = m_proxyModelPackages->mapToSource(item);
      QStandardItem *si = sim->item(mi.row(), ctn_COLUMN_PACKAGE_ICON);

      /*if ((lastType.cacheKey()!=0 )&&
          (lastType.pixmap(QSize(22,22)).toImage()) != si->icon().pixmap(QSize(22,22)).toImage())
      {
        allSameType = false;
      }*/

      if((si->icon().pixmap(QSize(22,22)).toImage()) == IconHelper::getIconForeign().pixmap(QSize(22,22)).toImage())
      {
        allInstallable = false;
      }
      else if((si->icon().pixmap(QSize(22,22)).toImage()) == IconHelper::getIconNonInstalled().pixmap(QSize(22,22)).toImage())
      {
        allRemovable = false;
      }

      //lastType = si->icon();
    }

    if (allSameType)
    {
      QMenu *menu = new QMenu(this);

      if(allInstallable)
      {
        menu->addAction(ui->actionInstall);
      }

      if(allRemovable)
      {
        menu->addAction(ui->actionRemove);
      }

      if(menu->actions().count() > 0)
      {
        QPoint pt2 = ui->tvPackages->mapToGlobal(point);
        pt2.setY(pt2.y() + ui->tvPackages->header()->height());
        menu->exec(pt2);
      }
    }
  }
}

/*
 * Re-populates the HTML view with selected package's information (tab ONE)
 */
void MainWindow::refreshTabInfo(bool clearContents)
{
  static QString strSelectedPackage;

  if(ui->twProperties->currentIndex() != ctn_TABINDEX_INFORMATION) return;
  if (clearContents || ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");
    if (text)
    {
      text->clear();
    }

    strSelectedPackage="";
    return;
  }

  QModelIndex item = ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).first();
  QModelIndex mi = m_proxyModelPackages->mapToSource(item);

  QStandardItem *siIcon;
  QStandardItem *siName;
  QStandardItem *siRepository;
  QStandardItem *siVersion;

  if (ui->actionNonInstalledPackages->isChecked())
  {
    siIcon = m_modelPackages->item( mi.row(), ctn_COLUMN_PACKAGE_ICON);
    siName = m_modelPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
    siRepository = m_modelPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
    siVersion = m_modelPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
  }
  else
  {
    siIcon = m_modelInstalledPackages->item( mi.row(), ctn_COLUMN_PACKAGE_ICON);
    siName = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
    siRepository = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
    siVersion = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
  }

  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == siRepository->text()+"#"+siName->text()+"#"+siVersion->text())
    return;

  CPUIntensiveComputing cic;

  /* Appends all info from the selected package! */
  QString pkgName=siName->text();
  PackageInfoData pid;

  if (!siRepository->text().isEmpty()){
    pid = Package::getInformation(pkgName);
  }
  else
  {
    pid = Package::getInformation(pkgName, true); //This is a foreign package!!!
  }

  //QString repository = StrConstants::getRepository();
  //QString name = StrConstants::getName();
  QString version = StrConstants::getVersion();
  QString url = StrConstants::getURL();
  QString licenses = StrConstants::getLicenses();
  QString groups = StrConstants::getGroups();
  QString provides = StrConstants::getProvides();
  QString dependsOn = StrConstants::getDependsOn();
  QString optionalDeps = StrConstants::getOptionalDeps();
  QString conflictsWith = StrConstants::getConflictsWith();
  QString replaces = StrConstants::getReplaces();
  QString downloadSize = StrConstants::getDownloadSize();
  QString installedSize = StrConstants::getInstalledSize();
  QString packager = StrConstants::getPackager();
  QString architecture = StrConstants::getArchitecture();
  QString buildDate = StrConstants::getBuildDate();
  QString description = StrConstants::getDescription();

  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

  if (text)
  {
    QString html;
    QString valDownloadSize =
        QString("%1 KB").arg(pid.downloadSize, 6, 'f', 2);

    QString valInstalledSize =
        QString("%1 KB").arg(pid.installedSize, 6, 'f', 2);

    text->clear();
    QString anchorBegin = "anchorBegin";

    html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
    html += "<a id=\"" + anchorBegin + "\"></a>";

    html += "<table border=\"0\">";

    html += "<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>";
    html += "<tr><td>" + description + "</td><td style=\"font-size:16px;\">" + pid.description + "</td></tr>";
    html += "<tr><td>" + url + "</td><td style=\"font-size:14px;\">" + pid.url + "</td></tr>";

    //html += "<tr><td>" + repository + "</td><td>" + siRepository->text() + "</td></tr>";
    //html += "<tr><td>" + name + "</td><td>" + siName->text() + "</td></tr>";

    int mark = siIcon->text().indexOf('^');
    if (mark >= 0)
    {
      QString outdatedVersion = siIcon->text().right(siIcon->text().size()-mark-1);
      html += "<tr><td>" + version + "</td><td>" + siVersion->text() + "<b><font color=\"red\">"
                       + StrConstants::getOutdatedInstalledVersion().arg(outdatedVersion) +
                       "</b></font></td></tr>";
    }
    else
    {
      html += "<tr><td>" + version + "</td><td>" + siVersion->text() + "</td></tr>";
    }

    html += "<tr><td>" + licenses + "</td><td>" + pid.license + "</td></tr>";
    html += "<tr><td>" + groups + "</td><td>" + pid.group + "</td></tr>";
    html += "<tr><td>" + provides + "</td><td>" + pid.provides + "</td></tr>";
    html += "<tr><td>" + dependsOn + "</td><td>" + pid.dependsOn + "</td></tr>";
    html += "<tr><td>" + optionalDeps + "</td><td>" + pid.optDepends + "</td></tr>";
    html += "<tr><td><b>" /*<font color=\"red\">*/ + conflictsWith +
        "</b></td><td><b>" /*<font color=\"red\"><b>"*/ + pid.conflictsWith + "</b></font></td></tr>";
    html += "<tr><td>" + replaces + "</td><td>" + pid.replaces + "</td></tr>";
    html += "<tr><td>" + downloadSize + "</td><td>" + valDownloadSize + "</td></tr>";
    html += "<tr><td>" + installedSize + "</td><td>" + valInstalledSize + "</td></tr>";
    html += "<tr><td>" + packager + "</td><td>" + pid.packager + "</td></tr>";
    html += "<tr><td>" + architecture + "</td><td>" + pid.arch + "</td></tr>";
    html += "<tr><td>" + buildDate + "</td><td>" + pid.buildDate.toString("ddd - dd/MM/yyyy hh:mm:ss") + "</td></tr>";

    html += "</table>";

    text->setHtml(html);
    text->scrollToAnchor(anchorBegin);
  }

  strSelectedPackage = siRepository->text()+"#"+siName->text()+"#"+siVersion->text();
}

/*
 * Re-populates the treeview with file list of selected package (tab TWO)
 */
void MainWindow::refreshTabFiles(bool clearContents)
{
  static QString strSelectedPackage;

  if(ui->twProperties->currentIndex() != ctn_TABINDEX_FILES) return;
  if (clearContents || ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).count() == 0){
    QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
    if(tvPkgFileList)
    {
      QStandardItemModel *modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
      modelPkgFileList->clear();
      strSelectedPackage="";
      return;
    }
  }

  QModelIndex item = ui->tvPackages->selectionModel()->selectedRows(ctn_PACKAGE_NAME_COLUMN).first();
  QModelIndex mi = m_proxyModelPackages->mapToSource(item);
  QStandardItem *siName;
  QStandardItem *siRepository;
  QStandardItem *siVersion;

  if (ui->actionNonInstalledPackages->isChecked())
  {
    siName = m_modelPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
    siRepository = m_modelPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
    siVersion = m_modelPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
  }
  else
  {
    siName = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_NAME_COLUMN);
    siRepository = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);
    siVersion = m_modelInstalledPackages->item( mi.row(), ctn_PACKAGE_VERSION_COLUMN);
  }

  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == siRepository->text()+"#"+siName->text()+"#"+siVersion->text())
    return;

  //Maybe this is a non-installed package...
  bool nonInstalled = (ui->actionNonInstalledPackages->isChecked() &&
                       (m_modelPackages->item(mi.row(), ctn_COLUMN_PACKAGE_ICON)->text() == "_NonInstalled"));

  QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
  if(tvPkgFileList){
    CPUIntensiveComputing cic;

    QString pkgName = siName->text();
    QStringList fileList = Package::getContents(pkgName);

    QStandardItemModel *fakeModelPkgFileList = new QStandardItemModel(this);
    QStandardItemModel *modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
    modelPkgFileList->clear();
    QStandardItem *fakeRoot = fakeModelPkgFileList->invisibleRootItem();
    QStandardItem *root = modelPkgFileList->invisibleRootItem();
    QStandardItem *bkpDir, *item, *bkpItem=root, *parent;
    bool first=true;
    bkpDir = root;

    if(nonInstalled){
      strSelectedPackage="";
      return;
    }

    foreach ( QString file, fileList ){
      QFileInfo fi ( file );
      //if ( file.endsWith ( '/' ) ){
      if(fi.isDir()){
        if ( first == true ){
          item = new QStandardItem ( IconHelper::getIconFolder(), file );
          item->setAccessibleDescription("directory " + item->text());
          fakeRoot->appendRow ( item );
        }
        else{
          if ( file.indexOf ( bkpDir->text() ) != -1 ){
            item = new QStandardItem ( IconHelper::getIconFolder(), file );
            item->setAccessibleDescription("directory " + item->text());
            bkpDir->appendRow ( item );
          }
          else{
            parent = bkpItem->parent();
            do{
              if ( parent == 0 || file.indexOf ( parent->text() ) != -1 ) break;
              parent = parent->parent();
            }
            while ( parent != fakeRoot );

            item = new QStandardItem ( IconHelper::getIconFolder(), file );
            item->setAccessibleDescription("directory " + item->text());
            if ( parent != 0 ) parent->appendRow ( item );
            else fakeRoot->appendRow ( item );
          }
        }
        bkpDir = item;
      }
      else{
        item = new QStandardItem ( IconHelper::getIconBinary(), fi.fileName() );
        item->setAccessibleDescription("file " + item->text());
        parent = bkpDir;

        do{
          if ( parent == 0 || file.indexOf ( parent->text() ) != -1 ) break;
          parent = parent->parent();
        }
        while ( parent != fakeRoot );

        parent->appendRow ( item );
      }

      bkpItem = item;
      first = false;
    }

    //tabPkgFileList->setStatusTip(pkgName);
    QFileInfo info(pkgName);
    //QString tabName(info.fileName());

    root = fakeRoot;
    fakeModelPkgFileList->sort(0);
    modelPkgFileList = fakeModelPkgFileList;
    tvPkgFileList->setModel(modelPkgFileList);
    tvPkgFileList->header()->setDefaultAlignment( Qt::AlignCenter );
    modelPkgFileList->setHorizontalHeaderLabels( QStringList() <<
                                                 tr("Contents of \"%1\"").arg(pkgName));

    QList<QStandardItem*> lit = modelPkgFileList->findItems( "/", Qt::MatchStartsWith | Qt::MatchRecursive );

    foreach( QStandardItem* it, lit ){
      QFileInfo fi( it->text() );
      if ( fi.isFile() == false ){
        QString s( it->text() );
        s.remove(s.size()-1, 1);
        s = s.right(s.size() - s.lastIndexOf('/') -1);
        it->setText( s );
      }
    }
  }

  strSelectedPackage = siRepository->text()+"#"+siName->text()+"#"+siVersion->text();
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
}

/*
 * This method clears the information showed on the current tab (Info or Files).
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
 * This method does a repository sync with "pacman -Sy" !
 */
void MainWindow::doSyncDatabase()
{
  //If there are no means to run the actions, we must warn!
  if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand());
    return;
  }

  m_commandExecuting = ectn_SYNC_DATABASE;
  ui->actionSyncPackages->setEnabled(false);
  ui->actionSystemUpgrade->setEnabled(false);

  clearTabOutput();
  disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

  writeToTabOutput("<B>" + StrConstants::getSyncDatabases() + "</B>");

  m_unixCommand = new UnixCommand(this);

  QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                   this, SLOT( actionsProcessReadOutput() ));
  QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                   this, SLOT( actionsProcessRaisedError() ));

  QStringList commands;
  commands << "pacman -Sy";
  m_unixCommand->executePackageActions(commands);
}

/*
 * This method does a system upgrade with "pacman -Su" !
 */
void MainWindow::doSystemUpgrade(bool syncDatabase)
{
  if(syncDatabase)
  {
    m_commandQueued = ectn_SYSTEM_UPGRADE;
    doSyncDatabase();
  }
  else
  {
    //Shows a dialog indicating the targets needed to be retrieved and asks for the user's permission.
    m_targets = Package::getTargetUpgradeList();

    //There are no new updates to install!
    if (m_targets->count() == 0)
    {
      clearTabOutput();
      writeToTabOutput("<b>" + StrConstants::getNoNewUpdatesAvailable() + "</b>");
      return;
    }

    m_currentTarget=0;
    QString list;

    foreach(QString target, *m_targets)
    {
      list = list + target + "\n";
    }
    list.remove(list.size()-1, 1);

    QMessageBox question;

    if(m_targets->count()==1)
      question.setText(StrConstants::getRetrieveTarget());
    else
      question.setText(StrConstants::getRetrieveTargets().arg(m_targets->count()));

    question.setInformativeText(StrConstants::getConfirmationQuestion());
    question.setDetailedText(list);
    question.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    question.setDefaultButton(QMessageBox::No);

    int result = question.exec();
    if(result == QMessageBox::Yes)
    {
      //If there are no means to run the actions, we must warn!
      if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
        QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand());
        return;
      }

      m_commandExecuting = ectn_SYSTEM_UPGRADE;
      ui->actionSyncPackages->setEnabled(false);
      ui->actionSystemUpgrade->setEnabled(false);

      clearTabOutput();
      disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

      writeToTabOutput("<B>" + StrConstants::getSystemUpgrade() + "</B><br>");

      m_unixCommand = new UnixCommand(this);

      QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
      QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                       this, SLOT( actionsProcessReadOutput() ));
      QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                       this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
      QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                       this, SLOT( actionsProcessRaisedError() ));

      m_currentTarget = 0;

      QStringList command;
      command << "pacman -Su --noconfirm";
      m_unixCommand->executePackageActions(command);
      m_commandQueued = ectn_NONE;
    }
  }
}

/*
 * This method removes ALL the packages selected by the user with "pacman -Rc (CASCADE)" !
 */
void MainWindow::doRemove()
{
  //Shows a dialog indicating the targets which will be removed and asks for the user's permission.
  QString listOfTargets = getToBeRemovedPackages();
  m_targets = Package::getTargetRemovalList(listOfTargets);
  m_currentTarget=0;
  QString list;

  foreach(QString target, *m_targets)
  {
    list = list + target + "\n";
  }
  list.remove(list.size()-1, 1);

  QMessageBox question;

  Q_ASSERT(m_targets->count() > 0);

  if(m_targets->count()==1)
  {
    if (m_targets->at(0).indexOf("HoldPkg was found in target list.") != -1)
    {
      QMessageBox::warning(this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
    else question.setText(StrConstants::getRemoveTarget());
  }
  else
    question.setText(StrConstants::getRemoveTargets().arg(m_targets->count()));

  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  question.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
  question.setDefaultButton(QMessageBox::No);

  m_commandExecuting = ectn_REMOVE;

  int result = question.exec();
  if(result == QMessageBox::Yes)
  {
    //If there are no means to run the actions, we must warn!
    if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
      QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand());
      return;
    }

    clearTabOutput();
    disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

    writeToTabOutput("<B>" + StrConstants::getRemovingPackages() + "</B>");

    m_unixCommand = new UnixCommand(this);

    QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                     this, SLOT( actionsProcessReadOutput() ));
    QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                     this, SLOT( actionsProcessRaisedError() ));

    QStringList command;
    command << "pacman -Rcs --noconfirm " + listOfTargets;
    m_unixCommand->executePackageActions(command);
  }
}

/*
 * This method installs ALL the packages selected by the user with "pacman -S (INCLUDING DEPENDENCIES)" !
 */
void MainWindow::doInstall()
{
  //Shows a dialog indicating the targets which will be removed and asks for the user's permission.
  QString listOfTargets = getToBeInstalledPackages();
  m_targets = Package::getTargetUpgradeList(listOfTargets);
  m_currentTarget=0;
  QString list;

  foreach(QString target, *m_targets)
  {
    list = list + target + "\n";
  }
  list.remove(list.size()-1, 1);

  QMessageBox question;

  Q_ASSERT(m_targets->count() > 0);

  if(m_targets->count()==1)
  {
    if (m_targets->at(0).indexOf("HoldPkg was found in target list.") != -1)
    {
      QMessageBox::warning(this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
    else question.setText(StrConstants::getRetrieveTarget());
  }
  else
    question.setText(StrConstants::getRetrieveTargets().arg(m_targets->count()));

  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  question.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
  question.setDefaultButton(QMessageBox::No);

  m_commandExecuting = ectn_INSTALL;

  int result = question.exec();
  if(result == QMessageBox::Yes)
  {
    //If there are no means to run the actions, we must warn!
    if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
      QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand());
      return;
    }

    clearTabOutput();
    disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

    writeToTabOutput("<B>" + StrConstants::getInstallingPackages() + "</B>");

    m_unixCommand = new UnixCommand(this);

    QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                     this, SLOT( actionsProcessReadOutput() ));
    QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                     this, SLOT( actionsProcessRaisedError() ));

    QStringList command;
    command << "pacman -S --noconfirm " + listOfTargets;
    m_unixCommand->executePackageActions(command);
  }
}

void MainWindow::commitTransaction()
{
  //Are there any remove actions to be commited?
  if(getRemoveTransactionParentItem()->rowCount() > 0 && getInstallTransactionParentItem()->rowCount() > 0)
  {
    doRemove();
    m_commandQueued = ectn_INSTALL;
  }
  else if(getRemoveTransactionParentItem()->rowCount() > 0)
  {
    doRemove();
  }
  else if(getInstallTransactionParentItem()->rowCount() > 0)
  {
    doInstall();
  }
}

void MainWindow::rollbackTransaction()
{
  int res = QMessageBox::question(this,
                        StrConstants::getConfirmation(),
                        StrConstants::getRollbackTransactionConfirmation(),
                        QMessageBox::Yes|QMessageBox::No,
                        QMessageBox::No);

  if(res == QMessageBox::Yes)
  {
    clearTransactionTreeView();
  }
}

void MainWindow::actionsProcessStarted()
{
  QString msg = m_unixCommand->readAllStandardOutput();
  msg.remove(QRegExp("\\b"));
  writeToTabOutput(msg);
}

void MainWindow::actionsProcessFinished(int exitCode, QProcess::ExitStatus)
{
  ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, tr("Output"));

  if (exitCode == 0){
    writeToTabOutput("<br><B>:: " +
                     StrConstants::getCommandFinishedOK() + "</B><br><br>");
  }
  else
  {
    writeToTabOutput("<br><B><font color=\"red\">:: " +
                     StrConstants::getCommandFinishedWithErrors() + "</font></B><br><br>");
  }

  if(m_commandQueued == ectn_SYSTEM_UPGRADE)
  {
    doSystemUpgrade(false);    
    m_commandQueued = ectn_NONE;
  }
  else if (m_commandQueued == ectn_INSTALL)
  {
    if(exitCode == 0) //If the removal actions were OK...
    {
      removePackagesFromRemoveTransaction();
      doInstall();
      removePackagesFromInstallTransaction();
      m_commandQueued = ectn_NONE;
    }
  }
  else if (m_commandQueued == ectn_NONE)
  {
    if(exitCode == 0)
    {
      //After the command, we can refresh the package list, so any change can be seem.
      buildPackageList();
      connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(buildPackageList()));

      clearTransactionTreeView();
    }
  }

  if (m_commandExecuting == ectn_SYNC_DATABASE){
    ui->actionSyncPackages->setEnabled(true);
    ui->actionSystemUpgrade->setEnabled(true);
  }
  else if (m_commandExecuting == ectn_SYSTEM_UPGRADE){
    ui->actionSyncPackages->setEnabled(true);
    ui->actionSystemUpgrade->setEnabled(true);
  }

  m_commandExecuting = ectn_NONE;
  m_unixCommand->removeTemporaryActionFile();
}

void MainWindow::actionsProcessReadOutput()
{
  QString msg = m_unixCommand->readAllStandardOutput();
  msg.remove(QRegExp("\\b"));

  if(!msg.isEmpty() &&
     msg.indexOf(":: Synchronizing package databases...") == -1 &&
     msg.indexOf(":: Starting full system upgrade...") == -1)
  {
    writeToTabOutput(msg);
  }
}

void MainWindow::_treatProcessOutput(const QString &pMsg)
{
  QString perc;
  QString msg = pMsg;
  msg.remove(QRegExp(".+\\[Y/n\\].+"));

  //If it is a percentage, we are talking about curl output...
  if(msg.indexOf("#]") != -1)
  {
    perc = "100%";
  }
  else if (msg.indexOf("-]") != -1)
  {
    QString target;
    perc = msg.right(4).trimmed();
    //std::cout << "Percentage: " << perc.toAscii().data() << std::endl;

    if (m_commandExecuting == ectn_SYSTEM_UPGRADE || m_commandExecuting == ectn_SYNC_DATABASE)
    {
      int ini = msg.indexOf("(");
      if (ini >= 0)
      {
        msg = msg.remove(0, 6);

        int pos = msg.indexOf(" ");
        if (pos >=0)
        {
          target = msg.left(pos);
          target = target.trimmed();
          //std::cout << "target: " << target.toAscii().data() << std::endl;

          if(!_textInTabOutput(target))
          {
            if(m_commandExecuting == ectn_SYNC_DATABASE)
            {
              writeToTabOutput("<font color=\"blue\">" + StrConstants::getSynchronizing() + " " + target + "</font>");
            }
            else writeToTabOutput("<font color=\"blue\">" + target + "</font>");
          }
        }
        else if (!_textInTabOutput(msg))
          writeToTabOutput("<font color=\"blue\">" + msg + "</font>");
      }
      else if (ini == -1)
      {
        int pos = msg.indexOf(" ");
        if (pos >=0)
        {
          target = msg.left(pos);
          target = target.trimmed();
          //std::cout << "target: " << target.toAscii().data() << std::endl;

          if(!_textInTabOutput(target))
            writeToTabOutput("<font color=\"blue\">" + target + "</font>");

        }
        else if (!_textInTabOutput(msg))
          writeToTabOutput("<font color=\"blue\">" + msg + "</font>");
      }
    }
    else if (m_commandExecuting == ectn_INSTALL)
    {
      int ini = msg.indexOf("(");
      if (ini >= 0)
      {
        msg = msg.remove(0, 6);
      }

      int pos = msg.indexOf("[");
      if (pos >=0)
      {
        target = msg.left(pos);
        target = target.trimmed();
        //std::cout << "target: " << target.toAscii().data() << std::endl;

        if(!_textInTabOutput(target))
          writeToTabOutput("<font color=\"blue\">" + target + "</font>");
      }
      else if (!_textInTabOutput(msg))
        writeToTabOutput("<font color=\"blue\">" + msg + "</font>");
    }
    else if (m_commandExecuting == ectn_REMOVE)
    {
      int ini = msg.indexOf("(");
      if (ini >= 0)
      {
        msg = msg.remove(0, 6);
      }

      int pos = msg.indexOf("[");
      if (pos >=0)
      {
        target = msg.left(pos);
        target = target.trimmed();
        //std::cout << "target: " << target.toAscii().data() << std::endl;

        if(!_textInTabOutput(target))
          writeToTabOutput("<font color=\"red\">" + target + "</font>");

      }
      else if (!_textInTabOutput(msg))
        writeToTabOutput("<font color=\"red\">" + msg + "</font>");
    }

    if(!perc.isEmpty() && perc.indexOf("%") > 0)
    {
      if(perc=="100%")
      {
        qApp->processEvents();
        ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, tr("Output (100%)"));
        qApp->processEvents();
      }
      else
      {
        qApp->processEvents();
        ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, tr("Output") + " (" + perc + ")");
        qApp->processEvents();
      }
    }
  }
  //It's another error, so we have to output it
  else
  {
    //(process:21527): Gtk-WARNING **: Locale not supported by C library. Using the fallback 'C' locale.
    //Gkr-Message: secret service operation failed: The name org.freedesktop.secrets was not provided by any .service files
    msg.remove(QRegExp("\\(process.+"));
    msg.remove(QRegExp("Using the fallback.+"));
    msg.remove(QRegExp("Gkr-Message: secret service operation failed: The name org.freedesktop.secrets was not provided by any .service files"));
    msg.remove(QRegExp("Do you want.+"));

    if (!_textInTabOutput(msg))
      writeToTabOutput("<b><font color=\"black\">" + msg + "</font></b>");
  }

  if(m_commandExecuting == ectn_NONE)
    ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, tr("Output"));
}

void MainWindow::actionsProcessRaisedError()
{
  QString msg = m_unixCommand->readAllStandardError();
  msg = msg.trimmed();
  QStringList msgs = msg.split(QRegExp("\\n"), QString::SkipEmptyParts);

  foreach (QString m, msgs)
  {

    QStringList m2 = m.split(QRegExp("\\(\\d/\\d\\) "));
    if (m2.count() <= 1)
    {
      if (!m.isEmpty())
      {
        //std::cout << "Error: " << m.toAscii().data() << std::endl;
        _treatProcessOutput(m);
      }
    }
    else
    {
      foreach (QString m3, m2)
      {
        if (!m3.isEmpty())
        {
          //std::cout << "Error: " << m3.toAscii().data() << std::endl;
          _treatProcessOutput(m3);
        }
      }
    }
  }
}

/*
 * This method inserts the current selected packages for removal into the Transaction Treeview
 */
void MainWindow::insertINRemovePackage()
{
  QStandardItemModel *sim;
  if(ui->actionNonInstalledPackages->isChecked())
  {
    sim = m_modelPackages;
  }
  else
  {
    sim = m_modelInstalledPackages;
  }

  foreach(QModelIndex item, ui->tvPackages->selectionModel()->selectedRows())
  {

    QModelIndex mi = m_proxyModelPackages->mapToSource(item);
    QStandardItem *si = sim->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);

    insertRemovePackageInTransaction(si->text());
  }
}

void MainWindow::insertINInstallPackage()
{
  QStandardItemModel *sim;
  if(ui->actionNonInstalledPackages->isChecked())
  {
    sim = m_modelPackages;
  }
  else
  {
    sim = m_modelInstalledPackages;
  }

  foreach(QModelIndex item, ui->tvPackages->selectionModel()->selectedRows())
  {

    QModelIndex mi = m_proxyModelPackages->mapToSource(item);
    QStandardItem *si = sim->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);

    insertInstallPackageInTransaction(si->text());
  }
}

/*
 * This method maximizes/de-maximizes the lower pane (tabwidget)
 */
void MainWindow::maximizeTabWidget()
{
  static QList<int> savedSizes = ui->splitterHorizontal->sizes();

  QList<int> l, rl;
  rl = ui->splitterHorizontal->sizes();

  if ( rl[0] != 0 )
  {
    ui->splitterHorizontal->setSizes( l << 0 << ui->twProperties->maximumHeight());
    if(ui->tvPackages->hasFocus())
      ui->twProperties->currentWidget()->setFocus();
  }
  else
  {
    ui->splitterHorizontal->setSizes(savedSizes);
  }
}

/*
 * This SLOT is called every time we press a key at FilterLineEdit
 */
void MainWindow::reapplyPackageFilter()
{
  CPUIntensiveComputing cic;

  bool isFilterPackageSelected = ui->leFilterPackage->hasFocus();
  QString search = Package::parseSearchString(ui->leFilterPackage->text());
  QRegExp regExp(search, Qt::CaseInsensitive, QRegExp::RegExp);

  m_proxyModelPackages->setFilterRegExp(regExp);
  int numPkgs = m_proxyModelPackages->rowCount();

  if (ui->leFilterPackage->text() != ""){
    if (numPkgs > 0)
      ui->leFilterPackage->setFoundStyle();
    else ui->leFilterPackage->setNotFoundStyle();
  }
  else{
    ui->leFilterPackage->initStyleSheet();;
    m_proxyModelPackages->setFilterRegExp("");
  }

  if (isFilterPackageSelected) ui->leFilterPackage->setFocus();
  m_proxyModelPackages->sort(m_PackageListOrderedCol, m_PackageListSortOrder);

  /*disconnect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
             SLOT(changedTabIndex()));*/

  ui->tvPackages->selectionModel()->clear();
  QModelIndex mi = m_proxyModelPackages->index(0, 0);
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  //changedTabIndex();
  invalidateTabs();

  /*connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
          SLOT(changedTabIndex()));*/
}

/*
 * Whenever a user clicks on the Sort indicator of the package treeview, we keep values to mantain his choices
 */
void MainWindow::headerViewPackageListSortIndicatorClicked( int col, Qt::SortOrder order )
{
  m_PackageListOrderedCol = col;
  m_PackageListSortOrder = order;
}

void MainWindow::initActions()
{
  connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this,
          SLOT(invalidateTabs()));

  connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
  connect(ui->actionNonInstalledPackages, SIGNAL(changed()), this, SLOT(changePackageListModel()));

  connect(ui->tvPackages, SIGNAL(activated(QModelIndex)), this, SLOT(changedTabIndex()));
  connect(ui->tvPackages, SIGNAL(clicked(QModelIndex)), this, SLOT(changedTabIndex()));
  connect(ui->tvPackages->header(), SIGNAL(sortIndicatorChanged(int,Qt::SortOrder)), this,
          SLOT(headerViewPackageListSortIndicatorClicked(int,Qt::SortOrder)));
  connect(ui->tvPackages, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(execContextMenuPackages(QPoint)));

  connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));

  connect(ui->actionSyncPackages, SIGNAL(triggered()), this, SLOT(doSyncDatabase()));
  connect(ui->actionSystemUpgrade, SIGNAL(triggered()), this, SLOT(doSystemUpgrade()));

  connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(insertINRemovePackage()));
  connect(ui->actionInstall, SIGNAL(triggered()), this, SLOT(insertINInstallPackage()));

  connect(ui->actionCommit, SIGNAL(triggered()), this, SLOT(commitTransaction()));
  connect(ui->actionRollback, SIGNAL(triggered()), this, SLOT(rollbackTransaction()));

  connect(m_lblCounters, SIGNAL(linkActivated(QString)), this, SLOT(outputOutdatedPackageList()));

  ui->actionCommit->setEnabled(false);
  ui->actionRollback->setEnabled(false);
}

/*
 * Helper method to position the text cursor always in the end of doc
 */
void MainWindow::_positionTextEditCursorAtEnd()
{
  QTextEdit *textEdit = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextEdit*>("textOutputEdit");
  if (textEdit){
    QTextCursor tc = textEdit->textCursor();
    tc.clearSelection();
    tc.movePosition(QTextCursor::End);
    textEdit->setTextCursor(tc);
  }
}

/*
 * Helper method to find the given "findText" in the Output TextEdit
 */
bool MainWindow::_textInTabOutput(const QString& findText)
{
  bool res;
  QTextEdit *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextEdit*>("textOutputEdit");
  if (text)
  {
    res = text->find(findText, QTextDocument::FindBackward);
    _positionTextEditCursorAtEnd();
  }

  return res;
}

/*
 * A helper method which writes the given string to the Output tab
 */
void MainWindow::writeToTabOutput(const QString &msg)
{
  QTextEdit *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextEdit*>("textOutputEdit");
  if (text)
  {
    _positionTextEditCursorAtEnd();    
    text->append(msg);
    text->ensureCursorVisible();
    ui->twProperties->setCurrentIndex(ctn_TABINDEX_OUTPUT);
    text->setFocus();
  }
}

/*
 * Whenever the user presses DEL over the Transaction TreeView, we:
 * - Delete the package if it's bellow of "To be removed" or "To be installed" parent;
 * - Delete all the parent's packages if the user clicked in "To be removed" or "To be installed" items.
 */
void MainWindow::onPressDelete()
{
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvTransaction->model());

  if (tvTransaction->hasFocus())
  {
    if(tvTransaction->currentIndex() == getRemoveTransactionParentItem()->index()){
      removePackagesFromRemoveTransaction();
    }
    else if(tvTransaction->currentIndex() == getInstallTransactionParentItem()->index()){
      removePackagesFromInstallTransaction();
    }
    else
      sim->removeRow(tvTransaction->currentIndex().row(), tvTransaction->currentIndex().parent());
  }
}

/*
 * Watches the state of the tvTransaction treeview to see if Commit/Rollback actions must be activated/deactivated
 */
void MainWindow::changeTransactionActionsState()
{
  bool state = (getRemoveTransactionParentItem()->hasChildren() || getInstallTransactionParentItem()->hasChildren());

  ui->actionCommit->setEnabled(state);
  ui->actionRollback->setEnabled(state);
}

void MainWindow::clearTransactionTreeView()
{
  removePackagesFromRemoveTransaction();
  removePackagesFromInstallTransaction();
}

void MainWindow::keyPressEvent(QKeyEvent* ke)
{    
  if (ke->key() == Qt::Key_Return)
  {
    QTreeView *tvPkgFileList = ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");
    if(tvPkgFileList)
    {
      if(tvPkgFileList->hasFocus())
      {
        openFile(tvPkgFileList->currentIndex());
    } }
  }
  else if(ke->key() == Qt::Key_Delete)
  {
    onPressDelete();
  }
  else if (ke->key() == Qt::Key_F12)
  {
    maximizeTabWidget();
  }
  else if(ke->key() == Qt::Key_F5)
  {
    invalidateTabs();
    buildPackageList();
  }
  else if(ke->key() == Qt::Key_L && ke->modifiers() == Qt::ControlModifier)
  {
    ui->leFilterPackage->setFocus();
    ui->leFilterPackage->selectAll();
  }
}

/*
 * This helper method opens an existing file using the available program/DE.
 */
void MainWindow::openFile(const QModelIndex& mi){
  const QStandardItemModel *sim = qobject_cast<const QStandardItemModel*>(mi.model());
  QStandardItem *si = sim->itemFromIndex(mi);

  if ((si->icon().pixmap(QSize(22,22)).toImage() ==
       IconHelper::getIconBinary().pixmap(QSize(22,22)).toImage()))
  {
    QString path = showFullPathOfObject(si->index());
    QFileInfo selectedFile(path);

    if (selectedFile.exists())
    {
      WMHelper::openFile(path);
    }
  }
}

/*
 * This method returns the full path of the selected file in any given TreeView.
 */
QString MainWindow::showFullPathOfObject(const QModelIndex & index){
  if (!index.isValid()) return "";

  const QStandardItemModel *sim = qobject_cast<const QStandardItemModel*>( index.model() );

  QStringList sl;
  QModelIndex nindex;
  QString str;
  sl << sim->itemFromIndex( index )->text();
  nindex = index;

  while (1){
    nindex = sim->parent( nindex );
    if ( nindex != sim->invisibleRootItem()->index() ) sl << sim->itemFromIndex( nindex )->text();
    else break;
  }
  str = QDir::separator() + str;

  for ( int i=sl.count()-1; i>=0; i-- ){
    if ( i < sl.count()-1 ) str += QDir::separator();
    str += sl[i];
  }

  return str;
}
