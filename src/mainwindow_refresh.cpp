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

#include <iostream>
#include <cassert>
#include "mainwindow.h"
#include "src/ui/octopitabinfo.h"
#include "searchlineedit.h"
#include "ui_mainwindow.h"
#include "strconstants.h"
#include "uihelper.h"
#include "globals.h"
#include "aurvote.h"
#include "utils.h"

#include <QElapsedTimer>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QComboBox>
#include <QTextBrowser>
#include <QStandardItem>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QSysInfo>
#include <QMessageBox>

/*
 * If we have some outdated packages, let's put an angry red face icon in this app!
 */
void MainWindow::refreshAppIcon()
{
  bool enableSystemUpgrade=false;

  if ((m_outdatedStringList->count() > 0))
  {
    setWindowIcon(IconHelper::getIconOctopiRed());
    if(m_commandExecuting != ectn_MIRROR_CHECK && !isAURGroupSelected()) enableSystemUpgrade=true;
  }
  else if(m_outdatedAURStringList->count() > 0)
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
 * Reconfigure Tools menu according available tools in the system
 */
void MainWindow::refreshMenuTools()
{
  if (UnixCommand::hasTheExecutable(QStringLiteral("mirror-check")))
  {
    ui->menuTools->menuAction()->setVisible(true);
    if (!m_actionMenuMirrorCheck->toolTip().contains(QLatin1String("(")))
      m_actionMenuMirrorCheck->setToolTip(m_actionMenuMirrorCheck->toolTip() + QLatin1String("  (") + m_actionMenuMirrorCheck->shortcut().toString() + QLatin1Char(')') );
    m_actionMenuMirrorCheck->setVisible(true);
  }
  else
    m_actionMenuMirrorCheck->setVisible(false);

  if(UnixCommand::hasTheExecutable(QStringLiteral("plv")))
  {
    static bool connectorPlv=false;

    ui->menuTools->menuAction()->setVisible(true);
    ui->actionPacmanLogViewer->setVisible(true);
    ui->actionPacmanLogViewer->setIcon(QIcon::fromTheme(QStringLiteral("plv")));

    if (!connectorPlv)
    {
      connect(ui->actionPacmanLogViewer, SIGNAL(triggered()), this, SLOT(launchPLV()));
      connectorPlv=true;
    }
  }
  else
    ui->actionPacmanLogViewer->setVisible(false);

  if(UnixCommand::hasTheExecutable(QStringLiteral("octopi-repoeditor")) && UnixCommand::getLinuxDistro() != ectn_KAOS)
  {
    static bool connectorRepo=false;

    ui->menuTools->menuAction()->setVisible(true);
    ui->actionRepositoryEditor->setVisible(true);

    if (!connectorRepo)
    {
      connect(ui->actionRepositoryEditor, SIGNAL(triggered()), this, SLOT(launchRepoEditor()));
      connectorRepo=true;
    }
  }
  else
    ui->actionRepositoryEditor->setVisible(false);

  if(UnixCommand::hasTheExecutable(QStringLiteral("octopi-cachecleaner")))
  {
    static bool connectorCleaner=false;

    ui->menuTools->menuAction()->setVisible(true);
    ui->actionCacheCleaner->setVisible(true);

    if (!connectorCleaner)
    {
      connect(ui->actionCacheCleaner, SIGNAL(triggered()), this, SLOT(launchCacheCleaner()));
      connectorCleaner=true;
    }
  }
  else
    ui->actionCacheCleaner->setVisible(false);

  ui->menuTools->menuAction()->setVisible(true);
  if (ui->menuTools->actions().indexOf(m_actionSysInfo) == -1)
  {
    static bool connectorSysInfo=false;

    ui->menuTools->addSeparator();
    m_actionSysInfo->setText(QStringLiteral("SysInfo..."));
    ui->menuTools->addAction(m_actionSysInfo);

    if (!connectorSysInfo)
    {
      connect(m_actionSysInfo, SIGNAL(triggered()), this, SLOT(doSysInfo()));
      connectorSysInfo=true;
    }
  }

  /*for (QAction * act,  ui->menuBar->actions())
  {
    QString text = act->text();
    text = text.remove("&");
    act->setText(qApp->translate("MainWindow", text.toUtf8(), 0));
  }*/
}

/*
 * Inserts the group names into the Groups treeView
 */
void MainWindow::refreshGroupsWidget()
{
  disconnect(ui->twGroups, SIGNAL(itemSelectionChanged()), this, SLOT(groupItemSelected()));

  QList<QTreeWidgetItem *> items;
  ui->twGroups->clear();

  items.append(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr), QStringList(QLatin1Char('<') + StrConstants::getDisplayAllGroups() + QLatin1Char('>'))));
  m_AllGroupsItem = items.at(0);
  const QStringList*const packageGroups = Package::getPackageGroups();
  for(const QString& group: *packageGroups)
  {
    items.append(new QTreeWidgetItem(static_cast<QTreeWidget*>(nullptr), QStringList(group)));
  }
  m_packageRepo.checkAndSetGroups(*packageGroups); // update Package Repository as well
  delete packageGroups;

  ui->twGroups->insertTopLevelItems(0, items);
  ui->twGroups->setCurrentItem(items.at(0));
  connect(ui->twGroups, SIGNAL(itemSelectionChanged()), this, SLOT(groupItemSelected()));
}

/*
 * User clicked AUR tool button in the toolbar
 */
void MainWindow::AURToolSelected()
{
  //If the system does not have any AUR tool, let's ask if user wants to get one
  if (m_actionSwitchToForeignTool->toolTip() == QLatin1String("AUR") && UnixCommand::getAvailableAURTools().count() ==1)
  {
    //Are we inside a 64bit platform?
    if (QSysInfo::buildCpuArchitecture() == QLatin1String("x86_64"))
    {
      //We can offer to install a temporary yay-bin and then get yay-bin package
      QMessageBox::StandardButton r = QMessageBox::question(this, StrConstants::getConfirmation(),
                            StrConstants::getDoYouWantToInstallYayTool(),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::Yes);
      if (r == QMessageBox::Yes)
      {
        doPreDownloadTempYay();
      }
    }
    else
    {
      QMessageBox::information(this,
                               StrConstants::getInformation(),
                               StrConstants::getYoullNeedToInstallAURTool());
    }

    return;
  }
  else if (m_actionSwitchToForeignTool->toolTip() == QLatin1String("AUR") && UnixCommand::getAvailableAURTools().count() >1)
  {
    onOptions(ectn_TAB_AUR);
    return;
  }

  bool lightPackageFilterConnected = false;
  static QStandardItemModel emptyModel;
  //if (UnixCommand::getLinuxDistro() != ectn_CHAKRA) savePackageColumnWidths();
  refreshTabInfo(true);
  refreshTabFiles(true);

  m_progressWidget->setRange(0, 100);
  m_progressWidget->setValue(0);
  m_progressWidget->show();

  //Here we are changing view to list AUR packages ONLY
  if (m_actionSwitchToForeignTool->isChecked())
  {
    ui->tvPackages->setModel(&emptyModel);
    //ui->twProperties->setTabEnabled(ctn_TABINDEX_ACTIONS, false);

    if ((UnixCommand::getLinuxDistro() != ectn_KAOS && ui->actionUseInstantSearch->isChecked()) ||
         (UnixCommand::getLinuxDistro() == ectn_KAOS && !ui->actionUseInstantSearch->isChecked()))
    {
      disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
      connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
      lightPackageFilterConnected = true;
    }
    else if (UnixCommand::getLinuxDistro() == ectn_KAOS && ui->actionUseInstantSearch->isChecked())
    {
      disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      lightPackageFilterConnected = false;
    }

    if (UnixCommand::getLinuxDistro() != ectn_KAOS) ui->actionUseInstantSearch->setEnabled(false);

    m_refreshForeignPackageList = false;
    m_actionMenuRepository->setEnabled(false);
    ui->twGroups->setEnabled(false);           
    ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_REPOSITORY_COLUMN, true);

    if (!SettingsManager::hasPacmanBackend())
      ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_SIZE_COLUMN, true);

    ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_POPULARITY_COLUMN, false);
  }
  //Here we are changing view to list all packages
  else
  {
    ui->actionUseInstantSearch->setEnabled(true);
    //ui->twProperties->setTabEnabled(ctn_TABINDEX_ACTIONS, true);

    if (UnixCommand::getLinuxDistro() != ectn_KAOS && ui->actionUseInstantSearch->isChecked())
    {
      disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
      disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      lightPackageFilterConnected = false;
    }
    else if (UnixCommand::getLinuxDistro() == ectn_KAOS && !ui->actionUseInstantSearch->isChecked())
    {
      disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
      disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
      lightPackageFilterConnected = true;
    }

    m_packageModel->applyFilter(QLatin1String(""));
    ui->tvPackages->setModel(&emptyModel);
    removePackageTreeViewConnections();
    //m_actionSwitchToAURTool->setEnabled(false);
    m_refreshForeignPackageList = true;
    m_actionMenuRepository->setEnabled(true);
    ui->twGroups->setEnabled(true);
    ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_POPULARITY_COLUMN, true);
    ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_REPOSITORY_COLUMN, false);

    if (!SettingsManager::hasPacmanBackend())
      ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_SIZE_COLUMN, false);

    clearStatusBar();
  }

  m_cachedPackageInInfo=QLatin1String("");

  //Let's clear the list of visited packages (pkg anchors in Info tab)
  m_listOfVisitedPackages.clear();
  m_indOfVisitedPackage = 0;

  switchToViewAllPackages();
  m_selectedRepository = QLatin1String("");
  m_actionRepositoryAll->setChecked(true);
  m_refreshPackageLists = false;

  if (lightPackageFilterConnected && !ui->actionUseInstantSearch->isChecked())
    disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));

  if (lightPackageFilterConnected && !ui->actionUseInstantSearch->isChecked())
    connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));

  metaBuildPackageList();
  refreshInfoAndFileTabs();
}

