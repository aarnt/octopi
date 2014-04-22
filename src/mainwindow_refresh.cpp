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
 * This is MainWindow's refresh UI components related code
 */

#include "mainwindow.h"
#include "searchlineedit.h"
#include "ui_mainwindow.h"
#include "strconstants.h"
#include "uihelper.h"
#include "globals.h"
#include "packagecontroller.h"
#include <iostream>
#include <cassert>

#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QTextBrowser>
#include <QStandardItem>
#include <QFutureWatcher>

#if QT_VERSION > 0x050000
  #include <QtConcurrent/QtConcurrentRun>
#else
  #include <QtConcurrentRun>
#endif

#if QT_VERSION < 0x050000
  using namespace QtConcurrent;
#endif

#include "src/ui/octopitabinfo.h"


/*
 * If we have some outdated packages, let's put an angry red face icon in this app!
 */
void MainWindow::refreshAppIcon()
{
  bool enableSystemUpgrade=false;

  if(m_outdatedPackageList->count() > 0)
  {
    setWindowIcon(IconHelper::getIconOctopiRed());
    if(m_commandExecuting != ectn_MIRROR_CHECK) enableSystemUpgrade=true;
  }
  else if(m_outdatedYaourtPackageList->count() > 0)
  {
    setWindowIcon(IconHelper::getIconOctopiYellow());
  }
  else
  {
    setWindowIcon(IconHelper::getIconOctopiGreen());
  }

  ui->actionSystemUpgrade->setEnabled(enableSystemUpgrade);
}

/*
 * Inserts the group names into the Groups comboBox
 */
void MainWindow::refreshGroupsWidget()
{
  disconnect(ui->twGroups, SIGNAL(itemSelectionChanged()), this, SLOT(metaBuildPackageList()));

  QList<QTreeWidgetItem *> items;

  ui->twGroups->clear();
  m_hasYaourt = UnixCommand::hasTheExecutable(StrConstants::getForeignRepositoryToolName()) && !UnixCommand::isRootRunning();

  items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList("<" + StrConstants::getDisplayAllGroups() + ">")));
  m_AllGroupsItem = items.at(0);

  if (m_hasYaourt)
  {
    items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(StrConstants::getYaourtGroup())));
    m_YaourtItem = items.at(1);
  }

  const QStringList*const packageGroups = Package::getPackageGroups();
  foreach(QString group, *packageGroups)
  {
    items.append(new QTreeWidgetItem((QTreeWidget*)0, QStringList(group)));
  }
  m_packageRepo.checkAndSetGroups(*packageGroups); // update Package Repository as well
  delete packageGroups;

  ui->twGroups->insertTopLevelItems(0, items);
  ui->twGroups->setCurrentItem(items.at(0));

  connect(ui->twGroups, SIGNAL(itemSelectionChanged()), this, SLOT(metaBuildPackageList()));
}

/*
 * Populates the list of available packages from the given groupName
 */
void MainWindow::buildPackagesFromGroupList(const QString group)
{
  CPUIntensiveComputing cic;

  if (isAllGroupsSelected())
  {
    reapplyPackageFilter();
    m_packageModel->applyFilter(!ui->actionNonInstalledPackages->isChecked(), "<Yaourt>");

    QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
    ui->tvPackages->setCurrentIndex(maux);
    ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
    ui->tvPackages->setCurrentIndex(maux);

    refreshTabInfo();
    refreshTabFiles();
    ui->tvPackages->setFocus();

    return;
  }

  QList<QString> *list = m_listOfPackagesFromGroup;

  QList<QString>::const_iterator it = list->begin();

  m_progressWidget->setRange(0, list->count());
  m_progressWidget->setValue(0);
  m_progressWidget->show();

  int installedCount = 0;
  int counter=0;
  while(it != list->end())
  {
    if (isPackageInstalled(*it)) {
      ++installedCount;
    }

    counter++;

    if (counter % 100 == 0)
    {
      qApp->processEvents();
    }

    m_progressWidget->setValue(counter);
    qApp->processEvents();
    it++;
  }
  m_packageRepo.checkAndSetMembersOfGroup(group, *list);
  m_packageModel->applyFilter(!ui->actionNonInstalledPackages->isChecked(), isAllGroups(group) ? "" : group);

//  resizePackageView();

  //Refresh counters
  m_numberOfInstalledPackages = installedCount;
  //Refresh statusbar widget
  refreshStatusBar();

  /*if (m_leFilterPackage->text() != "")*/ reapplyPackageFilter();

  QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->setCurrentIndex(maux);

  list->clear();
  refreshTabInfo();
  refreshTabFiles();
  ui->tvPackages->setFocus();

  m_progressWidget->close();
  refreshStatusBarToolButtons();
}

