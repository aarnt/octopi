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
 * This is MainWindow's Transaction related code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "uihelper.h"
#include "wmhelper.h"
#include "strconstants.h"
#include "transactiondialog.h"
#include "multiselectiondialog.h"
#include "searchlineedit.h"
#include "globals.h"
#include <iostream>
#include <cassert>
#include "termwidget.h"
#include "aurvote.h"

#include <QComboBox>
#include <QProgressBar>
#include <QMessageBox>
#include <QStandardItem>
#include <QTextBrowser>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>

/*
 * Watches the state of tvTransaction treeview to see if Commit/Cancel actions must be activated/deactivated
 */
void MainWindow::changeTransactionActionsState()
{
  bool state = areTherePendingActions();
  ui->actionApply->setEnabled(state);
  ui->actionCancel->setEnabled(state);
  ui->actionCheckUpdates->setEnabled(!state);

  if(m_hasMirrorCheck) m_actionMenuMirrorCheck->setEnabled(!state);
  if(m_hasAURTool) m_actionSwitchToAURTool->setEnabled(!state);

  if (state == false && m_outdatedStringList->count() > 0)
    ui->actionSystemUpgrade->setEnabled(true);
  else if (state == true)
    ui->actionSystemUpgrade->setEnabled(false);
}

/*
 * Removes all packages from the current transaction
 */
void MainWindow::clearTransactionTreeView()
{
  removePackagesFromRemoveTransaction();
  removePackagesFromInstallTransaction();
}

/*
 * Looks if the Transaction's Remove or Install parent items have any package inside them
 */
bool MainWindow::areTherePendingActions()
{
  return (getRemoveTransactionParentItem()->hasChildren() ||
          getInstallTransactionParentItem()->hasChildren());
}

/*
 * Retrieves the Remove parent item of the Transaction treeview
 */
QStandardItem * MainWindow::getRemoveTransactionParentItem()
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_ACTIONS)->findChild<QTreeView*>("tvTransaction");
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvTransaction->model());
  QStandardItem *si = 0;

  if(sim)
  {
    si = sim->item(0, 0);
  }

  return si;
}

/*
 * Retrieves the Install parent item of the Transaction treeview
 */
QStandardItem * MainWindow::getInstallTransactionParentItem()
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_ACTIONS)->findChild<QTreeView*>("tvTransaction");
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(tvTransaction->model());
  QStandardItem *si = 0;

  if(sim)
  {
    si = sim->item(1, 0);
  }

  return si;
}

/*
 * Inserts the given package into the Remove parent item of the Transaction treeview
 */
void MainWindow::insertRemovePackageIntoTransaction(const QString &pkgName)
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_ACTIONS)->findChild<QTreeView*>("tvTransaction");
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItem * siPackageToRemove = new QStandardItem(IconHelper::getIconRemoveItem(), pkgName);
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siRemoveParent->model());
  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  int slash = pkgName.indexOf("/");
  QString pkg = pkgName.mid(slash+1);
  siPackageToRemove->setText(pkg);

  if (foundItems.size() == 0)
  {
    slash = pkgName.indexOf("/");
    pkg = pkgName.mid(slash+1);
    QList<QStandardItem *> aux = sim->findItems(pkg, Qt::MatchRecursive | Qt::MatchExactly);

    if (aux.count() == 0) siRemoveParent->appendRow(siPackageToRemove);
  }
  else if (foundItems.size() == 1 && foundItems.at(0)->parent() == siInstallParent)
  {
    siInstallParent->removeRow(foundItems.at(0)->row());
    siRemoveParent->appendRow(siPackageToRemove);
  }

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_ACTIONS);
  tvTransaction->expandAll();
  changeTransactionActionsState();
}

/*
 * Inserts the given package into the Install parent item of the Transaction treeview
 */
void MainWindow::insertInstallPackageIntoTransaction(const QString &pkgName)
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_ACTIONS)->findChild<QTreeView*>("tvTransaction");
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItem * siPackageToInstall = new QStandardItem(IconHelper::getIconInstallItem(), pkgName);
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siInstallParent->model());
  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  if (foundItems.size() == 0)
  {
    int slash = pkgName.indexOf("/");
    QString pkg = pkgName.mid(slash+1);
    QList<QStandardItem *> aux = sim->findItems(pkg, Qt::MatchRecursive | Qt::MatchExactly);

    if (aux.count() > 0) siRemoveParent->removeRow(aux.at(0)->row());
    siInstallParent->appendRow(siPackageToInstall);
  }
  else if (foundItems.size() == 1 && foundItems.at(0)->parent() == siRemoveParent)
  {
    siRemoveParent->removeRow(foundItems.at(0)->row());
    siInstallParent->appendRow(siPackageToInstall);
  }

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_ACTIONS);
  tvTransaction->expandAll();
  changeTransactionActionsState();
}

/*
 * Removes all packages from the Remove parent item of the Transaction treeview
 */
void MainWindow::removePackagesFromRemoveTransaction()
{
  QStandardItem * siRemove = getRemoveTransactionParentItem();
  siRemove->removeRows(0, siRemove->rowCount());
  changeTransactionActionsState();
}

/*
 * Removes all packages from the Install parent item of the Transaction treeview
 */
void MainWindow::removePackagesFromInstallTransaction()
{
  QStandardItem * siInstall = getInstallTransactionParentItem();
  siInstall->removeRows(0, siInstall->rowCount());
  changeTransactionActionsState();
}

/*
 * Retrieves the number of "to be removed" packages
 */
int MainWindow::getNumberOfTobeRemovedPackages()
{
  QStandardItem * siRemoval = getRemoveTransactionParentItem();
  return siRemoval->rowCount();
}

/*
 * Retrieves the list of all packages scheduled to be removed
 */
QString MainWindow::getTobeRemovedPackages()
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
 * Retrieves the list of all packages scheduled to be installed
 */
QString MainWindow::getTobeInstalledPackages()
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

/*
 * Inserts the current selected packages for removal into the Transaction Treeview
 * This is the SLOT, it needs to call insertRemovePackageIntoTransaction(PackageName) to work!
 */
void MainWindow::insertIntoRemovePackage(QModelIndex *indexToInclude)
{
  qApp->processEvents();
  QStringList dependencies;

  if (!isAURGroupSelected())
  {
    bool checkDependencies=false;
    ensureTabVisible(ctn_TABINDEX_ACTIONS);
    QModelIndexList selectedRows;

    if (indexToInclude == nullptr)
      selectedRows = ui->tvPackages->selectionModel()->selectedRows();
    else
    {
      selectedRows.append(*indexToInclude);
    }

    //First, let's see if we are dealing with a package group
    if(!isAllGroupsSelected())
    {
      //If we are trying to remove all the group's packages, why not remove the entire group?
      if(selectedRows.count() == m_packageModel->getPackageCount())
      {
        insertRemovePackageIntoTransaction(getSelectedGroup());
        return;
      }
    }

    QString removeCmd = m_removeCommand;
    if (removeCmd == "Rcs" )
    {
      checkDependencies = true;
    }

    foreach(QModelIndex item, selectedRows)
    {
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      if (package == nullptr) {
        assert(false);
        continue;
      }

      if(checkDependencies)
      {
        QStringList *targets = Package::getTargetRemovalList(package->name, removeCmd);

        foreach(QString target, *targets)
        {
          int separator = target.lastIndexOf("-");
          QString candidate = target.left(separator);
          separator = candidate.lastIndexOf("-");
          candidate = candidate.left(separator);

          if (candidate != package->name)
          {
            dependencies.append(candidate);
          }
        }

        if (dependencies.count() > 0)
        {
          if (!dependencies.at(0).contains("HoldPkg was found in"))
          {
            if (!insertIntoRemovePackageDeps(dependencies))
              return;
          }
        }
      }

      insertRemovePackageIntoTransaction(package->repository + "/" + package->name);
    }
  }
  else
  {
    doRemoveAURPackage();
  }
}