/*
 * Whenever user selects another package group...
 */
void MainWindow::groupItemSelected()
{
  //Let us select ALL pkgs from ALL repos!
  switchToViewAllPackages();
  m_selectedRepository = QLatin1String("");
  m_actionRepositoryAll->setChecked(true);

  if (m_actionShowGroups->isChecked() && isAllGroupsSelected())
  {
    m_refreshPackageLists = false;
    m_groupWidgetNeedsFocus = true;
  }
  else
  {
    m_refreshPackageLists = true;
  }

  m_leFilterPackage->clear();
  metaBuildPackageList();
}

/*
 * Given the package name that owns the file user has just searched, goes to that package in the list
 */
void MainWindow::positionInPkgListSearchByFile()
{
  if (m_cic) {
    delete m_cic;
    m_cic = nullptr;
  }

  QString pkgName = g_fwPackageOwnsFile.result();

  if (!pkgName.isEmpty())
  {
    QModelIndex searchColumn = m_packageModel->index(0,
                                                     PackageModel::ctn_PACKAGE_NAME_COLUMN,
                                                     QModelIndex());
    QModelIndexList fi = m_packageModel->match(searchColumn, Qt::DisplayRole, pkgName, -1, Qt::MatchExactly);

    if (fi.count() >= 1)
    {
      ui->tvPackages->setCurrentIndex(fi.at(0));
      ui->tvPackages->scrollTo(fi.at(0), QAbstractItemView::PositionAtCenter);
    }
  }
  else //The pkg was not found, so we position on the first item of the list!
  {
    ui->tvPackages->setCurrentIndex(m_packageModel->index(0,0,QModelIndex()));
  }
}

/*
 * Populates the list of available packages from the given groupName
 */
void MainWindow::buildPackagesFromGroupList(const QString &group)
{
  CPUIntensiveComputing cic;
  const QList<QString>*const list = m_listOfPackagesFromGroup.get();
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
    m_progressWidget->setValue(counter);
    ++it;
  }

  counter = list->count();
  m_progressWidget->setValue(counter);
  m_progressWidget->close();

  m_packageRepo.checkAndSetMembersOfGroup(group, *list);
  m_packageModel->applyFilter(m_selectedViewOption, m_selectedRepository, isAllGroups(group) ? QLatin1String("") : group);

  //Refresh counters
  m_numberOfInstalledPackages = installedCount;
  //Refresh statusbar widget
  refreshStatusBar();

  reapplyPackageFilter();

  QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->setCurrentIndex(maux);

  m_listOfPackagesFromGroup.reset();

  refreshColumnSortSetup();
  refreshToolBar();
  refreshStatusBarToolButtons();

  tvPackagesSelectionChanged(QItemSelection(),QItemSelection());
  ui->twGroups->setFocus();
}

/*
 * Helper method to deal with the QFutureWatcher result before calling
 * AUR package list building method
 */
void MainWindow::preBuildAURPackageList()
{
  m_time->start();

  m_listOfAURPackages = g_fwAUR.result();
  buildAURPackageList();

  if (m_cic) {
    delete m_cic;
    m_cic = nullptr;
  }

  invalidateTabs();

  if (m_packageModel->getPackageCount() == 0)
  {
    m_leFilterPackage->setFocus();
  }

  emit buildAURPackageListDone();
}

/*
 * Executes QFuture to retrieve Foreign list of packages
 */