void MainWindow::_deleteStandardItemModel(QStandardItemModel * sim)
{
  for(int c=0; c< sim->columnCount(); c++)
  for(int r=0; r< sim->rowCount(); r++)
  {
    delete sim->item(r, c);
  }

  sim->clear();
  delete sim;
}

/*
 * Helper method to deal with the QFutureWatcher result before calling
 * Yaourt package list building method
 */
void MainWindow::preBuildYaourtPackageList()
{
  m_listOfYaourtPackages = g_fwYaourt.result();
  buildYaourtPackageList();

  if (m_cic) {
    delete m_cic;
    m_cic = NULL;
  }

  if (m_packageModel->getPackageCount() == 0)
  {
    m_leFilterPackage->setFocus();
  }
}

/*
 * Helper method to deal with the QFutureWatcher result before calling
 * Yaourt package list building method
 */
void MainWindow::preBuildYaourtPackageListMeta()
{
  m_listOfYaourtPackages = g_fwYaourtMeta.result();
  buildYaourtPackageList();

  if (m_cic) {
    delete m_cic;
    m_cic = 0;
  }

  if (m_packageModel->getPackageCount() == 0)
  {
    m_leFilterPackage->setFocus();
  }
}

/*
 * Helper method to deal with the QFutureWatcher result before calling
 * Pacman package list building method
 */
void MainWindow::preBuildPackageList()
{
  //Just a flag to keep the last "if" from executing twice...
  static bool secondTime=false;
  bool hasToCallSysUpgrade = (m_callSystemUpgrade || m_callSystemUpgradeNoConfirm);

  if (m_listOfPackages) m_listOfPackages->clear();
  m_listOfPackages = g_fwPacman.result();
  buildPackageList();

  if(!hasToCallSysUpgrade && !secondTime && UnixCommand::hasTheExecutable(ctn_MIRROR_CHECK_APP))
  {
    doMirrorCheck();
    secondTime=true;
  }
}

/*
 * Helper method to deal with the QFutureWatcher result before calling
 * Pacman packages from group list building method
 */
void MainWindow::preBuildPackagesFromGroupList()
{
  if (m_listOfPackagesFromGroup) m_listOfPackagesFromGroup->clear();
  GroupMemberPair result = g_fwPacmanGroup.result();
  m_listOfPackagesFromGroup = result.second;
  buildPackagesFromGroupList(result.first);
}

/*
 * Decides which SLOT to call: buildPackageList, buildYaourtPackageList or buildPackagesFromGroupList
 */
void MainWindow::metaBuildPackageList()
{
  ui->twGroups->setEnabled(false);

  if (ui->twGroups->topLevelItemCount() == 0 || isAllGroupsSelected())
  {
    toggleSystemActions(true);
    connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    reapplyPackageFilter();
    disconnect(&g_fwPacman, SIGNAL(finished()), this, SLOT(preBuildPackageList()));
    QFuture<QList<PackageListData> *> f;
    f = QtConcurrent::run(searchPacmanPackages);
    g_fwPacman.setFuture(f);
    connect(&g_fwPacman, SIGNAL(finished()), this, SLOT(preBuildPackageList()));
  }
  else if (isYaourtGroupSelected())
  {
    toggleSystemActions(false);
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    clearStatusBar();

    m_cic = new CPUIntensiveComputing();
    disconnect(&g_fwYaourtMeta, SIGNAL(finished()), this, SLOT(preBuildYaourtPackageListMeta()));
    QFuture<QList<PackageListData> *> f;
    f = QtConcurrent::run(searchYaourtPackages, m_leFilterPackage->text());
    g_fwYaourtMeta.setFuture(f);
    connect(&g_fwYaourtMeta, SIGNAL(finished()), this, SLOT(preBuildYaourtPackageListMeta()));
  }
  else
  {
    toggleSystemActions(true);
    connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
    reapplyPackageFilter();
    disconnect(&g_fwPacmanGroup, SIGNAL(finished()), this, SLOT(preBuildPackagesFromGroupList()));
    QFuture<GroupMemberPair> f;
    f = QtConcurrent::run(searchPacmanPackagesFromGroup, getSelectedGroup());
    g_fwPacmanGroup.setFuture(f);
    connect(&g_fwPacmanGroup, SIGNAL(finished()), this, SLOT(preBuildPackagesFromGroupList()));
  }
}