/*
 * Inserts the current selected group for removal into the Transaction Treeview
 */
void MainWindow::insertGroupIntoRemovePackage()
{
  ensureTabVisible(ctn_TABINDEX_ACTIONS);
  insertRemovePackageIntoTransaction(getSelectedGroup());
}

/*
 * Inserts the current selected packages for installation into the Transaction Treeview
 * This is the SLOT, it needs to call insertInstallPackageIntoTransaction(PackageName) to work!
 */
void MainWindow::insertIntoInstallPackage(QModelIndex *indexToInclude)
{
  qApp->processEvents();

  if (!isAURGroupSelected())
  {
    ensureTabVisible(ctn_TABINDEX_ACTIONS);
    QModelIndexList selectedRows;

    if (indexToInclude == nullptr)
      selectedRows = ui->tvPackages->selectionModel()->selectedRows();
    else
    {
      selectedRows.append(*indexToInclude);
    }

    //First, let's see if we are dealing with a package group
    if(!isAllGroupsSelected())
    {
      //If we are trying to insert all the group's packages, why not insert the entire group?
      if(selectedRows.count() == m_packageModel->getPackageCount())
      {
        insertInstallPackageIntoTransaction(getSelectedGroup());
        return;
      }
    }

    foreach(QModelIndex item, selectedRows)
    {
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      if (package == nullptr) {
        assert(false);
        continue;
      }

      insertIntoInstallPackageOptDeps(package->name); //Do we have any deps???
      insertInstallPackageIntoTransaction(package->repository + "/" + package->name);
    }
  }
  else
  {
    doInstallAURPackage();
  }
}

/*
 * Searches in Install Transaction queue for the given package
 */
bool MainWindow::isPackageInInstallTransaction(const QString &pkgName)
{
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siInstallParent->model());
  const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkgName);
  QString repo;

  if (package != nullptr) repo = package->repository;
  QList<QStandardItem *> foundItems = sim->findItems(repo + "/" + pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  return (foundItems.size() > 0);
}

/*
 * Searches in Remove Transaction queue for the given package
 */
bool MainWindow::isPackageInRemoveTransaction(const QString &pkgName)
{
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siRemoveParent->model());
  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  return (foundItems.size() > 0);
}

/*
 * Inserts all optional deps of the current select package into the Transaction Treeview
 */
void MainWindow::insertIntoInstallPackageOptDeps(const QString &packageName)
{
  CPUIntensiveComputing *cic = new CPUIntensiveComputing;

  //Does this package have non installed optional dependencies?
  QStringList optDeps = Package::getOptionalDeps(packageName);
  QList<const PackageRepository::PackageData*> optionalPackages;

  foreach(QString optDep, optDeps)
  {
    QString candidate = optDep;
    int points = candidate.indexOf(":");
    candidate = candidate.mid(0, points).trimmed();
    const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(candidate);

    if(!isPackageInInstallTransaction(candidate) &&
       !isPackageInstalled(candidate) && package != nullptr)
    {
      optionalPackages.append(package);
    }
  }

  if(optionalPackages.count() > 0)
  {
    MultiSelectionDialog *msd = new MultiSelectionDialog(this);
    msd->setWindowTitle(packageName + ": " + StrConstants::getOptionalDeps());
    msd->setWindowIcon(windowIcon());
    QStringList selectedPackages;

    foreach(const PackageRepository::PackageData* candidate, optionalPackages)
    {
      QString desc = candidate->description;
      int space = desc.indexOf(" ");
      desc = desc.mid(space+1);

      msd->addPackageItem(candidate->name, candidate->description, candidate->repository);
    }

    delete cic;
    if (msd->exec() == QMessageBox::Ok)
    {
      selectedPackages = msd->getSelectedPackages();

      foreach(QString pkg, selectedPackages)
      {
        insertInstallPackageIntoTransaction(pkg);
      }
    }

    delete msd;
  }
  else
  {
    delete cic;
  }
}

/*
 * Inserts all remove dependencies of the current select package into the Transaction Treeview
 * Returns TRUE if the user click OK or ENTER and number of selected packages > 0.
 * Returns FALSE otherwise.
 */
bool MainWindow::insertIntoRemovePackageDeps(const QStringList &dependencies)
{
  QList<const PackageRepository::PackageData*> newDeps;
  foreach(QString dep, dependencies)
  {
    const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(dep);
    if (package != nullptr && package->installed() && !isPackageInRemoveTransaction(dep))
    {
      newDeps.append(package);
    }
  }

  if (newDeps.count() > 0)
  {
    CPUIntensiveComputing *cic = new CPUIntensiveComputing;
    MultiSelectionDialog *msd = new MultiSelectionDialog(this);
    msd->setWindowTitle(StrConstants::getRemovePackages(newDeps.count()));
    msd->setWindowIcon(windowIcon());
    QStringList selectedPackages;

    foreach(const PackageRepository::PackageData* dep, newDeps)
    {
      QString desc = dep->description;
      int space = desc.indexOf(" ");
      desc = desc.mid(space+1);

      /*if(dep->repository == StrConstants::getForeignRepositoryName() && dep->description.isEmpty())
      {
        desc = Package::getInformationDescription(dep->name, true);
      }*/

      msd->addPackageItem(dep->name, desc, dep->repository);
    }

    msd->setAllSelected();
    delete cic;
    int res = msd->exec();

    if (res == QMessageBox::Ok)
    {
      selectedPackages = msd->getSelectedPackages();
      foreach(QString pkg, selectedPackages)
      {
        insertRemovePackageIntoTransaction(pkg);
      }
    }

    delete msd;

    return (res == QMessageBox::Ok && selectedPackages.count() >= 0);
  }
  else return true;
}

/*
 * Inserts the current selected group for removal into the Transaction Treeview
 */
void MainWindow::insertGroupIntoInstallPackage()
{
  ensureTabVisible(ctn_TABINDEX_ACTIONS);
  insertInstallPackageIntoTransaction(getSelectedGroup());
}

/*
 * Adjust the count and selection count status of the selected tvTransaction item (Remove or Insert parents)
 */
void MainWindow::tvTransactionAdjustItemText(QStandardItem *item)
{
  int countSelected=0;
  QTreeView *tvTransaction =
      ui->twProperties->currentWidget()->findChild<QTreeView*>("tvTransaction");
  if (!tvTransaction) return;

  for(int c=0; c < item->rowCount(); c++)
  {
    if(tvTransaction->selectionModel()->isSelected(item->child(c)->index()))
    {
      countSelected++;
    }
  }

  QString itemText = item->text();
  int slash = itemText.indexOf("/");
  int pos = itemText.indexOf(")");

  if (slash > 0){
    itemText.remove(slash, pos-slash);
  }

  pos = itemText.indexOf(")");
  itemText.insert(pos, "/" + QString::number(countSelected));
  item->setText(itemText);
}

/*
 * SLOT called each time the selection of items in tvTransaction is changed
 */
void MainWindow::tvTransactionSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  tvTransactionAdjustItemText(getRemoveTransactionParentItem());
  tvTransactionAdjustItemText(getInstallTransactionParentItem());
}

/*
 * Method called every time some item is inserted or removed in tvTransaction treeview
 */