void MainWindow::retrieveForeignPackageList()
{
  QEventLoop el;
  QFuture<QList<PackageListData> *> f;
  f = QtConcurrent::run(searchForeignPackages);
  connect(&g_fwForeignPacman, SIGNAL(finished()), this, SLOT(preBuildForeignPackageList()));
  connect(&g_fwForeignPacman, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwForeignPacman.setFuture(f);
  el.exec();

  assert(m_foreignPackageList != nullptr);
}

/*
 * Executes QFuture to retrieve Unrequired Pacman list of packages
 */
void MainWindow::retrieveUnrequiredPackageList()
{
  QEventLoop el;
  QFuture<QSet<QString> *> f;
  f = QtConcurrent::run(searchUnrequiredPacmanPackages);
  connect(&g_fwUnrequiredPacman, SIGNAL(finished()), this, SLOT(preBuildUnrequiredPackageList()));
  connect(&g_fwUnrequiredPacman, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwUnrequiredPacman.setFuture(f);
  el.exec();

  assert(m_unrequiredPackageList != nullptr);
}

/*
 * Executes QFuture to retrieve Outdated Pacman/AUR list of packages
 */
void MainWindow::retrieveOutdatedPackageList()
{
  QEventLoop el;
  QFuture<QStringList *> f;
  f = QtConcurrent::run(getOutdatedPkgStringList);
  connect(&g_fwOutdatedPkgStringList, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwOutdatedPkgStringList.setFuture(f);
  el.exec();

  m_outdatedStringList = g_fwOutdatedPkgStringList.result();
  m_numberOfOutdatedPackages = m_outdatedStringList->count();
}

/*
 * Helper method to assign QFuture for outdated AUR list of packages
 */
void MainWindow::preBuildForeignPackageList()
{
  m_foreignPackageList = g_fwForeignPacman.result();

  if(m_debugInfo)
    std::cout << "Time elapsed obtaining Foreign pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;
}

/*
 * Helper method to assign QFuture for Unrequired Pacman list of packages
 */
void MainWindow::preBuildUnrequiredPackageList()
{
  m_unrequiredPackageList = g_fwUnrequiredPacman.result();

  if(m_debugInfo)
    std::cout << "Time elapsed obtaining Unrequired pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;
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

  m_listOfPackages.reset(g_fwPacman.result());

  if(m_debugInfo)
    std::cout << "Time elapsed obtaining pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;

  buildPackageList();

  if(!hasToCallSysUpgrade && !secondTime && UnixCommand::getLinuxDistro() != ectn_CHAKRA && m_hasMirrorCheck)
  {
#ifdef OCTOPI_DEV_CODE
    if (!SettingsManager::getSkipMirrorCheckAtStartup())
      doMirrorCheck();
#else
    doMirrorCheck();
#endif

    secondTime=true;
  }

  toggleSystemActions(true);

  emit buildPackageListDone();
}

/*
 * Helper method to deal with the QFutureWatcher result before calling
 * Pacman packages from group list building method
 */
void MainWindow::preBuildPackagesFromGroupList()
{
  GroupMemberPair result = g_fwPacmanGroup.result();
  m_listOfPackagesFromGroup.reset(result.second);
  buildPackagesFromGroupList(result.first);

  toggleSystemActions(true);

  emit buildPackagesFromGroupListDone();
}

/*
 * Decides which SLOT to call: buildPackageList, buildAURPackageList or buildPackagesFromGroupList
 */
void MainWindow::metaBuildPackageList()
{
  static bool firstTime = true;

  if (!firstTime) m_time->start();

  if (isSearchByFileSelected())
    m_leFilterPackage->setRefreshValidator(ectn_FILE_VALIDATOR);
  else if (isAURGroupSelected())
    m_leFilterPackage->setRefreshValidator(ectn_AUR_VALIDATOR);
  else
    m_leFilterPackage->setRefreshValidator(ectn_DEFAULT_VALIDATOR);

  m_packageModel->setShowColumnPopularity(false);
  ui->twGroups->setEnabled(false);
  ui->tvPackages->setSelectionMode(QAbstractItemView::ExtendedSelection);

  //Here we build the list of all available packages
  if (ui->twGroups->topLevelItemCount() == 0 || isAllGroupsSelected())
  {        
    ui->actionSearchByFile->setEnabled(true);
    //ui->actionSearchByName->setChecked(true);
    m_actionLastSearchMethod->setChecked(true);
    if (ui->actionSearchByFile->isChecked()) m_leFilterPackage->setRefreshValidator(ectn_FILE_VALIDATOR);
    else if (ui->actionSearchByName->isChecked()) m_leFilterPackage->setRefreshValidator(ectn_DEFAULT_VALIDATOR);

    toggleSystemActions(false);
    disconnect(&g_fwPacman, SIGNAL(finished()), this, SLOT(preBuildPackageList()));

    if(m_debugInfo)
      std::cout << m_packageModel->getPackageCount() << " pkgs => " <<
                 "Time elapsed before building pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;

    if (m_cic == nullptr)
    {
      m_cic = new CPUIntensiveComputing;
    }

    QEventLoop el;
    QFuture<QList<PackageListData> *> f;
    f = QtConcurrent::run(searchPacmanPackages, m_checkUpdatesNameNewVersion);
    connect(&g_fwPacman, SIGNAL(finished()), this, SLOT(preBuildPackageList()));
    disconnect(this, SIGNAL(buildPackageListDone()), &el, SLOT(quit()));
    connect(this, SIGNAL(buildPackageListDone()), &el, SLOT(quit()));

    g_fwPacman.setFuture(f);
    el.exec(QEventLoop::ExcludeUserInputEvents);

    if(m_debugInfo)
      std::cout << m_packageModel->getPackageCount() << " pkgs => " <<
                 "Time elapsed building pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;
  }
  //Here we build the list of all AUR/KCP packages
  else if (isAURGroupSelected())
  {
    m_toolButtonPacman->hide();
    m_toolButtonAUR->hide();
    switchToViewAllPackages();
    ui->actionSearchByFile->setEnabled(false);
    m_packageModel->setShowColumnPopularity(true);

    if (UnixCommand::getLinuxDistro() == ectn_KAOS)
    {
      m_leFilterPackage->setRefreshValidator(ectn_DEFAULT_VALIDATOR);
      ui->tvPackages->setSelectionMode(QAbstractItemView::SingleSelection);

      toggleSystemActions(false);
      m_listOfAURPackages = new QList<PackageListData>();
      m_leFilterPackage->setFocus();
      ui->twGroups->setEnabled(false);

      QEventLoop el;
      QFuture<QList<PackageListData> *> f;
      disconnect(&g_fwAUR, SIGNAL(finished()), this, SLOT(preBuildAURPackageList()));
      m_cic = new CPUIntensiveComputing();
      f = QtConcurrent::run(searchForeignToolPackages, m_leFilterPackage->text());
      connect(&g_fwAUR, SIGNAL(finished()), this, SLOT(preBuildAURPackageList()));
      disconnect(this, SIGNAL(buildAURPackageListDone()), &el, SLOT(quit()));
      connect(this, SIGNAL(buildAURPackageListDone()), &el, SLOT(quit()));

      g_fwAUR.setFuture(f);
      el.exec();

      if(m_debugInfo)
        std::cout << m_packageModel->getPackageCount() << " pkgs => " <<
                   "Time elapsed building pkgs from '" <<
                     StrConstants::getForeignPkgRepositoryName().toLatin1().data() <<
                     " group' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;

      return;
    }

    toggleSystemActions(false);
    //disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter())); //WATCH OUT!
    clearStatusBar();

    m_cic = new CPUIntensiveComputing();       

    if(!m_leFilterPackage->text().isEmpty())
    {
      m_toolButtonPacman->hide();
      //disconnect(&g_fwAURMeta, SIGNAL(finished()), this, SLOT(preBuildAURPackageListMeta()));
      disconnect(&g_fwAUR, SIGNAL(finished()), this, SLOT(preBuildAURPackageList()));

      QFuture<QList<PackageListData> *> f;
      f = QtConcurrent::run(searchForeignToolPackages, m_leFilterPackage->text());
      //connect(&g_fwAURMeta, SIGNAL(finished()), this, SLOT(preBuildAURPackageListMeta()));
      connect(&g_fwAUR, SIGNAL(finished()), this, SLOT(preBuildAURPackageList()));
      //g_fwAURMeta.setFuture(f);
      g_fwAUR.setFuture(f);
    }
    else
    {
      m_listOfAURPackages = new QList<PackageListData>();
      buildAURPackageList();

      if (m_cic)
      {
        delete m_cic;
        m_cic = nullptr;
      }

      m_leFilterPackage->setFocus();
    }
  }
  //Here we build the list of packages available in the selected package group
  else
  {
    ui->actionSearchByFile->setEnabled(false);
    toggleSystemActions(false);
    disconnect(&g_fwPacmanGroup, SIGNAL(finished()), this, SLOT(preBuildPackagesFromGroupList()));

    QEventLoop el;
    QFuture<GroupMemberPair> f;
    f = QtConcurrent::run(searchPacmanPackagesFromGroup, getSelectedGroup());
    connect(&g_fwPacmanGroup, SIGNAL(finished()), this, SLOT(preBuildPackagesFromGroupList()));
    disconnect(this, SIGNAL(buildPackagesFromGroupListDone()), &el, SLOT(quit()));
    connect(this, SIGNAL(buildPackagesFromGroupListDone()), &el, SLOT(quit()));

    g_fwPacmanGroup.setFuture(f);
    el.exec();

    if(m_debugInfo)
      std::cout << m_packageModel->getPackageCount() << " pkgs => " <<
                 "Time elapsed building pkgs from '" << getSelectedGroup().toLatin1().data() <<
                   " group' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;
  }

  firstTime = false;
}

/*
 * Outputs a list of packages that don't have a description
 */
void MainWindow::showPackagesWithNoDescription()
{
  bool printHeader = false;
  QList<PackageListData> *list = Package::getPackageList(QLatin1String(""), m_checkUpdatesNameNewVersion);
  QList<PackageListData>::const_iterator it = list->cbegin();

  while(it != list->cend())
  {
    PackageListData pld = *it;

    if (pld.description == (pld.name + QLatin1String("  ")))
    {
      if (!printHeader)
      {
        std::cout << std::endl << "List of packages without description:" << std::endl << std::endl;
        printHeader=true;
      }

      std::cout << pld.name.toLatin1().data() << std::endl;
    }

    ++it;
  }
}

/*
 * Populates the list of available packages (installed [+ non-installed])
 *
 * It's called Only: when the selected group is <All> !
 */
void MainWindow::buildPackageList()
{
  disconnect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(invalidateTabs()));
  connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(invalidateTabs()));

  ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_POPULARITY_COLUMN, true);

  static bool firstTime = true;
  bool searchOutdatedPackages=SettingsManager::getSearchOutdatedAURPackages();
  if (!searchOutdatedPackages)
  {
    m_outdatedAURStringList->clear();
    m_outdatedAURPackagesNameVersion->clear();
  }

  if(m_refreshPackageLists) //If it's not the starting of the app...
  {
    //Let's get outdatedPackages list again!
    m_outdatedStringList = Package::getOutdatedStringList();
    if (m_outdatedStringList->count() < m_checkupdatesStringList->count())
      m_outdatedStringList = m_checkupdatesStringList;

    if(m_debugInfo)
      std::cout << "Time elapsed refreshing outdated pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;

    m_numberOfOutdatedPackages = m_outdatedStringList->count();
    if (m_numberOfOutdatedPackages == 0 && m_checkupdatesStringList->count() > 0)
      m_numberOfOutdatedPackages = m_checkupdatesStringList->count();

    m_unrequiredPackageList->clear();
    delete m_unrequiredPackageList;
    m_unrequiredPackageList = nullptr;

    m_unrequiredPackageList = Package::getUnrequiredPackageList();

    if(m_debugInfo)
      std::cout << "Time elapsed obtaining unrequired pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;
  }

  // Fetch package list
  QList<PackageListData> *list;
  list = m_listOfPackages.release();

  if (!isSearchByFileSelected())
  {
    if (!m_refreshPackageLists)
    {
      if(m_debugInfo)
        std::cout << "Time elapsed setting outdated foreign pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;
    }  

    if (m_hasForeignTool && m_refreshForeignPackageList)
    {
      m_foreignPackageList->clear();
      delete m_foreignPackageList;
      m_foreignPackageList = nullptr;

      m_foreignPackageList = markForeignPackagesInPkgList(m_hasForeignTool, m_outdatedAURStringList);

      list->append(*m_foreignPackageList);

      if(m_debugInfo)
        std::cout << "Time elapsed obtaining outdated foreign pkgs from 'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;
    }
  }

  m_progressWidget->setRange(0, list->count());
  m_progressWidget->setValue(0);
  m_progressWidget->show();

  int counter=0;
  QList<PackageListData>::const_iterator it = list->cbegin();

  while(it != list->cend())
  {
    counter++;
    m_progressWidget->setValue(counter);
    ++it;
  }

  if(m_debugInfo)
    std::cout << "Time elapsed iterating over all pkgs from  'ALL group' list: " << m_time->elapsed() << " mili seconds." << std::endl;

  counter = list->count();
  m_progressWidget->setValue(counter);
  m_progressWidget->close();

  m_packageRepo.setData(list, *m_unrequiredPackageList);

  if (ui->tvPackages->model() != m_packageModel.get())
  {
    ui->tvPackages->setModel(m_packageModel.get());
    removePackageTreeViewConnections();
    initPackageTreeView();
    ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_POPULARITY_COLUMN, true);
  }

  if(m_debugInfo)
    std::cout << "Time elapsed setting the list to the treeview: " << m_time->elapsed() << " mili seconds." << std::endl;

  if (ui->actionSearchByDescription->isChecked())
  {
    m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_DESCRIPTION_FILTER_NO_COLUMN);
  }
  else
  {
    m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_NAME_COLUMN);
  }

  if (isAllGroupsSelected()) m_packageModel->applyFilter(m_selectedViewOption, m_selectedRepository, QLatin1String(""));
  //if (m_leFilterPackage->text() != QLatin1String("")) reapplyPackageFilter();

  //reapplyPackageFilter(); DANGER

  if(m_debugInfo)
    std::cout << "Time elapsed after applying filters to the treeview: " << m_time->elapsed() << " mili seconds." << std::endl;

  QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->setCurrentIndex(maux);

  list->clear();
  delete list;
  list = nullptr;

  refreshColumnSortSetup();

  //Refresh statusbar widget
  refreshStatusBar();

  //Refresh application icon
  refreshAppIcon();

  if (firstTime)
  {
    if (!isNotifierBusy())
    {
      UnixCommand::removeSharedMemFiles();
    }

    m_initializationCompleted = true;
    firstTime = false;

    if(SettingsManager::getEnableAURVoting())
    {
      m_aurVote = new AurVote(this);
      if (m_debugInfo) m_aurVote->turnDebugInfoOn();
      m_aurVote->setUserName(SettingsManager::getAURUserName());
      m_aurVote->setPassword(SettingsManager::getAURPassword());
      m_aurVote->login();
    }

    refreshHelpUsageText();

    if (m_outdatedStringList->count() > 0)
    {
      if(m_debugInfo)
        std::cout << "Time elapsed until calling PACMAN_SUP: " << m_time->elapsed() << " mili seconds." << std::endl;

      QStringList sl;
      sl << QStringLiteral("--print-format") << QStringLiteral("%n %v %s") << QStringLiteral("-Spu");
      execCommandInAnotherThread(QStringLiteral("pacman"), sl);
    }

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

    //reapplyPackageFilter(); //DANGER
  }// end of firstime

  reapplyPackageFilter();
  resizePackageView();
  refreshToolBar();

  m_refreshPackageLists = true;
  m_refreshForeignPackageList = true;

  if(m_debugInfo)
    std::cout << "Time elapsed after refreshToolBar(): " << m_time->elapsed() << " mili seconds." << std::endl;

  if (searchOutdatedPackages) m_outdatedAURTimer->start();
  else
  {
    if (m_hasForeignTool && !m_actionSwitchToForeignTool->isEnabled()) m_actionSwitchToForeignTool->setEnabled(true);

    QModelIndex mi = ui->tvPackages->currentIndex();
    m_packageRepo.setAUROutdatedData(m_foreignPackageList, *m_outdatedAURStringList);
    std::cout << "Time elapsed after setting AURoutdatedData: " << m_time->elapsed() << " mili seconds." << std::endl;

    ui->tvPackages->setCurrentIndex(mi);

    //std::cout << "Time elapsed after setting current index: " << m_time->elapsed() << " mili seconds." << std::endl;
    m_leFilterPackage->setFocus();
  }

  if (!ui->twGroups->isEnabled()) ui->twGroups->setEnabled(true);

  if (m_cic != nullptr)
  {
    delete m_cic;
    m_cic = nullptr;
  }

  //changePackageListModel(ectn_INSTALLED_PKGS, QStringLiteral("garuda"));
}