/*
 * Outputs a list of packages that don't have a description
 */
void MainWindow::_showPackagesWithNoDescription()
{
  bool printHeader = false;
  QList<PackageListData> *list = Package::getPackageList();
  QList<PackageListData>::const_iterator it = list->begin();

  while(it != list->end())
  {
    PackageListData pld = *it;

    if (pld.description == (pld.name + "  "))
    {
      if (!printHeader)
      {
        std::cout << std::endl << "List of packages without description:" << std::endl << std::endl;
        printHeader=true;
      }

      std::cout << pld.name.toLatin1().data() << std::endl;
    }

    it++;
  }
}

/*
 * Populates the list of available packages (installed [+ non-installed])
 *
 * It's called Only: when the selected group is <All> !
 */
void MainWindow::buildPackageList(bool nonBlocking)
{
  CPUIntensiveComputing cic;
  bool hasYaourt = UnixCommand::hasTheExecutable(StrConstants::getForeignRepositoryToolName()) && !UnixCommand::isRootRunning();

  static bool firstTime = true;

  if(!firstTime) //If it's not the starting of the app...
  {
    //Let's get outdatedPackages list again!
    m_outdatedPackageList = Package::getOutdatedPackageList();
    m_numberOfOutdatedPackages = m_outdatedPackageList->count();

    if (hasYaourt)
    {
      m_outdatedYaourtPackageList = Package::getOutdatedYaourtPackageList();
    }
  }

  qApp->processEvents();
  const QSet<QString>*const unrequiredPackageList = Package::getUnrequiredPackageList();

  // fetch package list
  QList<PackageListData> *list;
  if(nonBlocking)
    list = m_listOfPackages;
  else
    list = Package::getPackageList();

  // fetch foreign package list
  QList<PackageListData> *listForeign = Package::getForeignPackageList();
  qApp->processEvents();

  m_progressWidget->setRange(0, list->count());
  m_progressWidget->setValue(0);

  PackageListData pld;
  QList<PackageListData>::const_iterator itForeign = listForeign->begin();
  while (itForeign != listForeign->end())
  {
    if (!hasYaourt || !m_outdatedYaourtPackageList->contains(itForeign->name))
    {
      pld = PackageListData(
            itForeign->name, itForeign->repository, itForeign->version,
            itForeign->name + " " + Package::getInformationDescription(itForeign->name, true),
            ectn_FOREIGN);
    }
    else
    {
      pld = PackageListData(
            itForeign->name, itForeign->repository, itForeign->version,
            itForeign->name + " " + Package::getInformationDescription(itForeign->name, true),
            ectn_FOREIGN_OUTDATED);
    }

    list->append(pld);

    itForeign++;
  }

  m_packageRepo.setData(list, *unrequiredPackageList);
  if (isAllGroupsSelected()) m_packageModel->applyFilter(!ui->actionNonInstalledPackages->isChecked(), "");
  m_progressWidget->show();
  QList<PackageListData>::const_iterator it = list->begin();

  int counter=0;
  int installedCount = 0;

  while(it != list->end())
  {
    if (isPackageInstalled(it->name)) {
      ++installedCount;
    }
    counter++;

    if (counter % 200 == 0)
    {
      qApp->processEvents();
    }

    m_progressWidget->setValue(counter);
    it++;
  }

  m_progressWidget->setValue(list->count());

//  resizePackageView();

  if (m_leFilterPackage->text() != "") reapplyPackageFilter();

  QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->setCurrentIndex(maux);

  list->clear();
  refreshTabInfo();
  refreshTabFiles();

  if (_isPackageTreeViewVisible())
  {
    ui->tvPackages->setFocus();
  }

  //Refresh counters
  m_numberOfInstalledPackages = installedCount;

  //Refresh statusbar widget
  refreshStatusBar();

  //Refresh application icon
  refreshAppIcon();

  m_progressWidget->close();

  if (firstTime)
  {
    if (_isPackageTreeViewVisible())
    {
      m_leFilterPackage->setFocus();
    }

    m_initializationCompleted = true;
    firstTime = false;

    if (m_callSystemUpgrade)
    {
      //First, let us throw away that 'wainting cursor'...
      QApplication::restoreOverrideCursor();
      doSystemUpgrade();
    }
    else if (m_callSystemUpgradeNoConfirm)
    {
      //First, let us throw away that 'wainting cursor'...
      QApplication::restoreOverrideCursor();
      doSystemUpgrade(ectn_NOCONFIRM_OPT);
    }
    else if (m_packagesToInstallList.count() > 0)
    {
      //First, let us throw away that 'wainting cursor'...
      QApplication::restoreOverrideCursor();
      doInstallLocalPackages();
    }
  }

  refreshStatusBarToolButtons();
}