void MainWindow::tvTransactionRowsChanged(const QModelIndex& parent)
{
  QStandardItem *item = m_modelTransaction->itemFromIndex(parent);
  QString count = QString::number(item->rowCount());
  QStandardItem * itemRemove = getRemoveTransactionParentItem();
  QStandardItem * itemInstall = getInstallTransactionParentItem();

  if (item == itemRemove)
  {
    if (item->rowCount() > 0)
    {
      itemRemove->setText(StrConstants::getTransactionRemoveText() + " (" + count + ")");
      tvTransactionAdjustItemText(itemRemove);
    }
    else itemRemove->setText(StrConstants::getTransactionRemoveText());
  }
  else if (item == itemInstall)
  {
    if (item->rowCount() > 0)
    {
      itemInstall->setText(StrConstants::getTransactionInstallText() + " (" + count + ")");
      tvTransactionAdjustItemText(itemInstall);
    }
    else itemInstall->setText(StrConstants::getTransactionInstallText());
  }
}

/*
 * SLOT called each time some item is inserted into tvTransaction
 */
void MainWindow::tvTransactionRowsInserted(const QModelIndex& parent, int, int)
{
  tvTransactionRowsChanged(parent);
}

/*
 * SLOT called each time some item is removed from tvTransaction
 */
void MainWindow::tvTransactionRowsRemoved(const QModelIndex& parent, int, int)
{
  tvTransactionRowsChanged(parent);
}

/*
 * Whenever the user presses DEL over the main Packages TreeView we remove the selected pkg (if installed).
 *
 * Whenever the user presses DEL over the Transaction TreeView, we:
 * - Delete the package if it's bellow of "To be removed" or "To be installed" parent;
 * - Delete all the parent's packages if the user clicked in "To be removed" or "To be installed" items.
 */
void MainWindow::onPressDelete()
{
  if (ui->tvPackages->hasFocus())
  {
    execKeyActionOnPackage(ectn_REMOVE);
  }
  else
  {
    QTreeView *tvTransaction =
        ui->twProperties->widget(ctn_TABINDEX_ACTIONS)->findChild<QTreeView*>("tvTransaction");

    if (tvTransaction->hasFocus())
    {
      if(tvTransaction->currentIndex() == getRemoveTransactionParentItem()->index()){
        removePackagesFromRemoveTransaction();
      }
      else if(tvTransaction->currentIndex() == getInstallTransactionParentItem()->index()){
        removePackagesFromInstallTransaction();
      }
      else
      {
        for(int c=tvTransaction->selectionModel()->selectedIndexes().count()-1; c>=0; c--)
        {
          const QModelIndex mi = tvTransaction->selectionModel()->selectedIndexes().at(c);
          if (m_modelTransaction->itemFromIndex(mi)->parent() != 0)
          {
            m_modelTransaction->removeRow(mi.row(), mi.parent());
          }
        }
      }

      changeTransactionActionsState();
    }
  }
}

/*
 * Checks if Internet connection is up/down
 */
bool MainWindow::isInternetAvailable()
{
  bool res=true;

  //Test if Internet access exists
  if (SettingsManager::getEnableInternetChecking() && !UnixCommand::hasInternetConnection())
  {
    QMessageBox::critical(this, StrConstants::getError(), StrConstants::getInternetUnavailableError());
    res=false;
  }

  return res;
}

/*
 * Checks if we have "octopi-sudo" utility available...
 * Returns false if not!
 */
bool MainWindow::isSUAvailable()
{
  if (UnixCommand::hasTheExecutable(ctn_OCTOPISUDO))
    return true;
  else
  {
    QMessageBox::critical( nullptr, StrConstants::getApplicationName(),
                           StrConstants::getErrorNoSuCommand() +
                           "\n" + StrConstants::getYoullNeedSuFrontend());
    return false;
  }
}

/*
 * Get list of outdated packages using a temporary db
 */
void MainWindow::doCheckUpdates()
{
  if (m_commandExecuting != ectn_NONE) return;

  if (!isInternetAvailable()) return;

  m_toolButtonPacman->hide();
  m_toolButtonAUR->hide();

  //Let's synchronize kcp database too...
  if (UnixCommand::getLinuxDistro() == ectn_KAOS && UnixCommand::hasTheExecutable(ctn_KCP_TOOL))
    UnixCommand::execCommandAsNormalUser("kcp -u");

  m_commandExecuting = ectn_CHECK_UPDATES;
  disableTransactionActions();

  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));

  m_pacmanExec->doCheckUpdates();
}

/*
 * This is KaOS specific code which uses mirror-check tool.
 */
void MainWindow::doMirrorCheck()
{
  if (m_commandExecuting != ectn_NONE) return;

  m_commandExecuting = ectn_MIRROR_CHECK;
  disableTransactionActions();

  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));

  m_pacmanExec->doMirrorCheck();
}

/*
 * Does a repository sync with "pacman -Sy" !
 */
/*void MainWindow::doSyncDatabase()
{
  if (!isSUAvailable()) return;

  if (!isInternetAvailable()) return;

  //Let's synchronize kcp database too...
  if (UnixCommand::getLinuxDistro() == ectn_KAOS && UnixCommand::hasTheExecutable(ctn_KCP_TOOL) && !UnixCommand::isRootRunning())
    UnixCommand::execCommandAsNormalUser("kcp -u");

  m_commandExecuting = ectn_SYNC_DATABASE;
  disableTransactionActions();

  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));

  m_pacmanExec->doSyncDatabase();
}*/

/*
 * Updates the outdated AUR packages with "yaourt -S <list>"
 */
void MainWindow::doAURUpgrade()
{
  QString listOfTargets;
  QString auxPkg;

  foreach(QString pkg, *m_outdatedAURStringList)
  {
    auxPkg = pkg;
    auxPkg.remove("[1;39m");
    auxPkg.remove("[0m");
    auxPkg.remove("");
    listOfTargets += auxPkg + " ";
  }

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_pacmanExec->doAURUpgrade(listOfTargets);
}

/*
 * doSystemUpgrade shared code ...
 */
bool MainWindow::prepareSystemUpgrade()
{
  m_systemUpgradeDialog = false;
  bool res = isSUAvailable();
  if (!res) return false;

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  disableTransactionActions();
  return true;
}

/*
 * Does a system upgrade with "pacman -Su" !
 */