void MainWindow::refreshOutdatedPackageList()
{
  m_packageRepo.setOutdatedData(*m_checkUpdatesNameNewVersion);

  if (m_outdatedStringList->count() < m_checkupdatesStringList->count())
    m_outdatedStringList = m_checkupdatesStringList;

  m_numberOfOutdatedPackages = m_outdatedStringList->count();
  if (m_numberOfOutdatedPackages == 0 && m_checkupdatesStringList->count() > 0)
    m_numberOfOutdatedPackages = m_checkupdatesStringList->count();

  refreshColumnSortSetup();
  refreshStatusBar();
  refreshAppIcon();

  QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->setCurrentIndex(maux);
}


/*
 * Whenever horizontal splitter handler is moved
 */
void MainWindow::horizontalSplitterMoved(int pos, int index)
{
  Q_UNUSED(index)

  QList<int> rl;
  rl = ui->splitterHorizontal->sizes();

  if (pos == 0 && rl[0] == 0)
  {
    ui->twProperties->tabBar()->hide();
    //ui->mainToolBar->hide();
  }
  else
  {
    ui->twProperties->tabBar()->show();
    //ui->mainToolBar->show();
  }
}

/*
 * This slot is called just after the list of packages is constructed
 */
void MainWindow::postBuildPackageList()
{
  m_outdatedAURTimer->stop();
  LinuxDistro distro = UnixCommand::getLinuxDistro();

  if (distro != ectn_KAOS && isAURGroupSelected()) return;

  m_reconectSlotInvalidateTabs = false;
  //bool reconnectSlot = false;

  if (ui->twProperties->currentIndex() == ctn_TABINDEX_FILES)
  {
    disconnect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
               this, SLOT(invalidateTabs()));

    //reconnectSlot = true;
    m_reconectSlotInvalidateTabs = true;
  }

  refreshOutdatedAURStringList();
}