/*
 * Populates the list of found Yaourt packages (installed [+ non-installed])
 * given the searchString parameter passed.
 *
 */
void MainWindow::buildYaourtPackageList()
{
  ui->actionSearchByDescription->setChecked(true);
  tvPackagesSearchColumnChanged(ui->actionSearchByDescription);

  m_progressWidget->show();

  const QSet<QString>*const unrequiredPackageList = Package::getUnrequiredPackageList();
  QList<PackageListData> *list = m_listOfYaourtPackages;

  m_progressWidget->setRange(0, list->count());
  m_progressWidget->setValue(0);

  int counter=0;
  int installedCount = 0;
  QList<PackageListData>::const_iterator it = list->begin();

  while(it != list->end())
  {
    if (isPackageInstalled(it->name)) {
      ++installedCount;
    }
    counter++;
    m_progressWidget->setValue(counter);
    it++;
  }

  m_packageRepo.setAURData(list, *unrequiredPackageList);
  m_packageModel->applyFilter(!ui->actionNonInstalledPackages->isChecked(), "<Yaourt>");

//  resizePackageView();

  QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->setCurrentIndex(maux);

  list->clear();
  refreshTabInfo();
  refreshTabFiles();

  if (_isPackageTreeViewVisible())
  {
    ui->tvPackages->setFocus();
  }

  //Refresh counters
  m_numberOfInstalledPackages = installedCount;

  //Refresh statusbar widget
  refreshStatusBar();

  //Refresh application icon
  refreshAppIcon();

  reapplyPackageFilter();

  counter = list->count();
  m_progressWidget->setValue(counter);
  m_progressWidget->close();

  refreshStatusBarToolButtons();
}

/*
 * Prints the Yaourt toolButton at the left of the statusbar.
 * It warns the user about outdated yaourt packages!
 */
void MainWindow::showToolButtonYaourt()
{
  m_outdatedYaourtPackagesNameVersion = &g_fwOutdatedYaourtPackages.result()->content;

  if(m_outdatedYaourtPackageList->count() > 0 && m_outdatedYaourtPackageList->count() > 0)
  {
    if (m_outdatedYaourtPackageList->count() == 1)
    {
      m_toolButtonYaourt->setText("(1)");
      m_toolButtonYaourt->setToolTip(StrConstants::getOneNewUpdate());
    }
    else
    {
      m_toolButtonYaourt->setText("(" + QString::number(m_outdatedYaourtPackagesNameVersion->count()) + ")");
      m_toolButtonYaourt->setToolTip(
          StrConstants::getNewUpdates().arg(m_outdatedYaourtPackagesNameVersion->count()));
    }

    m_toolButtonYaourt->show();
  }
  else
  {
    m_toolButtonYaourt->setText("");
    m_toolButtonYaourt->setToolTip("");
    m_toolButtonYaourt->hide();
  }

  ui->statusBar->addWidget(m_toolButtonYaourt);
}

/*
 * Refreshes the toolButtons which indicate outdated packages
 */