void MainWindow::doSystemUpgrade(SystemUpgradeOptions systemUpgradeOptions)
{
  Q_UNUSED(systemUpgradeOptions)

  if (isAURGroupSelected() || m_systemUpgradeDialog) return;

  if(m_callSystemUpgrade && m_numberOfOutdatedPackages == 0)
  {
    m_callSystemUpgrade = false;
    return;
  }
  else if (m_callSystemUpgradeNoConfirm && m_numberOfOutdatedPackages == 0)
  {
    m_callSystemUpgrade = false;
    return;
  }

  if (!isSUAvailable()) return;

  if(!isInternetAvailable()) return;

  if(!SettingsManager::getEnableConfirmationDialogInSysUpgrade())
  {
    int res = prepareSystemUpgrade();
    if (!res)
    {
      m_systemUpgradeDialog = false;
      enableTransactionActions();
      return;
    }

    if( (m_checkupdatesStringList->count() != 0 && m_checkupdatesStringList->contains("pacman")) ||
        (m_outdatedStringList->count() != 0 && m_outdatedStringList->contains("pacman")) )
    {
      m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
      //m_pacmanExec->setSharedMemory(m_sharedMem);
      m_pacmanExec->doSystemUpgradeInTerminal();
      m_commandQueued = ectn_NONE;
    }
    else
    {
      m_commandExecuting = ectn_SYSTEM_UPGRADE;
      //m_pacmanExec->setSharedMemory(m_sharedMem);
      m_pacmanExec->doSystemUpgrade();
      m_commandQueued = ectn_NONE;
    }
  }
  else
  {
    //Shows a dialog indicating the targets needed to be retrieved and asks for the user's permission.
    QList<PackageListData> * targets = new QList<PackageListData>();
    if (m_checkupdatesStringList->count() == 0 || (m_checkupdatesStringList->count() == m_outdatedStringList->count()))
      targets = Package::getTargetUpgradeList();

    //There are no new updates to install!
    if (targets->count() == 0 && m_outdatedStringList->count() == 0)
    {
      clearTabOutput();
      writeToTabOutput("<b>" + StrConstants::getNoNewUpdatesAvailable() + "</b><br>");
      return;
    }
    else if (targets->count() < m_outdatedStringList->count())
    {
      //This is a bug and should be shown to the user!
      //This is bug, let us find if "breaks dependency" string is here:
      if (UnixCommand::getTargetUpgradeList().contains("breaks dependency") ||
          UnixCommand::getTargetUpgradeList().contains(": requires "))
      {
        QString msg = StrConstants::getThereHasBeenATransactionError() + "\n" +
            StrConstants::getConfirmExecuteTransactionInTerminal();

        int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                        msg,
                                        QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

        if (res == QMessageBox::Yes)
        {
          res = prepareSystemUpgrade();
          if (!res)
          {
            m_systemUpgradeDialog = false;
            enableTransactionActions();
            return;
          }

          m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
          //m_pacmanExec->setSharedMemory(m_sharedMem);
          m_pacmanExec->doSystemUpgradeInTerminal(ectn_SYNC_DATABASE);
          m_commandQueued = ectn_NONE;
        }

        return;
      }
      else
      {
        targets->clear();
        foreach(QString name, *m_checkupdatesStringList)
        {
          PackageListData aux;
          const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(name);
          QString size;
          if (package)
          {
            size = size.number(package->downloadSize, 'f', 0);
          }
          aux = PackageListData(name, m_checkUpdatesNameNewVersion->value(name), size);
          targets->append(aux);
        }
      }
    }

    QString list;
    double totalDownloadSize = 0;

    foreach(PackageListData target, *targets)
    {
      totalDownloadSize += target.downloadSize;
      list = list + target.name + "-" + target.version + "\n";
    }
    list.remove(list.size()-1, 1);

    //Let's build the system upgrade transaction dialog...
    QString ds;
    if (totalDownloadSize > 0)
      ds = Package::kbytesToSize(totalDownloadSize);

    TransactionDialog question(this);

    if(targets->count()==1)
      question.setText(StrConstants::getRetrievePackage() +
                       "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));
    else
      question.setText(StrConstants::getRetrievePackages(targets->count()) +
                       "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));

    question.setWindowTitle(StrConstants::getConfirmation());
    question.setInformativeText(StrConstants::getConfirmationQuestion());
    question.setDetailedText(list);

    m_systemUpgradeDialog = true;
    int result = question.exec();

    if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
    {
      int res = prepareSystemUpgrade();
      if (!res)
      {
        m_systemUpgradeDialog = false;
        enableTransactionActions();
        return;
      }

      if (result == QDialogButtonBox::Yes)
      {
        m_commandExecuting = ectn_SYSTEM_UPGRADE;
        //m_pacmanExec->setSharedMemory(m_sharedMem);
        m_pacmanExec->doSystemUpgrade();
        m_commandQueued = ectn_NONE;
      }
      else if (result == QDialogButtonBox::AcceptRole)
      {
        m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
        //m_pacmanExec->setSharedMemory(m_sharedMem);
        m_pacmanExec->doSystemUpgradeInTerminal();
        m_commandQueued = ectn_NONE;
      }
    }
    else if (result == QDialogButtonBox::No)
    {
      m_systemUpgradeDialog = false;
      enableTransactionActions();
    }
  }
}

/*
 * Removes and Installs all selected packages in one transaction just using:
 * "pacman -R alltoRemove; pacman -S alltoInstall"
 */
void MainWindow::doRemoveAndInstall()
{
  QString listOfRemoveTargets = getTobeRemovedPackages();
  QStringList *pRemoveTargets = Package::getTargetRemovalList(listOfRemoveTargets, m_removeCommand);
  QString removeList;
  QString allLists;
  QStringList pkgsToInstall;

  TransactionDialog question(this);
  QString dialogText;

  QStringList removeTargets = listOfRemoveTargets.split(" ", QString::SkipEmptyParts);
  foreach(QString target, removeTargets)
  {
    removeList = removeList + StrConstants::getRemove() + " "  + target + "\n";
  }

  QString listOfInstallTargets = getTobeInstalledPackages();
  QList<PackageListData> *installTargets = Package::getTargetUpgradeList(listOfInstallTargets);
  QString installList;

  double totalDownloadSize = 0;
  foreach(PackageListData installTarget, *installTargets)
  {
    totalDownloadSize += installTarget.downloadSize;
    installList.append(StrConstants::getInstall() + " " +
                       installTarget.name + "-" + installTarget.version + "\n");
    pkgsToInstall.append(installTarget.name);
  }

  installList.remove(installList.size()-1, 1);

  if (hasPartialUpgrade(pkgsToInstall)) return;

  QString ds = Package::kbytesToSize(totalDownloadSize);

  if (installList.count() == 0)
  {
    installTargets->append(PackageListData(listOfInstallTargets, "", 0));
    installList.append(StrConstants::getInstall() + " " + listOfInstallTargets);
  }

  allLists.append(removeList);
  allLists.append(installList);

  if(removeTargets.count()==1)
  {
    if (pRemoveTargets->at(0).indexOf("HoldPkg was found in") != -1)
    {
      QMessageBox::warning(
            this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
  }
  else if(installTargets->count()==1)
  {
    if (installTargets->at(0).name.indexOf("HoldPkg was found in") != -1)
    {
      QMessageBox::warning(
            this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
  }

  if (removeTargets.count() == 1)
  {
    dialogText = StrConstants::getRemovePackage() + "\n";
  }
  else if (removeTargets.count() > 1)
  {
    dialogText = StrConstants::getRemovePackages(removeTargets.count()) + "\n";
  }
  if (installTargets->count() == 1)
  {
    dialogText += StrConstants::getRetrievePackage() +
      "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB");
  }
  else if (installTargets->count() > 1)
  {
    dialogText += StrConstants::getRetrievePackages(installTargets->count()) +
      "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB");
  }

  question.setText(dialogText);
  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(allLists);
  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    if (!isSUAvailable()) return;

    disableTransactionActions();
    m_progressWidget->setValue(0);
    m_progressWidget->setMaximum(100);
    clearTabOutput();

    m_pacmanExec = new PacmanExec();
    if (m_debugInfo) m_pacmanExec->setDebugMode(true);

    QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

    QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
    QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_REMOVE_INSTALL;
      //m_pacmanExec->setSharedMemory(m_sharedMem);
      m_pacmanExec->doRemoveAndInstall(listOfRemoveTargets, listOfInstallTargets);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      //m_pacmanExec->setSharedMemory(m_sharedMem);
      m_pacmanExec->doRemoveAndInstallInTerminal(listOfRemoveTargets, listOfInstallTargets);
    }
  }
}

/*
 * Removes ALL the packages selected by the user with "pacman -Rcs (CASCADE)" !
 */
void MainWindow::doRemove()
{
  QString listOfTargets = getTobeRemovedPackages();
  m_targets = Package::getTargetRemovalList(listOfTargets, m_removeCommand);
  QString list;

  QStringList targets = listOfTargets.split(" ", QString::SkipEmptyParts);
  foreach(QString target, targets)
  {
    list = list + target + "\n";
  }

  TransactionDialog question(this);

  //Shows a dialog indicating the targets which will be removed and asks for the user's permission.
  if(targets.count()==1)
  {
    if (m_targets->at(0).indexOf("HoldPkg was found in") != -1)
    {
      QMessageBox::warning(
            this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
    else question.setText(StrConstants::getRemovePackage());
  }
  else
    question.setText(StrConstants::getRemovePackages(targets.count()));

  if (getNumberOfTobeRemovedPackages() < targets.count())
    question.setWindowTitle(StrConstants::getWarning());
  else
    question.setWindowTitle(StrConstants::getConfirmation());

  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    if (!isSUAvailable()) return;

    disableTransactionActions();
    m_progressWidget->setValue(0);
    m_progressWidget->setMaximum(100);
    clearTabOutput();

    m_pacmanExec = new PacmanExec();
    if (m_debugInfo) m_pacmanExec->setDebugMode(true);

    QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

    QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
    QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_REMOVE;
      //m_pacmanExec->setSharedMemory(m_sharedMem);
      m_pacmanExec->doRemove(listOfTargets);
    }

    if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      //m_pacmanExec->setSharedMemory(m_sharedMem);
      m_pacmanExec->doRemoveInTerminal(listOfTargets);
    }
  }
}

/*
 * If the Pacman lock file exists ("/var/run/pacman.lck"), removes it!
 */
/*bool MainWindow::doRemovePacmanLockFile()
{
  //If there are no means to run the actions, we must warn!
  if (!isSUAvailable()) return false;

  if (PacmanExec::isDatabaseLocked())
  {
    int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                    StrConstants::getRemovePacmanTransactionLockFileConfirmation(),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
      qApp->processEvents();

      clearTabOutput();
      writeToTabOutput("<b>" + StrConstants::getRemovingPacmanTransactionLockFile() + "</b><br>");
      PacmanExec::removeDatabaseLock();
      writeToTabOutput("<b>" + StrConstants::getCommandFinishedOK() + "</b>");
    }
    else
    {
      return false;
    }
  }

  return true;
}*/

/*
 * Installs the selected package with "yaourt -S"
 */
void MainWindow::doInstallAURPackage()
{
  const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (selectionModel == NULL || selectionModel->selectedRows().count() < 1 || m_hasAURTool == false) {
    std::cerr << "Octopi could not install selection using AUR tool" << std::endl;
    return;
  }
  QString listOfTargets;
  QModelIndexList selectedRows = selectionModel->selectedRows();
  foreach(QModelIndex item, selectedRows)
  {
    const PackageRepository::PackageData*const package = m_packageModel->getData(item);
    if (package == nullptr) {
      assert(false);
      continue;
    }
    if (package->repository != StrConstants::getForeignRepositoryName()) {
      std::cerr << "Octopi could not install selection using " <<
                   Package::getForeignRepositoryToolName().toLatin1().data() << std::endl;
      return;
    }

    if (Package::getForeignRepositoryToolName() != ctn_PACAUR_TOOL &&
        Package::getForeignRepositoryToolName() != ctn_KCP_TOOL)
      listOfTargets += StrConstants::getForeignRepositoryTargetPrefix() + package->name + " ";
    else
      listOfTargets += package->name + " ";
  }

  if (listOfTargets.isEmpty()) {
    std::cerr << "Octopi could not install selection using " <<
                 Package::getForeignRepositoryToolName().toLatin1().data() << std::endl;
    return;
  }

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_pacmanExec->doAURInstall(listOfTargets);
}

/*
 * Begins the download thread of temporary yay-bin
 */
void MainWindow::doPreDownloadTempYay()
{
  if (!isInternetAvailable()) return;

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();
  writeToTabOutput(StrConstants::getDownloadingTempYay() + "<br>");

  //Here we start the download thread... and bind its finished state to doInstallYayPackage()
  QFuture<bool> f;
  QEventLoop el;
  f = QtConcurrent::run(installTempYayHelper);
  connect(&g_fwInstallTempYayHelper, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwInstallTempYayHelper.setFuture(f);
  el.exec();
  doInstallYayPackage();
}

/*
 * Installs yay-bin AUR package using temporary yay-bin
 */
void MainWindow::doInstallYayPackage()
{
  bool downloadedYay = g_fwInstallTempYayHelper.result();
  if (!downloadedYay)
  {
    //We print a message saying the download did not go right
    writeToTabOutput(StrConstants::getErrorCouldNotDownloadTempYay() + "<br>");
    enableTransactionActions();
  }

  writeToTabOutput(StrConstants::getTempYayDownloaded() + "<br>");
  m_pacmanExec = new PacmanExec();
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  //m_tempYayInstalledYay = true;
  m_commandExecuting = ectn_INSTALL_YAY;
  m_pacmanExec->doInstallYayUsingTempYay();
}

/*
 * Removes the selected package with "yaourt -R"
 */
void MainWindow::doRemoveAURPackage()
{
  //If there are no means to run the actions, we must warn!
  if (!isSUAvailable()) return;

  QString listOfTargets;
  QModelIndexList selectedRows = ui->tvPackages->selectionModel()->selectedRows();
  foreach(QModelIndex item, selectedRows)
  {
    const PackageRepository::PackageData*const package = m_packageModel->getData(item);
    if (package == nullptr) {
      assert(false);
      continue;
    }

    listOfTargets += package->name + " ";
  }

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_pacmanExec->doAURRemove(listOfTargets);
}

/*
 * Whenever user chooses another AUR tool in OptionsDialog
 */
void MainWindow::onAURToolChanged()
{
  if (SettingsManager::getAURToolName() == ctn_NO_AUR_TOOL)
  {
    if (m_actionSwitchToAURTool->isChecked())
    {
      ui->actionUseInstantSearch->setEnabled(true);
      ui->twProperties->setTabEnabled(ctn_TABINDEX_ACTIONS, true);

      if (ui->actionUseInstantSearch->isChecked())
      {
        disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
        disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
        connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      }

      m_packageModel->applyFilter("");
      static QStandardItemModel emptyModel;
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
      m_cachedPackageInInfo="";

      //Let's clear the list of visited packages (pkg anchors in Info tab)
      m_listOfVisitedPackages.clear();
      m_indOfVisitedPackage = 0;

      switchToViewAllPackages();
      m_selectedRepository = "";
      m_actionRepositoryAll->setChecked(true);
      m_refreshPackageLists = false;

      if (!ui->actionUseInstantSearch->isChecked())
      {
        disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
        connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
      }

      m_actionSwitchToAURTool->setChecked(false);
      metaBuildPackageList();
      refreshInfoAndFileTabs();
    }

    m_hasAURTool = false;
    //m_actionSwitchToAURTool->setVisible(false);
    m_refreshForeignPackageList = false;
    m_outdatedAURPackagesNameVersion->clear();
    m_outdatedAURStringList->clear();

    m_actionSwitchToAURTool->setText("");
    m_actionSwitchToAURTool->setToolTip("AUR");
    m_actionSwitchToAURTool->setCheckable(false);
    m_actionSwitchToAURTool->setChecked(false);
  }
  else //We are using pacaur/yaourt tool
  {
    m_hasAURTool = true;

    if (!isAURGroupSelected())
    {
      //m_actionSwitchToAURTool->setVisible(true);
      m_actionSwitchToAURTool->setCheckable(true);
      m_actionSwitchToAURTool->setChecked(false);
    }

    m_actionSwitchToAURTool->setText(StrConstants::getUseAURTool());
    m_actionSwitchToAURTool->setToolTip(m_actionSwitchToAURTool->text() + "  (Ctrl+Shift+Y)");
    m_refreshForeignPackageList = true;
  }

  refreshHelpUsageText();
  metaBuildPackageList();
}

/*
 * Whenever user updates AUR voting settings in OptionsDialog
 */
void MainWindow::onAURVotingChanged()
{
  if(SettingsManager::getEnableAURVoting())
  {
    if (m_aurVote!=nullptr) delete m_aurVote;
    m_aurVote = new AurVote(this);
    m_aurVote->setUserName(SettingsManager::getAURUserName());
    m_aurVote->setPassword(SettingsManager::getAURPassword());
    m_aurVote->login();
  }
}

/*
 * Checks if the user is trying to install ONLY SOME of the outdated packages
 */
bool MainWindow::hasPartialUpgrade(QStringList &pkgsToInstall)
{
  if (m_numberOfOutdatedPackages == 0) return false;

  bool result = false;
  pkgsToInstall.sort();

  if (m_numberOfOutdatedPackages > 0 && m_numberOfOutdatedPackages > pkgsToInstall.count())
  {
    foreach(QString n, pkgsToInstall)
    {
      if (m_outdatedStringList->contains(n))
      {
        result = true;
      }
    }
  }
  else if (pkgsToInstall.count() > m_numberOfOutdatedPackages)
  {
    int found=0;

    foreach(QString n, pkgsToInstall)
    {
      if (m_outdatedStringList->contains(n)) found++;
    }

    if (found != m_numberOfOutdatedPackages)
    {
      result = true;
    }
  }
  else if (m_numberOfOutdatedPackages == pkgsToInstall.count())
  {
    //We have to compare the lists...
    if (*m_outdatedStringList != pkgsToInstall)
    {
      result = true;
    }
  }

  if (result == true)
  {
    QMessageBox::warning(
          this, StrConstants::getAttention(), StrConstants::getPartialUpdatesNotSupported(), QMessageBox::Ok);
  }

  return result;
}

/*
 * Installs ALL the packages selected by the user with "pacman -S (INCLUDING DEPENDENCIES)" !
 */
void MainWindow::doInstall()
{
  QString listOfTargets = getTobeInstalledPackages();
  QList<PackageListData> *targets = Package::getTargetUpgradeList(listOfTargets); 
  QString list;
  QStringList pkgsToInstall;

  double totalDownloadSize = 0;
  foreach(PackageListData target, *targets)
  {
    totalDownloadSize += target.downloadSize;
    pkgsToInstall.append(target.name);
    list = list + target.name + "-" + target.version + "\n";
  }

  list.remove(list.size()-1, 1);

  if (hasPartialUpgrade(pkgsToInstall)) return;

  QString ds = Package::kbytesToSize(totalDownloadSize);

  if (list.count() == 0)
  {
    targets->append(PackageListData(listOfTargets, "", 0));
    list.append(listOfTargets);
  }

  TransactionDialog question(this);

  if(targets->count()==1)
  {
    if (targets->at(0).name.indexOf("HoldPkg was found in") != -1)
    {
      QMessageBox::warning(
            this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
    else question.setText(StrConstants::getRetrievePackage() +
                          "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));
  }
  else
    question.setText(StrConstants::getRetrievePackages(targets->count()) +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds).remove(" KB"));

  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    if (!isSUAvailable()) return;

    disableTransactionActions();
    m_progressWidget->setValue(0);
    m_progressWidget->setMaximum(100);
    clearTabOutput();

    m_pacmanExec = new PacmanExec();
    if (m_debugInfo) m_pacmanExec->setDebugMode(true);

    QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

    QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
    QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_INSTALL;
      //m_pacmanExec->setSharedMemory(m_sharedMem);
      m_pacmanExec->doInstall(listOfTargets);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      //m_pacmanExec->setSharedMemory(m_sharedMem);
      m_pacmanExec->doInstallInTerminal(listOfTargets);
    }
  }
}

/*
 * Installs ALL the packages manually selected by the user with "pacman -U (INCLUDING DEPENDENCIES)" !
 */
void MainWindow::doInstallLocalPackages()
{
  QString listOfTargets;
  QString list;
  QFileInfo fi;

  foreach(QString target, m_packagesToInstallList)
  {
    fi.setFile(target);
    list = list + fi.fileName() + "\n";
  }

  foreach(QString pkgToInstall, m_packagesToInstallList)
  {
    listOfTargets += pkgToInstall + " ";
  }

  TransactionDialog question(this);
  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);

  if(m_packagesToInstallList.count()==1)
  {
    if (m_packagesToInstallList.at(0).indexOf("HoldPkg was found in") != -1)
    {
      QMessageBox::warning(
            this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
    else question.setText(StrConstants::getRetrievePackage());
  }
  else
    question.setText(StrConstants::getRetrievePackages(m_packagesToInstallList.count()));

  int result = question.exec();
  qApp->processEvents();
  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    if (!isSUAvailable()) return;

    disableTransactionActions();
    m_progressWidget->setValue(0);
    m_progressWidget->setMaximum(100);
    clearTabOutput();

    m_pacmanExec = new PacmanExec();
    if (m_debugInfo) m_pacmanExec->setDebugMode(true);

    QObject::connect(m_pacmanExec, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

    QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
    QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_INSTALL;
      //m_pacmanExec->setSharedMemory(m_sharedMem);
      m_pacmanExec->doInstallLocal(listOfTargets);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      //m_pacmanExec->setSharedMemory(m_sharedMem);
      m_pacmanExec->doInstallLocalInTerminal(listOfTargets);
    }
  }
}

/*
 * Disables all Transaction related actions
 */
void MainWindow::disableTransactionActions()
{
  toggleSystemActions(false);
  toggleTransactionActions(false);
}

/*
 * Enables all Transaction related actions
 */
void MainWindow::enableTransactionActions()
{
  toggleTransactionActions(true);
}

/*
 * Sets with the given boolean the state of all Transaction related actions
 */
void MainWindow::toggleTransactionActions(const bool value)
{
  bool pendingTransaction = areTherePendingActions();

  if (value == true && pendingTransaction == true)
  {
    ui->actionApply->setEnabled(true);
    ui->actionCancel->setEnabled(true);

    if(m_hasMirrorCheck) m_actionMenuMirrorCheck->setEnabled(false);
    if(m_hasAURTool) m_actionSwitchToAURTool->setEnabled(false);

    ui->actionCheckUpdates->setEnabled(false);
    ui->actionSystemUpgrade->setEnabled(false);
  }
  else if (value == true && pendingTransaction == false)
  {
    ui->actionApply->setEnabled(false);
    ui->actionCancel->setEnabled(false);

    if(m_hasMirrorCheck) m_actionMenuMirrorCheck->setEnabled(true);
    if(m_hasAURTool && m_commandExecuting == ectn_NONE && m_initializationCompleted) m_actionSwitchToAURTool->setEnabled(true);

    if (!isAURGroupSelected())
    {
      ui->actionCheckUpdates->setEnabled(true);
      if (m_outdatedStringList->count() > 0)
        ui->actionSystemUpgrade->setEnabled(true);
    }
    else
    {
      ui->actionCheckUpdates->setEnabled(false);
      ui->actionSystemUpgrade->setEnabled(false);
    }
  }
  else if (value == false)
  {
    ui->actionApply->setEnabled(false);
    ui->actionCancel->setEnabled(false);

    if(m_hasMirrorCheck) m_actionMenuMirrorCheck->setEnabled(false);
    if(m_hasAURTool) m_actionSwitchToAURTool->setEnabled(false);

    ui->actionCheckUpdates->setEnabled(false);
    ui->actionSystemUpgrade->setEnabled(false);
  }

  ui->actionInstall->setEnabled(value);
  ui->actionInstallGroup->setEnabled(value);
  ui->actionInstallAUR->setEnabled(value);
  m_actionInstallPacmanUpdates->setEnabled(value);
  m_actionInstallAURUpdates->setEnabled(value);

  ui->actionRemoveTransactionItem->setEnabled(value);
  ui->actionRemoveTransactionItems->setEnabled(value);
  ui->actionRemove->setEnabled(value);

  ui->actionPacmanLogViewer->setEnabled(value);
  ui->actionCacheCleaner->setEnabled(value);
  ui->actionRepositoryEditor->setEnabled(value);
  m_actionSysInfo->setEnabled(value);

  if (value == true && m_initializationCompleted) m_actionSwitchToAURTool->setEnabled(value);

  ui->actionGetNews->setEnabled(value);  
  if (!isAURGroupSelected()) ui->actionInstallLocalPackage->setEnabled(value);
  m_actionMenuOptions->setEnabled(value);
  ui->actionHelpUsage->setEnabled(value);
  ui->actionDonate->setEnabled(value);
  ui->actionHelpAbout->setEnabled(value);
  ui->actionExit->setEnabled(value);

  //We have to toggle the combobox groups as well
  if (m_initializationCompleted) ui->twGroups->setEnabled(value);
}

void MainWindow::toggleSystemActions(const bool value)
{
  if (value == true && m_commandExecuting != ectn_NONE) return;

  if(m_hasMirrorCheck)
  {
    m_actionMenuMirrorCheck->setEnabled(value);
  }

  if (isAURGroupSelected() && Package::getForeignRepositoryToolName() == ctn_KCP_TOOL)
  {
    ui->actionCheckUpdates->setEnabled(true);
  }
  else if (Package::getForeignRepositoryToolName() != ctn_KCP_TOOL)
  {
    ui->actionCheckUpdates->setEnabled(value);
  }
  else if (!isAURGroupSelected())
    m_actionMenuOptions->setEnabled(value);

  ui->actionInstallLocalPackage->setEnabled(value);
  ui->actionGetNews->setEnabled(value);

  if (value == true && m_outdatedStringList->count() > 0)
    ui->actionSystemUpgrade->setEnabled(true);
  else
    ui->actionSystemUpgrade->setEnabled(false);
}

/*
 * Triggers the especific methods that need to be called given the packages in the transaction
 */
void MainWindow::commitTransaction()
{
  if (!isInternetAvailable()) return;

  //Are there any remove actions to be commited?
  if(getRemoveTransactionParentItem()->rowCount() > 0 && getInstallTransactionParentItem()->rowCount() > 0)
  {
    doRemoveAndInstall();
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

/*
 * Clears the transaction treeview
 */
void MainWindow::cancelTransaction()
{
  int res = QMessageBox::question(this,
                        StrConstants::getConfirmation(),
                        StrConstants::getCancelActionsConfirmation(),
                        QMessageBox::Yes|QMessageBox::No,
                        QMessageBox::No);

  if(res == QMessageBox::Yes)
  {
    clearTransactionTreeView();
  }
}

/*
 * Kills the running pacman command
 */
void MainWindow::stopTransaction()
{
  if (m_commandExecuting != ectn_NONE && m_pacmanExec != NULL)
  {
    m_pacmanExec->cancelProcess();
  }
}

/*
 * Called whenever Octopi's parser detects a potential for enabling/disabling stop transaction button
 */
void MainWindow::onCanStopTransaction(bool yesNo)
{
  if (yesNo == true && m_progressWidget->isHidden()) return;
  if (SettingsManager::getShowStopTransaction()) m_toolButtonStopTransaction->setVisible(yesNo);
}

/*
 * This SLOT is called when Pacman's process has finished execution [PacmanExec based!!!]
 */
void MainWindow::pacmanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  UnixCommand::removeTemporaryFiles();

  bool bRefreshGroups = true;
  m_progressWidget->close();

  m_progressWidget->setValue(0);
  m_progressWidget->show();

  if (SettingsManager::getShowStopTransaction()) m_toolButtonStopTransaction->setVisible(false);
  ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName());

  if (m_commandExecuting == ectn_CHECK_UPDATES)
  {
    QStringList pkgs = m_pacmanExec->getOutdatedPackages();

    //We have to treat outdated pkgs list...
    //Each pkg is in this format: libpng 1.6.36-1 -> 1.6.37-1
    if (pkgs.count() > 0)
    {
      m_checkupdatesStringList->clear();
      m_checkUpdatesNameNewVersion->clear();

      foreach(QString pkg, pkgs)
      {
        QStringList names = pkg.split(" ", QString::SkipEmptyParts);
        if (names.count() > 0)
        {
          m_checkupdatesStringList->append(names.at(0));
          m_checkUpdatesNameNewVersion->insert(names.at(0), names.at(3));
        }
      }
    }
    else if (pkgs.count()==0)
    {
      writeToTabOutput(StrConstants::getNoUpdatesAvailable() + "<br>");
    }
  }

  //mate-terminal is returning code 255 sometimes...
  if ((exitCode == 0 || exitCode == 255 ||
       (exitCode == 1 && m_commandExecuting == ectn_CHECK_UPDATES)) && exitStatus == QProcess::NormalExit)
  {
    //First, we empty the tabs cache!
    m_cachedPackageInInfo = "";
    m_cachedPackageInFiles = "";
    writeToTabOutput("<br><b>" + StrConstants::getCommandFinishedOK() + "</b><br>");
  }
  else if (exitCode == ctn_PACMAN_PROCESS_EXECUTING)
  {
    writeToTabOutput(StrConstants::getErrorPacmanProcessExecuting() + "<br>");
    writeToTabOutput("<br><b>" + StrConstants::getCommandFinishedWithErrors() + "</b><br>");
  }
  else
  {
    writeToTabOutput("<br><b>" + StrConstants::getCommandFinishedWithErrors() + "</b><br>");
  }

  if(m_commandQueued == ectn_SYSTEM_UPGRADE)
  {
    //Did it synchronize any repo? If so, let's refresh some things...
    if (textInTabOutput(StrConstants::getSyncing()))
    {
      bool aurGroup = isAURGroupSelected();

      if (!aurGroup)
      {
        metaBuildPackageList();
      }
    }

    doSystemUpgrade();
    m_commandQueued = ectn_NONE;
  }
  else if (m_commandQueued == ectn_NONE)
  {
    if(exitCode == 0 || exitCode == 255) //mate-terminal is returning code 255 sometimes...
    {
      //After the command, we can refresh the package list, so any change can be seem.
      if (m_commandExecuting == ectn_SYNC_DATABASE || m_commandExecuting == ectn_CHECK_UPDATES)
      {
        //Sets NOW as the last sync time value
        SettingsManager::setLastCheckUpdatesTime(QDateTime::currentDateTime());

        //Retrieves the RSS News from respective Distro site...
        refreshDistroNews(true, false);

        //Did it synchronize any repo? If so, let's refresh some things...
        if (UnixCommand::isAppRunning("octopi-notifier", true) ||
            (IsSyncingRepoInTabOutput()))
        {
          bool aurGroup = isAURGroupSelected();

          if (!aurGroup)
          {
            metaBuildPackageList();
          }
          else if (Package::getForeignRepositoryToolName() == ctn_KCP_TOOL)
          {
            metaBuildPackageList();
            delete m_pacmanExec;
            m_commandExecuting = ectn_NONE;
            enableTransactionActions();
            m_progressWidget->close();
            return;
          }
        }        
        if (m_outdatedStringList->count() > 0)
          execCommandInAnotherThread(ctn_PACMAN_SUP_COMMAND);
      }
      else if (m_commandExecuting == ectn_SYSTEM_UPGRADE ||
               m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL)
      {        
        m_checkupdatesStringList->clear();
        m_checkUpdatesNameNewVersion->clear();
        metaBuildPackageList();
      }
      else if (m_commandExecuting != ectn_MIRROR_CHECK && m_commandExecuting != ectn_CHECK_UPDATES)
      {
        //If we are in a package group, maybe we have installed/removed something, so...
        if (!isAURGroupSelected())
        {
          metaBuildPackageList();
        }
        else
        {
          bRefreshGroups = false;
          metaBuildPackageList();
        }
      }

      clearTransactionTreeView();

      //Does it still need to upgrade another packages due to SyncFirst issues???
      if ((m_commandExecuting == ectn_SYSTEM_UPGRADE ||
           m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL)
          && m_outdatedStringList->count() > 0)
      {
        m_commandExecuting = ectn_NONE;
        doSystemUpgrade();
        m_progressWidget->close();
        return;
      }
    }
  }

  if (exitCode != 0 && (textInTabOutput("conflict"))) //|| _textInTabOutput("could not satisfy dependencies")))
  {
    int res = QMessageBox::question(this, StrConstants::getThereHasBeenATransactionError(),
                                    StrConstants::getConfirmExecuteTransactionInTerminal(),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
      m_pacmanExec->runLatestCommandInTerminal();
      m_progressWidget->close();
      return;
    }
  }

  enableTransactionActions();
  if (isAURGroupSelected())
  {
    toggleSystemActions(false);
  }

  if ((m_commandExecuting != ectn_MIRROR_CHECK && m_commandExecuting != ectn_CHECK_UPDATES) && bRefreshGroups)
    refreshGroupsWidget();

  refreshMenuTools(); //Maybe some of octopi tools were added/removed...

  QStringList aurTools = UnixCommand::getAvailableAURTools();
  if(aurTools.count() > 1 && SettingsManager::getAURToolName() != ctn_NO_AUR_TOOL)
  {
    m_actionSwitchToAURTool->setCheckable(true);
    m_actionSwitchToAURTool->setChecked(false);
    m_actionSwitchToAURTool->setText(StrConstants::getUseAURTool());
    m_actionSwitchToAURTool->setToolTip(m_actionSwitchToAURTool->text() + "  (Ctrl+Shift+Y)");
  }
  else if (aurTools.count() > 1) //It seems the AUR tool has just been removed...
  {
    m_actionSwitchToAURTool->setText("");
    m_actionSwitchToAURTool->setToolTip("AUR");
    m_actionSwitchToAURTool->setCheckable(false);
    m_actionSwitchToAURTool->setChecked(false);
  }

  delete m_pacmanExec;
  if (m_progressWidget->isVisible()) m_progressWidget->close();
  m_commandExecuting = ectn_NONE;

  if (isPackageTreeViewVisible() && !m_leFilterPackage->hasFocus()) m_leFilterPackage->setFocus();
}

/*
 * THIS IS THE COUNTERPART OF "pacmanProcessFinished" FOR QTERMWIDGET AUR COMMANDS
 * Whenever the terminal transaction has finished, we can update the UI
 */
void MainWindow::onPressAnyKeyToContinue()
{
  if (m_commandExecuting == ectn_NONE) return;

  if (m_commandExecuting == ectn_INSTALL_YAY)
  {
    QString octopiConfDir = QDir::homePath() + QDir::separator() + ".config/octopi";
    QString yaySymlink = octopiConfDir + QDir::separator() + "yay";
    QFileInfo info(yaySymlink);
    if (info.isSymLink())
    {
      QFileInfo fi(info.symLinkTarget());
      QFile::remove(yaySymlink);
      QProcess remove;
      remove.start("rm -Rf " + fi.canonicalPath());
      remove.waitForFinished();
    }

    if (UnixCommand::hasTheExecutable("yay"))
    {
      refreshHelpUsageText();
      SettingsManager::setAURTool(ctn_YAY_TOOL);
      m_actionSwitchToAURTool->setToolTip(StrConstants::getUseAURTool());
      m_actionSwitchToAURTool->setToolTip(m_actionSwitchToAURTool->toolTip() + "  (Ctrl+Shift+Y)");
      m_actionSwitchToAURTool->setCheckable(true);
      m_actionSwitchToAURTool->setChecked(false);
    }
  }

  m_progressWidget->setValue(0);
  m_progressWidget->show();
  bool bRefreshGroups = true;
  clearTransactionTreeView();
  metaBuildPackageList();

  if (isAURGroupSelected())
  {
    toggleSystemActions(false);
    bRefreshGroups = false;
  }

  if (m_commandExecuting != ectn_MIRROR_CHECK && bRefreshGroups)
    refreshGroupsWidget();

  refreshMenuTools(); //Maybe some of octopi tools were added/removed...

  enableTransactionActions();

  if (m_pacmanExec == nullptr)
    delete m_pacmanExec;

  m_commandExecuting = ectn_NONE;
  UnixCommand::removeTemporaryFiles();
  m_console->execute("");
  m_console->setFocus();

  if (m_cic)
  {
    delete m_cic;
    m_cic = NULL;
  }
}

/*
 * Whenever a user strikes Ctrl+C, Ctrl+D or Ctrl+Z in the terminal
 */
void MainWindow::onCancelControlKey()
{
  if (m_commandExecuting != ectn_NONE)
  {
    clearTransactionTreeView();
    enableTransactionActions();

    if (m_pacmanExec == nullptr)
      delete m_pacmanExec;

    m_pacmanExec = nullptr;
    m_commandExecuting = ectn_NONE;
    UnixCommand::removeTemporaryFiles();
  }
}

/*
 * A helper method which writes the given string to OutputTab's textbrowser
 */
void MainWindow::writeToTabOutput(const QString &msg, TreatURLLinks treatURLLinks)
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    ensureTabVisible(ctn_TABINDEX_OUTPUT);
    utils::writeToTextBrowser(text, msg, treatURLLinks);
  }
}

/*
 * Sets new percentage value to the progressbar
 */
void MainWindow::incrementPercentage(int percentage)
{
  if (!m_progressWidget->isVisible())
  {
    m_progressWidget->show();
    if (SettingsManager::getShowStopTransaction()) m_toolButtonStopTransaction->setVisible(true);
  }
  m_progressWidget->setValue(percentage);
}

/*
 * A helper method which writes the given string to OutputTab's textbrowser
 */
void MainWindow::outputText(const QString &output)
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {
    if ((m_commandExecuting == ectn_RUN_IN_TERMINAL && SettingsManager::getTerminal() != ctn_QTERMWIDGET) ||
        (m_commandExecuting != ectn_RUN_IN_TERMINAL && SettingsManager::getTerminal() == ctn_QTERMWIDGET))
    {
      ensureTabVisible(ctn_TABINDEX_OUTPUT);
      positionTextEditCursorAtEnd();
    }

    text->insertHtml(output);
    text->ensureCursorVisible();
  }
}