/*
 * Refreshes the list of outdated AUR packages
 */
void MainWindow::refreshOutdatedAURStringList()
{
  QFuture<QStringList *> f = QtConcurrent::run(getOutdatedAURStringList);
  connect(&g_fwOutdatedAURStringList, SIGNAL(finished()), this, SLOT(postRefreshOutdatedAURStringList()));
  g_fwOutdatedAURStringList.setFuture(f);
}

void MainWindow::postRefreshOutdatedAURStringList()
{
  m_outdatedAURStringList = g_fwOutdatedAURStringList.result();
  m_foreignPackageList->clear();
  delete m_foreignPackageList;
  m_foreignPackageList = nullptr;
  m_foreignPackageList = markForeignPackagesInPkgList(m_hasForeignTool, m_outdatedAURStringList);

  LinuxDistro distro = UnixCommand::getLinuxDistro();
  if (distro != ectn_KAOS && isAURGroupSelected()) return;

  QModelIndex mi = ui->tvPackages->currentIndex();
  m_packageRepo.setAUROutdatedData(m_foreignPackageList, *m_outdatedAURStringList);
  ui->tvPackages->setCurrentIndex(mi);

  if (m_outdatedStringList->count() == 0 && m_outdatedAURStringList->count() > 0)
    refreshAppIcon();

  refreshStatusBarToolButtons();

  if (m_reconectSlotInvalidateTabs)
  {
    connect(ui->tvPackages->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(invalidateTabs()));
  }

  if (m_commandExecuting == ectn_NONE && !m_actionSwitchToForeignTool->isEnabled()) m_actionSwitchToForeignTool->setEnabled(true);

  if (m_groupWidgetNeedsFocus && ui->splitterVertical->sizes().at(1) != 0) //if group is not hidden...
  {
    ui->twGroups->setFocus();
    m_groupWidgetNeedsFocus = false;
  }
  else
  {
    if (SettingsManager::getSearchOutdatedAURPackages())
      m_leFilterPackage->setFocus();
  }
}

/*
 * Whenever a package list is built, we refresh its column sort setup
 */
void MainWindow::refreshColumnSortSetup()
{
  if (isAURGroupSelected())
  {
    int packageListOrderedCol = SettingsManager::instance()->getForeignPackageListOrderedCol();
    Qt::SortOrder packageListSortOrder = (Qt::SortOrder) SettingsManager::instance()->getForeignPackageListSortOrder();

    ui->tvPackages->header()->setSortIndicator( packageListOrderedCol, packageListSortOrder );
    ui->tvPackages->sortByColumn( packageListOrderedCol, packageListSortOrder );
  }
  else
  {
    int packageListOrderedCol = SettingsManager::instance()->getPackageListOrderedCol();
    Qt::SortOrder packageListSortOrder = (Qt::SortOrder) SettingsManager::instance()->getPackageListSortOrder();

    ui->tvPackages->header()->setSortIndicator( packageListOrderedCol, packageListSortOrder );
    ui->tvPackages->sortByColumn( packageListOrderedCol, packageListSortOrder );
  }
}

/*
 * Populates the list of found AUR packages (installed [+ non-installed])
 * given the searchString parameter passed.
 *
 */
void MainWindow::buildAURPackageList()
{
  ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_REPOSITORY_COLUMN, true);
  ui->actionSearchByDescription->setChecked(true);
  m_leFilterPackage->setRefreshValidator(ectn_AUR_VALIDATOR);
  m_progressWidget->show();

  const QSet<QString>*const unrequiredPackageList = Package::getUnrequiredPackageList();
  QList<PackageListData> *list = m_listOfAURPackages;

  m_progressWidget->setRange(0, list->count());
  m_progressWidget->setValue(0);
  int counter=0;
  int installedCount = 0;
  QList<PackageListData>::const_iterator it = list->cbegin();

  while(it != list->cend())
  {
    if (isPackageInstalled(it->name)) {
      ++installedCount;
    }
    counter++;
    m_progressWidget->setValue(counter);
    ++it;
  }

  m_packageRepo.setAURData(list, *unrequiredPackageList);
  if (ui->tvPackages->model() != m_packageModel.get())
  {
    ui->tvPackages->setModel(m_packageModel.get());
    removePackageTreeViewConnections();
    initPackageTreeView();
    ui->tvPackages->setColumnHidden(PackageModel::ctn_PACKAGE_REPOSITORY_COLUMN, true);
  }

  m_packageModel->applyFilter(m_selectedViewOption, m_selectedRepository, StrConstants::getForeignToolGroup());
  m_packageModel->applyFilter(PackageModel::ctn_PACKAGE_DESCRIPTION_FILTER_NO_COLUMN);

  if(m_debugInfo)
    std::cout << m_packageModel->getPackageCount() << " pkgs => " <<
               "Time elapsed building pkgs from 'AUR' list: " << m_time->elapsed() << " mili seconds." << std::endl << std::endl;

  QModelIndex maux = m_packageModel->index(0, 0, QModelIndex());
  ui->tvPackages->setCurrentIndex(maux);
  ui->tvPackages->scrollTo(maux, QAbstractItemView::PositionAtCenter);
  ui->tvPackages->setCurrentIndex(maux);

  list->clear();

  if (UnixCommand::getLinuxDistro() != ectn_KAOS ||
      (UnixCommand::getLinuxDistro() == ectn_KAOS && !ui->actionUseInstantSearch->isChecked()))
  {
    QString search = Package::parseSearchString(m_leFilterPackage->text());
    search.remove(QLatin1Char('^'));
    search.remove(QLatin1Char('$'));
    m_packageModel->applyFilter(search);

    ui->tvPackages->selectionModel()->clear();
    QModelIndex mi = m_packageModel->index(0, PackageModel::ctn_PACKAGE_NAME_COLUMN, QModelIndex());
    ui->tvPackages->setCurrentIndex(mi);
    ui->tvPackages->scrollTo(mi);
    invalidateTabs();

    int numPkgs = m_packageModel->getPackageCount();

    if (m_leFilterPackage->text() != QLatin1String("")){
      if (numPkgs > 0) m_leFilterPackage->setFoundStyle();
      else m_leFilterPackage->setNotFoundStyle();
    }
    else{
      m_leFilterPackage->initStyleSheet();
      m_packageModel->applyFilter(QLatin1String(""));
    }
  }
  else
  {
    reapplyPackageFilter();
  }

  //Refresh counters
  m_numberOfInstalledPackages = installedCount;

  //Refresh statusbar widget
  refreshStatusBar();
  tvPackagesSelectionChanged(QItemSelection(),QItemSelection());

  //Refresh application icon
  refreshAppIcon();

  counter = list->count();
  m_progressWidget->setValue(counter);
  m_progressWidget->close();

  refreshColumnSortSetup();
  refreshToolBar();
  refreshStatusBarToolButtons();
}

/*
 * Prints the AUR toolButton at the left of the statusbar.
 * It warns the user about outdated AUR packages!
 */
void MainWindow::showToolButtonAUR()
{
  if (isAURGroupSelected() && UnixCommand::getLinuxDistro() != ectn_KAOS) return;

  m_outdatedAURPackagesNameVersion = &g_fwOutdatedAURPackages.result()->content;

  if(m_outdatedAURPackagesNameVersion->count() > 0)
  {
    if (m_outdatedAURPackagesNameVersion->count() == 1)
    {
      m_toolButtonAUR->setText(QStringLiteral("(1)"));
      m_toolButtonAUR->setToolTip(StrConstants::getOneNewUpdate());
    }
    else
    {
      m_toolButtonAUR->setText(QLatin1Char('(') + QString::number(m_outdatedAURPackagesNameVersion->count()) + QLatin1Char(')'));
      m_toolButtonAUR->setToolTip(
          StrConstants::getNewUpdates(m_outdatedAURPackagesNameVersion->count()));
    }

    m_toolButtonAUR->show();

    //Let's disable menu for AUR packages updating if KaOS is running
    if (UnixCommand::getLinuxDistro() == ectn_KAOS)
      m_menuToolButtonAUR->setEnabled(false);
  }
  else
  {
    m_toolButtonAUR->setText(QLatin1String(""));
    m_toolButtonAUR->setToolTip(QLatin1String(""));
    m_toolButtonAUR->hide();
  }

  ui->statusBar->addWidget(m_toolButtonAUR);
}