void MainWindow::refreshStatusBarToolButtons()
{
  QFuture<YaourtOutdatedPackages *> f;
  f = QtConcurrent::run(getOutdatedYaourtPackages);
  g_fwOutdatedYaourtPackages.setFuture(f);
  connect(&g_fwOutdatedYaourtPackages, SIGNAL(finished()), this, SLOT(showToolButtonYaourt()));

  ui->twGroups->setEnabled(true);
}

/*
 * Prints the values of the package counters at the left of the statusBar
 */
void MainWindow::refreshStatusBar()
{
  QString text;

  ui->statusBar->removeWidget(m_toolButtonPacman);
  ui->statusBar->removeWidget(m_toolButtonYaourt);

  if(m_numberOfInstalledPackages > 0)
  {
    text = "| " + StrConstants::getNumberInstalledPackages().arg(m_numberOfInstalledPackages);
  }

  m_lblTotalCounters->setText(text);
  ui->statusBar->addWidget(m_lblTotalCounters);

  if(m_numberOfOutdatedPackages > 0)
  {
    m_toolButtonPacman->show();

    if (m_numberOfOutdatedPackages == 1)
    {
      m_toolButtonPacman->setText("(1)");
      m_toolButtonPacman->setToolTip(StrConstants::getOneNewUpdate());
    }
    else
    {
      m_toolButtonPacman->setText("(" + QString::number(m_numberOfOutdatedPackages) + ")");
      m_toolButtonPacman->setToolTip(StrConstants::getNewUpdates().arg(m_numberOfOutdatedPackages));
    }

    ui->statusBar->addWidget(m_toolButtonPacman);
  }
  else
  {
    m_toolButtonPacman->hide();
    m_toolButtonPacman->setText("");
    m_toolButtonPacman->setToolTip("");
  }
}

/*
 * Clears the counters at the left of the Statusbar
 */
void MainWindow::clearStatusBar()
{
  m_lblSelCounter->clear();
  m_lblTotalCounters->clear();
}

/*
 * Re-populates the HTML view with 'pkgName' package information (tab ONE)
 */
void MainWindow::refreshTabInfo(QString pkgName)
{
  const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkgName);
  if (package == NULL)
    return;

  CPUIntensiveComputing cic;
  QTextBrowser *text = ui->twProperties->widget(
        ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

  if (text)
  {
    text->clear();
    text->setHtml(OctopiTabInfo::formatTabInfo(*package, *m_outdatedYaourtPackagesNameVersion));
    text->scrollToAnchor(OctopiTabInfo::anchorBegin);
  }
}

/*
 * Re-populates the HTML view with selected package's information (tab ONE)
 */
void MainWindow::refreshTabInfo(bool clearContents, bool neverQuit)
{
  static QString strSelectedPackage;

  if(neverQuit == false &&
     (ui->twProperties->currentIndex() != ctn_TABINDEX_INFORMATION || !_isPropertiesTabWidgetVisible())) return;

  QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (clearContents || selectionModel == NULL ||
      selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

    if (text)
    {
      text->clear();
    }

    strSelectedPackage="";
    return;
  }

  QModelIndex item = selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).first();
  const PackageRepository::PackageData*const package = m_packageModel->getData(item);
  if (package == NULL) {
    assert(false);
    return;
  }

  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == package->repository+"#"+package->name+"#"+package->version)
  {
    if (neverQuit)
    {
      _changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
    }

    return;
  }

  CPUIntensiveComputing cic;

  /* Appends all info from the selected package! */
  QString pkgName=package->name;

  if (isYaourtGroupSelected() && package->installed() == false)
  {
    QString aux_desc = package->description;
    int space = aux_desc.indexOf(' ');
    QString pkgDescription = aux_desc.mid(space+1);
    QString version = StrConstants::getVersion();

    QTextBrowser*const text = ui->twProperties->widget(
          ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

    if (text)
    {
      QString html;
      text->clear();
      QString anchorBegin = "anchorBegin";

      html += "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
      html += "<a id=\"" + anchorBegin + "\"></a>";

      html += "<h2>" + pkgName + "</h2>";
      html += "<a style=\"font-size:16px;\">" + pkgDescription + "</a>";

      html += "<table border=\"0\">";
      html += "<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>";

      html += "<tr><td>" + version + "</td><td>" + package->version + "</td></tr>";

      html += "</table>";
      text->setHtml(html);
      text->scrollToAnchor(anchorBegin);
    }
  }
  else //We are not in the Yaourt group
  {
    QTextBrowser *text = ui->twProperties->widget(
          ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>("textBrowser");

    if (text)
    {
      text->clear();
      text->setHtml(OctopiTabInfo::formatTabInfo(*package, *m_outdatedYaourtPackagesNameVersion));
      text->scrollToAnchor(OctopiTabInfo::anchorBegin);
    }
  }

  strSelectedPackage = package->repository+"#"+package->name+"#"+package->version;

  if (neverQuit)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
  }
}