/*
 * Refreshes toolbar in order to insert/remove AUR tool button
 */
void MainWindow::refreshToolBar()
{
  m_hasForeignTool =
      UnixCommand::hasTheExecutable(Package::getForeignRepositoryToolName());

  //If there's no AUR helper installed let's check if we are running an ARCH based OS
  if (!m_hasForeignTool)
  {
    LinuxDistro ld = UnixCommand::getLinuxDistro();
    if (ld != ectn_KAOS &&
        ld != ectn_CHAKRA &&
        ld != ectn_PARABOLA)
    {
      //Here the user lost the AUR tool he was using
      m_hasForeignTool = true;
      m_actionSwitchToForeignTool->setToolTip(QStringLiteral("AUR"));
      SettingsManager::setAURTool(ctn_NO_AUR_TOOL);
    }
  }

  if (m_hasForeignTool)
  {
    if (!ui->mainToolBar->actions().contains(m_actionSwitchToForeignTool))
    {
      ui->mainToolBar->insertAction(m_dummyAction, m_actionSwitchToForeignTool);
      m_separatorForActionForeign = ui->mainToolBar->insertSeparator(m_actionSwitchToForeignTool);
    }
  }
  else
  {
    if (ui->mainToolBar->actions().contains(m_actionSwitchToForeignTool))
    {
      bool wasChecked = (m_actionSwitchToForeignTool->isChecked());

      ui->mainToolBar->removeAction(m_actionSwitchToForeignTool);
      ui->mainToolBar->removeAction(m_separatorForActionForeign);

      if (wasChecked)
      {
        m_actionSwitchToForeignTool->setChecked(false);
        ui->twGroups->setEnabled(true);
        groupItemSelected();
      }
    }
  }
}

/*
 * Refreshes the toolButtons which indicate outdated packages
 */
void MainWindow::refreshStatusBarToolButtons()
{
  if (isAURGroupSelected() && UnixCommand::getLinuxDistro() != ectn_KAOS) return;

  if (m_hasForeignTool && SettingsManager::getSearchOutdatedAURPackages())
  {
    QFuture<FTOutdatedPackages *> f;
    f = QtConcurrent::run(getOutdatedForeignToolPackages);
    g_fwOutdatedAURPackages.setFuture(f);
    connect(&g_fwOutdatedAURPackages, SIGNAL(finished()), this, SLOT(showToolButtonAUR()));
  }

  if (m_commandExecuting == ectn_NONE && !isSearchByFileSelected() && !m_actionSwitchToForeignTool->isChecked())
    ui->twGroups->setEnabled(true);
}

/*
 * Prints the values of the package counters at the left of the statusBar
 */
void MainWindow::refreshStatusBar()
{
  QString text;
  ui->statusBar->removeWidget(m_toolButtonPacman);
  ui->statusBar->removeWidget(m_toolButtonAUR);

  int numberOfInstalledPackages = m_packageModel->getInstalledPackagesCount();

  if(numberOfInstalledPackages > 0)
  {
    text = StrConstants::getNumberInstalledPackages(numberOfInstalledPackages);
  }
  else if (m_packageModel->getPackageCount() > 0 && m_leFilterPackage->text().isEmpty() && !m_packageModel->isFiltered())
  {
    text = StrConstants::getNumberInstalledPackages(m_numberOfInstalledPackages);
  }
  else if (m_packageModel->getPackageCount() == 0)
  {
    m_lblSelCounter->setText(QLatin1String(""));
    m_lblTotalCounters->setText(QLatin1String(""));
  }

  m_lblTotalCounters->setText(text);
  ui->statusBar->addWidget(m_lblTotalCounters);

  if((m_numberOfOutdatedPackages > 0) && (!isAURGroupSelected()))
  {
    m_toolButtonPacman->show();

    if (m_numberOfOutdatedPackages == 1)
    {
      m_toolButtonPacman->setText(QStringLiteral("(1)"));
      m_toolButtonPacman->setToolTip(StrConstants::getOneNewUpdate());
    }
    else
    {
      m_toolButtonPacman->setText(QLatin1Char('(') + QString::number(m_numberOfOutdatedPackages) + QLatin1Char(')'));
      m_toolButtonPacman->setToolTip(StrConstants::getNewUpdates(m_numberOfOutdatedPackages));
    }

    ui->statusBar->addWidget(m_toolButtonPacman);
  }
  else
  {
    m_toolButtonPacman->hide();
    m_toolButtonPacman->setText(QLatin1String(""));
    m_toolButtonPacman->setToolTip(QLatin1String(""));
  }
  if (m_outdatedAURStringList->count() > 0 && !isAURGroupSelected())
  {
    m_toolButtonAUR->show();
    ui->statusBar->addWidget(m_toolButtonAUR);
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
  if (package == nullptr)
    return;

  CPUIntensiveComputing cic;
  QTextBrowser *text = ui->twProperties->widget(
        ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));

  if (text)
  {
    text->clear();
    text->setHtml(OctopiTabInfo::formatTabInfo(*package, *m_outdatedAURPackagesNameVersion));
    text->scrollToAnchor(OctopiTabInfo::anchorBegin);
    //We have to clear the cached Info contents...
    m_cachedPackageInInfo = QLatin1String("");
  }
}

/*
 * Re-populates the HTML view with selected package's information (tab ONE)
 */