/*
 * Re-populates the treeview which contains the file list of selected package (tab TWO)
 */
void MainWindow::refreshTabFiles(bool clearContents, bool neverQuit)
{
  static QString strSelectedPackage;

  if(neverQuit == false &&
     (ui->twProperties->currentIndex() != ctn_TABINDEX_FILES || !_isPropertiesTabWidgetVisible()))
  {
    return;
  }

  QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (clearContents || selectionModel == NULL ||
      selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTreeView*const tvPkgFileList =
        ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");

    if(tvPkgFileList)
    {
      QStandardItemModel*const modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
      modelPkgFileList->clear();
      strSelectedPackage="";

      bool filterHasFocus = m_leFilterPackage->hasFocus();
      bool tvPackagesHasFocus = ui->tvPackages->hasFocus();
      _closeTabFilesSearchBar();
      if (filterHasFocus) m_leFilterPackage->setFocus();
      else if (tvPackagesHasFocus) ui->tvPackages->setFocus();

      return;
    }
  }

  QModelIndex item = selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).first();
  const PackageRepository::PackageData*const package = m_packageModel->getData(item);
  if (package == NULL) {
    assert(false);
    return;
  }

  //If we are trying to refresh an already displayed package...
  if (strSelectedPackage == package->repository+"#"+package->name+"#"+package->version)
  {
    if (neverQuit)
    {
      _changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
      _selectFirstItemOfPkgFileList();
    }
    else
    {
      QTreeView*const tv = ui->twProperties->currentWidget()->findChild<QTreeView *>("tvPkgFileList") ;
      if (tv)
        tv->scrollTo(tv->currentIndex());
    }

    return;
  }

  //Maybe this is a non-installed package...
  bool nonInstalled = (package->installed() == false);

  QTreeView*const tvPkgFileList =
      ui->twProperties->widget(ctn_TABINDEX_FILES)->findChild<QTreeView*>("tvPkgFileList");

  if (tvPkgFileList)
  {
    QString pkgName = package->name;
    QStringList fileList;
    QStandardItemModel *fakeModelPkgFileList = new QStandardItemModel(this);
    QStandardItemModel *modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());

    modelPkgFileList->clear();
    QStandardItem *fakeRoot = fakeModelPkgFileList->invisibleRootItem();
    QStandardItem *root = modelPkgFileList->invisibleRootItem();
    QStandardItem *lastDir, *item, *lastItem=root, *parent;
    bool first=true;
    lastDir = root;

    fileList = Package::getContents(pkgName, !nonInstalled);

    if (fileList.count() > 0) CPUIntensiveComputing cic;

    QString fullPath;
    bool isSymLinkToDir = false;
    foreach ( QString file, fileList ){
      bool isDir = file.endsWith('/');
      isSymLinkToDir = false;
      QString baseFileName = _extractBaseFileName(file);

      //Let's test if it is not a symbolic link to a dir
      if(!isDir)
      {
        QFileInfo fiTestForSymLink(file);
        if(fiTestForSymLink.isSymLink())
        {
          QFileInfo fiTestForDir(fiTestForSymLink.symLinkTarget());
          isSymLinkToDir = fiTestForDir.isDir();
        }
      }

      if(isDir){
        if ( first == true ){
          item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
          item->setAccessibleDescription("directory " + item->text());
          fakeRoot->appendRow ( item );
        }
        else{
          fullPath = PackageController::showFullPathOfItem(lastDir->index());
          //std::cout << "Testing if " << file.toLatin1().data() << " contains " << fullPath.toLatin1().data() << std::endl;
          if ( file.contains ( fullPath )) {
            //std::cout << "It contains !!! So " << fullPath.toLatin1().data() << " is its parent." << std::endl;
            item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
            item->setAccessibleDescription("directory " + item->text());
            lastDir->appendRow ( item );
          }
          else{
            //std::cout << "It doens't contain..." << std::endl;
            parent = lastItem->parent();
            if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());

            do{
              //if (parent != 0) std::cout << "Testing if " << file.toLatin1().data() << " contains " << fullPath.toLatin1().data() << std::endl;
              if ( parent == 0 || file.contains ( fullPath )) break;
              parent = parent->parent();
              if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());
            }
            while ( parent != fakeRoot );

            item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
            item->setAccessibleDescription("directory " + item->text());

            if ( parent != 0 )
            {
              //std::cout << item->text().toLatin1().data() << " is son of " << fullPath.toLatin1().data() << std::endl;
              parent->appendRow ( item );
            }
            else
            {
              //std::cout << item->text().toLatin1().data() << " is son of <FAKEROOT>" << std::endl;
              fakeRoot->appendRow ( item );
            }
          }
        }

        lastDir = item;
      }            
      else if (isSymLinkToDir)
      {
        item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
        item->setAccessibleDescription("directory " + item->text());
        parent = lastDir;
        if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());

        do{
          //if (parent != 0) std::cout << "Testing if symlink" << file.toLatin1().data() << " contains " << fullPath.toLatin1().data() << std::endl;
          if ( parent == 0 || file.contains ( fullPath )) break;
          parent = parent->parent();
          if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());
        }
        while ( parent != fakeRoot );

        if (parent != 0)
        {
          parent->appendRow ( item );
        }
        else
        {
          fakeRoot->appendRow ( item );
        }
      }
      else
      {
        item = new QStandardItem ( IconHelper::getIconBinary(), baseFileName );
        item->setAccessibleDescription("file " + item->text());
        parent = lastDir;
        if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());

        do{
          if ( parent == 0 || file.contains ( fullPath )) break;
          parent = parent->parent();
          if (parent != 0) fullPath = PackageController::showFullPathOfItem(parent->index());
        }
        while ( parent != fakeRoot );

        parent->appendRow ( item );
      }

      lastItem = item;
      first = false;
    }

    root = fakeRoot;
    fakeModelPkgFileList->sort(0);
    modelPkgFileList = fakeModelPkgFileList;
    tvPkgFileList->setModel(modelPkgFileList);
    tvPkgFileList->header()->setDefaultAlignment( Qt::AlignCenter );
    modelPkgFileList->setHorizontalHeaderLabels( QStringList() <<
                                                 StrConstants::getContentsOf().arg(pkgName));   
  }

  strSelectedPackage = package->repository+"#"+package->name+"#"+package->version;

  if (neverQuit)
  {
    _changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
    _selectFirstItemOfPkgFileList();
  }

  _closeTabFilesSearchBar();
}