void MainWindow::refreshTabInfo(bool clearContents, bool neverQuit)
{
  if(!neverQuit &&
     (ui->twProperties->currentIndex() != ctn_TABINDEX_INFORMATION || !isPropertiesTabWidgetVisible() || !m_initializationCompleted)) return;

  QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (clearContents || selectionModel == nullptr ||
      selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTextBrowser *text =
        ui->twProperties->widget(ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));

    if (text)
    {
      text->clear();
    }

    m_cachedPackageInInfo = QLatin1String("");
    return;
  }

  QModelIndex item = selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).first();
  const PackageRepository::PackageData*const package = m_packageModel->getData(item);
  if (package == nullptr) {
    assert(false);
    return;
  }

  //If we are trying to refresh an already displayed package...
  if (m_cachedPackageInInfo == package->repository+QLatin1Char('#')+package->name+QLatin1Char('#')+package->version)
  {
    if (neverQuit)
    {
      changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
    }

    return;
  }

  CPUIntensiveComputing cic;

  /* Appends all info from the selected package! */
  QString pkgName=package->name;

  if (isAURGroupSelected() && !package->installed())
  {
    QString aux_desc = package->description;
    int space = aux_desc.indexOf(QLatin1Char(' '));
    QString pkgDescription = aux_desc.mid(space+1);
    QString version = StrConstants::getVersion();
    QTextBrowser*const text = ui->twProperties->widget(
          ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));

    if (text)
    {
      PackageInfoData kcp;

      if (Package::getForeignRepositoryToolName() == ctn_KCP_TOOL)
      {
        QEventLoop el;
        QFuture<PackageInfoData> f;
        f = QtConcurrent::run(getKCPInformation, pkgName);
        connect(&g_fwKCPInformation, SIGNAL(finished()), &el, SLOT(quit()));
        g_fwKCPInformation.setFuture(f);
        el.exec();

        kcp = f.result();
      }

      QString html;
      text->clear();
      QString anchorBegin = QStringLiteral("anchorBegin");

      html += QLatin1String("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
      html += QLatin1String("<a id=\"") + anchorBegin + QLatin1String("\"></a>");

      if (UnixCommand::getLinuxDistro() != ectn_KAOS && UnixCommand::getLinuxDistro() != ectn_CHAKRA)
        html += QLatin1String("<h2><a href=\"https://aur.archlinux.org/packages/") + pkgName + QLatin1String("\">") + pkgName + QLatin1String("</a></h2>");
      else
        html += QLatin1String("<h2>") + pkgName + QLatin1String("</h2>");

      if (Package::getForeignRepositoryToolName() != ctn_KCP_TOOL)
      {
        html += QLatin1String("<a style=\"font-size:16px;\">") + pkgDescription + QLatin1String("</a>");
        html += QLatin1String("<table border=\"0\">");
        html += QLatin1String("<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>");
        html += QLatin1String("<tr><td>") + version + QLatin1String("</td><td>") + package->version + QLatin1String("</td></tr>");

        QString aurPkgInfo = QString::fromUtf8(UnixCommand::getAURInformation(pkgName));
        QString url = Package::getURL(aurPkgInfo);
        if (!url.isEmpty() && !url.contains(QLatin1String("(null)")))
        {
          html += QLatin1String("<tr><td>") + StrConstants::getURL() + QLatin1String("</td><td>") + url + QLatin1String("</td></tr>");
        }

        if (Package::getForeignRepositoryToolName() != ctn_CHASER_TOOL)
        {
          QString licenses=Package::getLicense(aurPkgInfo);
          if (!licenses.isEmpty() && !licenses.contains(QLatin1String("(null)")))
          {
            html += QLatin1String("<tr><td>") + StrConstants::getLicenses() + QLatin1String("</td><td>") + licenses + QLatin1String("</td></tr>");
          }

          QString maintainer=Package::getMaintainer(aurPkgInfo);
          if (!maintainer.isEmpty())
          {
            html += QLatin1String("<tr><td>") + StrConstants::getMaintainer() + QLatin1String("</td><td>") + maintainer + QLatin1String("</td></tr>");
          }

          QString dependsOn=Package::getDependsOn(aurPkgInfo);
          if (!dependsOn.isEmpty())
          {
            html += QLatin1String("<tr><td>") + StrConstants::getDependsOn() + QLatin1String("</td><td>") + dependsOn + QLatin1String("</td></tr>");
          }

          QString makeDeps=Package::getMakeDeps(aurPkgInfo);
          if (!makeDeps.isEmpty())
          {
            html += QLatin1String("<tr><td>") + StrConstants::getMakeDeps() + QLatin1String("</td><td>") + makeDeps + QLatin1String("</td></tr>");
          }

          QString lastModified=Package::getLastModified(aurPkgInfo);
          if (!lastModified.isEmpty())
          {
            html += QLatin1String("<tr><td>") + StrConstants::getLastModified() + QLatin1String("</td><td>") + lastModified + QLatin1String("</td></tr>");
          }

          QString outOfDate=Package::getOutOfDate(aurPkgInfo);
          if (!outOfDate.isEmpty())
          {
            html += QLatin1String("<tr><td>") + StrConstants::getOutOfDate() + QLatin1String("</td><td>") + outOfDate + QLatin1String("</td></tr>");
          }
        }
      }
      else if (Package::getForeignRepositoryToolName() == ctn_KCP_TOOL)
      {
        html += QLatin1String("<a style=\"font-size:16px;\">") + kcp.description + QLatin1String("</a>");
        html += QLatin1String("<table border=\"0\">");
        html += QLatin1String("<tr><th width=\"20%\"></th><th width=\"80%\"></th></tr>");
        html += QLatin1String("<tr><td>") + version + QLatin1String("</td><td>") + package->version + QLatin1String("</td></tr>");

        if (kcp.url != QLatin1String("--"))
          html += QLatin1String("<tr><td>") + StrConstants::getURL() + QLatin1String("</td><td>") + kcp.url + QLatin1String("</td></tr>");

        if (kcp.license != QLatin1String("--"))
          html += QLatin1String("<tr><td>") + StrConstants::getLicenses() + QLatin1String("</td><td>") + kcp.license + QLatin1String("</td></tr>");

        if (kcp.provides != QLatin1String("--"))
          html += QLatin1String("<tr><td>") + StrConstants::getProvides() + QLatin1String("</td><td>") + kcp.provides + QLatin1String("</td></tr>");

        if (kcp.dependsOn != QLatin1String("--"))
          html += QLatin1String("<tr><td>") + StrConstants::getDependsOn() + QLatin1String("</td><td>") +
              Package::makeAnchorOfPackage(kcp.dependsOn) + QLatin1String("</td></tr>");
      }

      html += QLatin1String("</table>");
      text->setHtml(html);
      text->scrollToAnchor(anchorBegin);
    }
  }
  else //We are not in the AUR group
  {
    QTextBrowser *text = ui->twProperties->widget(
          ctn_TABINDEX_INFORMATION)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));

    if (text)
    {
      text->clear();
      text->setHtml(OctopiTabInfo::formatTabInfo(*package, *m_outdatedAURPackagesNameVersion));
      text->scrollToAnchor(OctopiTabInfo::anchorBegin);
    }
  }

  m_cachedPackageInInfo = package->repository+QLatin1Char('#')+package->name+QLatin1String("#")+package->version;

  if (neverQuit)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_INFORMATION);
  }
}

/*
 * Re-populates the treeview which contains the file list of selected package (tab TWO)
 */
void MainWindow::refreshTabFiles(bool clearContents, bool neverQuit)
{
  if (m_progressWidget->isVisible() || !m_initializationCompleted) return;

  if(!neverQuit &&
     (ui->twProperties->currentIndex() != ctn_TABINDEX_FILES || !isPropertiesTabWidgetVisible()))
  {
    return;
  }

  bool filterHasFocus = m_leFilterPackage->hasFocus();
  bool tvPackagesHasFocus = ui->tvPackages->hasFocus();

  QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();

  if (clearContents || selectionModel == nullptr ||
      selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).count() == 0)
  {
    QTreeView*const tvPkgFileList =
        ui->twProperties->getTvPkgFileList();

    if(tvPkgFileList)
    {
      QStandardItemModel*const modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
      modelPkgFileList->clear();
      m_cachedPackageInFiles = QLatin1String("");
      closeTabFilesSearchBar();
      if (filterHasFocus) m_leFilterPackage->setFocus();
      else if (tvPackagesHasFocus) ui->tvPackages->setFocus();

      return;
    }
  }

  QModelIndex pkg = selectionModel->selectedRows(PackageModel::ctn_PACKAGE_NAME_COLUMN).first();
  const PackageRepository::PackageData*const package = m_packageModel->getData(pkg);

  if (package == nullptr) {
    assert(false);
    return;
  }

  //If we are trying to refresh an already displayed package...
  if (m_cachedPackageInFiles == package->repository+QLatin1Char('#')+package->name+QLatin1Char('#')+package->version)
  {
    if (neverQuit)
    {
      changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
      selectFirstItemOfPkgFileList();
    }
    else
    {
      QTreeView*const tv = ui->twProperties->getTvPkgFileList();
      if (tv)
        tv->scrollTo(tv->currentIndex());
    }

    return;
  }

  //Maybe this is a non-installed package...
  bool nonInstalled = (!package->installed());

  QTreeView*const tvPkgFileList =
      ui->twProperties->getTvPkgFileList();

  if (tvPkgFileList)
  {
    QString pkgName = package->name;
    QStandardItemModel *fakeModelPkgFileList = new QStandardItemModel(this);
    QStandardItemModel *modelPkgFileList = qobject_cast<QStandardItemModel*>(tvPkgFileList->model());
    modelPkgFileList->clear();

    QStandardItem *fakeRoot = fakeModelPkgFileList->invisibleRootItem();
    QStandardItem *root = modelPkgFileList->invisibleRootItem();
    QStandardItem *lastDir, *item, *lastItem=root, *parent;
    bool first=true;
    lastDir = root;

    QEventLoop el;
    QFuture<QStringList> f;
    QFutureWatcher<QStringList> fwPackageContents;
    f = QtConcurrent::run(Package::getContents, pkgName, !nonInstalled);
    connect(&fwPackageContents, SIGNAL(finished()), &el, SLOT(quit()));
    fwPackageContents.setFuture(f);

    //Let's wait before we get the pkg file list from the other thread...
    el.exec();
    const QStringList fileList = fwPackageContents.result();

    QString fullPath;
    int counter = 0;
    m_progressWidget->setRange(0, fileList.count());
    m_progressWidget->setValue(0);
    m_progressWidget->show();

    for (const QString& file: fileList)
    {
      bool isDir = file.endsWith(QLatin1Char('/'));
      bool isSymLinkToDir = false;
      QString baseFileName = extractBaseFileName(file);

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

      if(isDir)
      {
        if (first)
        {
          item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
          item->setAccessibleDescription(QLatin1String("directory ") + item->text());
          fakeRoot->appendRow ( item );
        }
        else
        {
          fullPath = utils::showFullPathOfItem(lastDir->index());
          //std::cout << "Testing if " << file.toLatin1().data() << " contains " << fullPath.toLatin1().data() << std::endl;
          if ( file.contains ( fullPath )) {
            //std::cout << "It contains !!! So " << fullPath.toLatin1().data() << " is its parent." << std::endl;
            item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
            item->setAccessibleDescription(QLatin1String("directory ") + item->text());
            lastDir->appendRow ( item );
          }
          else
          {
            //std::cout << "It doens't contain..." << std::endl;
            parent = lastItem->parent();
            if (parent != nullptr) fullPath = utils::showFullPathOfItem(parent->index());

            do
            {
              //if (parent != 0) std::cout << "Testing if " << file.toLatin1().data() << " contains " << fullPath.toLatin1().data() << std::endl;
              if ( parent == nullptr || file.contains ( fullPath )) break;
              parent = parent->parent();
              if (parent != nullptr) fullPath = utils::showFullPathOfItem(parent->index());
            }
            while (parent != fakeRoot);

            item = new QStandardItem ( IconHelper::getIconFolder(), baseFileName );
            item->setAccessibleDescription(QLatin1String("directory ") + item->text());

            if ( parent != nullptr )
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
        item->setAccessibleDescription(QLatin1String("directory ") + item->text());
        parent = lastDir;
        if (parent != nullptr) fullPath = utils::showFullPathOfItem(parent->index());

        do
        {
          if ( parent == nullptr || file.contains ( fullPath )) break;
          parent = parent->parent();
          if (parent != nullptr) fullPath = utils::showFullPathOfItem(parent->index());
        }
        while ( parent != fakeRoot );

        if (parent != nullptr)
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
        item->setAccessibleDescription(QLatin1String("file ") + item->text());
        parent = lastDir;
        if (parent != nullptr) fullPath = utils::showFullPathOfItem(parent->index());

        do
        {
          if ( parent == nullptr || file.contains ( fullPath )) break;
          parent = parent->parent();
          if (parent != nullptr) fullPath = utils::showFullPathOfItem(parent->index());
        }
        while ( parent != fakeRoot );

        parent->appendRow ( item );
      }

      counter++;
      m_progressWidget->setValue(counter);
      lastItem = item;
      first = false;
    }

    m_progressWidget->close();
    root = fakeRoot;
    fakeModelPkgFileList->sort(0);
    modelPkgFileList = fakeModelPkgFileList;
    tvPkgFileList->setModel(modelPkgFileList);
    tvPkgFileList->header()->setDefaultAlignment( Qt::AlignCenter );
    modelPkgFileList->setHorizontalHeaderLabels( QStringList() << StrConstants::getContentsOf().arg(pkgName));
  }

  m_cachedPackageInFiles = package->repository+QLatin1Char('#')+package->name+QLatin1Char('#')+package->version;

  if (neverQuit)
  {
    changeTabWidgetPropertiesIndex(ctn_TABINDEX_FILES);
    selectFirstItemOfPkgFileList();
  }

  closeTabFilesSearchBar();

  if (filterHasFocus) m_leFilterPackage->setFocus();
  else if (tvPackagesHasFocus) ui->tvPackages->setFocus();
}

/*
 * This SLOT is called every time we press a key at FilterLineEdit
 */
void MainWindow::reapplyPackageFilter()
{
  clearTabsInfoOrFiles();

  //We are not in a search by filenames...
  if (!isSearchByFileSelected())
  {
    bool isFilterPackageSelected = m_leFilterPackage->hasFocus();
    int numPkgs = m_packageModel->getPackageCount();

    if (m_leFilterPackage->text() != QLatin1String(""))
    {
      QString search = m_leFilterPackage->text();
      search = search.replace(QLatin1String("+"), QLatin1String("\\+"));
      m_packageModel->applyFilter(search);
      numPkgs = m_packageModel->getPackageCount();

      if (numPkgs > 0) m_leFilterPackage->setFoundStyle();
      else m_leFilterPackage->setNotFoundStyle();
    }
    else
    {
      m_leFilterPackage->initStyleSheet();
      m_packageModel->applyFilter(QLatin1String(""));
    }

    if (isFilterPackageSelected || numPkgs == 0)
    {
      m_leFilterPackage->setFocus();
    }

    if (numPkgs == 0)
      tvPackagesSelectionChanged(QItemSelection(),QItemSelection());

    ui->tvPackages->selectionModel()->clear();
    QModelIndex mi = m_packageModel->index(0, PackageModel::ctn_PACKAGE_NAME_COLUMN, QModelIndex());
    ui->tvPackages->setCurrentIndex(mi);
    ui->tvPackages->scrollTo(mi);
  }
  //If we are using "Search By file...
  else
  {
    m_leFilterPackage->initStyleSheet();

    //We need to provide QCompleter data to the SearchLineEdit...
    if (!m_leFilterPackage->text().isEmpty())
      m_leFilterPackage->refreshCompleterData();
  }
}

/*
 * This SLOT is called every time we press a key at FilterLineEdit, but ONLY when INSTANT SEARCH is disabled
 */
void MainWindow::lightPackageFilter()
{
  if (isAURGroupSelected())
  {
    if (m_leFilterPackage->text() == QLatin1String(""))
    {
      if (UnixCommand::getLinuxDistro() != ectn_KAOS)
      {
        m_packageModel->applyFilter(QStringLiteral("ççç"));
        m_leFilterPackage->initStyleSheet();
        invalidateTabs();
        refreshStatusBar();
      }
      else
      {
        m_packageModel->applyFilter(QLatin1String(""));
        reapplyPackageFilter();
        refreshStatusBar();
      }
    }
  }
  else if (!isAURGroupSelected())
  {
    if (m_leFilterPackage->text() == QLatin1String(""))
    {
      CPUIntensiveComputing cic;
      m_packageModel->applyFilter(QLatin1String(""));
      reapplyPackageFilter();
      refreshStatusBar();
    }
  }
}

/*
 * Whenever user selects View/All we show him all the available packages
 */
void MainWindow::selectedAllPackagesMenu()
{  
  m_selectedViewOption = ectn_ALL_PKGS;
  changePackageListModel(ectn_ALL_PKGS, m_selectedRepository);
}

/*
 * Whenever user selects View/Installed we show him only the installed packages
 */
void MainWindow::selectedInstalledPackagesMenu()
{
  m_selectedViewOption = ectn_INSTALLED_PKGS;
  changePackageListModel(ectn_INSTALLED_PKGS, m_selectedRepository);
}

/*
 * Whenever user selects View/Non Installed we show him only the non installed packages
 */
void MainWindow::selectedNonInstalledPackagesMenu()
{
  m_selectedViewOption = ectn_NON_INSTALLED_PKGS;
  changePackageListModel(ectn_NON_INSTALLED_PKGS, m_selectedRepository);
}

/*
 * Whenever user selects View/Outdated we show him only the non installed packages
 */
void MainWindow::selectedOutdatedPackagesMenu()
{
  m_selectedViewOption = ectn_OUTDATED_PKGS;
  changePackageListModel(ectn_OUTDATED_PKGS, m_selectedRepository);
}

/*
 * Whenever user selects any of the available repositories in View/Repository submenu,
 * we filter the packages from that chosen repo
 */
void MainWindow::selectedRepositoryMenu(QAction *actionRepoSelected)
{
  if (actionRepoSelected->text() == StrConstants::getAll())
    m_selectedRepository = QLatin1String("");
  else
    m_selectedRepository = actionRepoSelected->text();

  changePackageListModel(m_selectedViewOption, m_selectedRepository);
}