/*
 * This SLOT is called every time we press a key at FilterLineEdit
 */
void MainWindow::reapplyPackageFilter()
{
  CPUIntensiveComputing cic;

  bool isFilterPackageSelected = m_leFilterPackage->hasFocus();
  QString search = Package::parseSearchString(m_leFilterPackage->text());

  m_packageModel->applyFilter(search);
  int numPkgs = m_packageModel->getPackageCount();

  if (m_leFilterPackage->text() != ""){
    if (numPkgs > 0) m_leFilterPackage->setFoundStyle();
    else m_leFilterPackage->setNotFoundStyle();
  }
  else{
    m_leFilterPackage->initStyleSheet();;
    m_packageModel->applyFilter("");
  }

  if (isFilterPackageSelected || numPkgs == 0)
  {
    m_leFilterPackage->setFocus();
  }

  ui->tvPackages->selectionModel()->clear();
  QModelIndex mi = m_packageModel->index(0, PackageModel::ctn_PACKAGE_NAME_COLUMN, QModelIndex());
  ui->tvPackages->setCurrentIndex(mi);
  ui->tvPackages->scrollTo(mi);

  //We need to call this method to refresh package selection counters
  tvPackagesSelectionChanged(QItemSelection(),QItemSelection());
  invalidateTabs();
}
