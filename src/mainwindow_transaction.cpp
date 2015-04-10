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
#include <iostream>
#include <cassert>

#include <QComboBox>
#include <QProgressBar>
#include <QMessageBox>
#include <QStandardItem>
#include <QTextBrowser>

/*
 * Watches the state of tvTransaction treeview to see if Commit/Cancel actions must be activated/deactivated
 */
void MainWindow::changeTransactionActionsState()
{
  bool state = isThereAPendingTransaction();
  ui->actionCommit->setEnabled(state);
  ui->actionCancel->setEnabled(state);
  ui->actionSyncPackages->setEnabled(!state);

  if(m_hasMirrorCheck) m_actionMirrorCheck->setEnabled(!state);
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
bool MainWindow::isThereAPendingTransaction()
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
      ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
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
      ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
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
      ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
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
    int slash = pkgName.indexOf("/");
    QString pkg = pkgName.mid(slash+1);
    QList<QStandardItem *> aux = sim->findItems(pkg, Qt::MatchRecursive | Qt::MatchExactly);

    if (aux.count() == 0) siRemoveParent->appendRow(siPackageToRemove);
  }
  else if (foundItems.size() == 1 && foundItems.at(0)->parent() == siInstallParent)
  {
    siInstallParent->removeRow(foundItems.at(0)->row());
    siRemoveParent->appendRow(siPackageToRemove);
  }

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_TRANSACTION);
  tvTransaction->expandAll();
  changeTransactionActionsState();
}

/*
 * Inserts the given package into the Install parent item of the Transaction treeview
 */
void MainWindow::insertInstallPackageIntoTransaction(const QString &pkgName)
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
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

  ui->twProperties->setCurrentIndex(ctn_TABINDEX_TRANSACTION);
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
void MainWindow::insertIntoRemovePackage()
{
  qApp->processEvents();
  bool checkDependencies=false;
  QStringList dependencies;

  if (!isAURGroupSelected())
  {
    ensureTabVisible(ctn_TABINDEX_TRANSACTION);
    QModelIndexList selectedRows = ui->tvPackages->selectionModel()->selectedRows();

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
      if (package == NULL) {
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
  ensureTabVisible(ctn_TABINDEX_TRANSACTION);
  insertRemovePackageIntoTransaction(getSelectedGroup());
}

/*
 * Inserts the current selected packages for installation into the Transaction Treeview
 * This is the SLOT, it needs to call insertInstallPackageIntoTransaction(PackageName) to work!
 */
void MainWindow::insertIntoInstallPackage()
{
  qApp->processEvents();

  if (!isAURGroupSelected())
  {
    ensureTabVisible(ctn_TABINDEX_TRANSACTION);

    QModelIndexList selectedRows = ui->tvPackages->selectionModel()->selectedRows();
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
      if (package == NULL) {
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

  if (package != NULL) repo = package->repository;

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
  QStringList optDeps = Package::getOptionalDeps(packageName); //si->text());
  QList<const PackageRepository::PackageData*> optionalPackages;

  foreach(QString optDep, optDeps)
  {
    QString candidate = optDep;
    int points = candidate.indexOf(":");
    candidate = candidate.mid(0, points).trimmed();

    const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(candidate);
    if(!isPackageInInstallTransaction(candidate) &&
       !isPackageInstalled(candidate) && package != 0)
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
    if (package != NULL && package->installed() && !isPackageInRemoveTransaction(dep))
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

      if(dep->repository == StrConstants::getForeignRepositoryName() && dep->description.isEmpty())
      {
        desc = Package::getInformationDescription(dep->name, true);
      }

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
  ensureTabVisible(ctn_TABINDEX_TRANSACTION);
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
 * Whenever the user presses DEL over the Transaction TreeView, we:
 * - Delete the package if it's bellow of "To be removed" or "To be installed" parent;
 * - Delete all the parent's packages if the user clicked in "To be removed" or "To be installed" items.
 */
void MainWindow::onPressDelete()
{
  QTreeView *tvTransaction =
      ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");

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

/*
 * Checks if some SU utility is available...
 * Returns false if not!
 */
bool MainWindow::isSUAvailable()
{
  //If there are no means to run the actions, we must warn!
  if (UnixCommand::isRootRunning() && WMHelper::isKDERunning())
  {
    return true;
  }
  else if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
    QMessageBox::critical( 0, StrConstants::getApplicationName(),
                           StrConstants::getErrorNoSuCommand() +
                           "\n" + StrConstants::getYoullNeedSuFrontend());
    return false;
  }
  else
    return true;
}

/*
 * This is KaOS specific code which uses mirror-check tool.
 */
void MainWindow::doMirrorCheck()
{
  if (m_commandExecuting != ectn_NONE ||
      !UnixCommand::hasInternetConnection()) return;

  m_commandExecuting = ectn_MIRROR_CHECK;
  disableTransactionActions();

  m_unixCommand = new UnixCommand(this);

  clearTabOutput();
  writeToTabOutput("<b>" + StrConstants::getSyncMirror() + "</b><br><br>");

  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError()),
                   this, SLOT( actionsProcessReadOutputErrorMirrorCheck()));

  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                   this, SLOT( actionsProcessReadOutputMirrorCheck()));
  QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));

  QString command = ctn_MIRROR_CHECK_APP;
  m_unixCommand->executeCommandAsNormalUser(command);
}

/*
 * Does a repository sync with "pacman -Sy" !
 */
void MainWindow::doSyncDatabase()
{
  if (!doRemovePacmanLockFile()) return;

  //Let's synchronize kcp database too...
  if (UnixCommand::getLinuxDistro() == ectn_KAOS && UnixCommand::hasTheExecutable("kcp"))
    UnixCommand::execCommandAsNormalUser("kcp -u");

  m_commandExecuting = ectn_SYNC_DATABASE;
  disableTransactionActions();
  m_unixCommand = new UnixCommand(this);

  QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                   this, SLOT( actionsProcessReadOutput() ));
  QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                   this, SLOT( actionsProcessRaisedError() ));

  QString command;

  if (UnixCommand::isRootRunning())
    command = "pacman -Sy";
  else
    command = "pacman -Syy";

  m_unixCommand->executeCommand(command);
}

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

  m_lastCommandList.clear();

  if (StrConstants::getForeignRepositoryToolName() == "pacaur")
  {
    m_lastCommandList.append(StrConstants::getForeignRepositoryToolName() + " -Sa " + listOfTargets + ";");
  }
  else if (StrConstants::getForeignRepositoryToolName() == "yaourt")
  {
    m_lastCommandList.append(StrConstants::getForeignRepositoryToolName() + " -S " + listOfTargets + ";");
  }

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  disableTransactionActions();
  m_unixCommand = new UnixCommand(this);

  QObject::connect(m_unixCommand, SIGNAL( startedTerminal() ), this, SLOT( actionsProcessStarted()));
  QObject::connect(m_unixCommand, SIGNAL( finishedTerminal ( int, QProcess::ExitStatus )),
                   this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_unixCommand->runCommandInTerminalAsNormalUser(m_lastCommandList);
}

/*
 * doSystemUpgrade shared code ...
 */
void MainWindow::prepareSystemUpgrade()
{
  m_systemUpgradeDialog = false;
  if (!doRemovePacmanLockFile()) return;

  m_lastCommandList.clear();
  m_lastCommandList.append("pacman -Su;");
  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_unixCommand = new UnixCommand(this);

  QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                   this, SLOT( actionsProcessReadOutput() ));
  QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                   this, SLOT( actionsProcessRaisedError() ));

  disableTransactionActions();
}

/*
 * Does a system upgrade with "pacman -Su" !
 */
void MainWindow::doSystemUpgrade(SystemUpgradeOptions systemUpgradeOptions)
{
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

  qApp->processEvents();

  if(systemUpgradeOptions == ectn_SYNC_DATABASE_OPT)
  {
    m_commandQueued = ectn_SYSTEM_UPGRADE;
    doSyncDatabase();
  }
  else
  {
    //Shows a dialog indicating the targets needed to be retrieved and asks for the user's permission.
    QList<PackageListData> * targets = Package::getTargetUpgradeList();

    //There are no new updates to install!
    if (targets->count() == 0 && m_outdatedStringList->count() == 0)
    {
      clearTabOutput();
      writeToTabOutputExt("<b>" + StrConstants::getNoNewUpdatesAvailable() + "</b>");
      return;
    }
    else if (targets->count() == 0 && m_outdatedStringList->count() > 0)
    {
      //This is a bug and should be shown to the user!
      clearTabOutput();
      writeToTabOutputExt(UnixCommand::getTargetUpgradeList());
      return;
    }

    QString list;
    double totalDownloadSize = 0;

    foreach(PackageListData target, *targets)
    {
      totalDownloadSize += target.downloadSize;
      list = list + target.name + "-" + target.version + "\n";
    }
    list.remove(list.size()-1, 1);

    //User already confirmed all updates in the notifier window!
    if (systemUpgradeOptions == ectn_NOCONFIRM_OPT)
    {
      prepareSystemUpgrade();

      m_commandExecuting = ectn_SYSTEM_UPGRADE;

      QString command;
      command = "pacman -Su --noconfirm";

      m_unixCommand->executeCommand(command);
      m_commandQueued = ectn_NONE;
    }
    else
    {
      //Let's build the system upgrade transaction dialog...
      totalDownloadSize = totalDownloadSize / 1024;
      QString ds = Package::kbytesToSize(totalDownloadSize);

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
        prepareSystemUpgrade();

        if (result == QDialogButtonBox::Yes)
        {
          m_commandExecuting = ectn_SYSTEM_UPGRADE;

          QString command;
          command = "pacman -Su --noconfirm";

          m_unixCommand->executeCommand(command);
          m_commandQueued = ectn_NONE;
        }
        else if (result == QDialogButtonBox::AcceptRole)
        {
          m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
          m_unixCommand->runCommandInTerminal(m_lastCommandList);
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
  }
  installList.remove(installList.size()-1, 1);

  totalDownloadSize = totalDownloadSize / 1024;
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
    if (!doRemovePacmanLockFile()) return;

    QString command;
    command = "pacman -R --noconfirm " + listOfRemoveTargets +
        "; pacman -S --noconfirm " + listOfInstallTargets;

    m_lastCommandList.clear();
    m_lastCommandList.append("pacman -R " /*+ m_removeCommand + " "*/ + listOfRemoveTargets + ";");
    m_lastCommandList.append("pacman -S " + listOfInstallTargets + ";");
    m_lastCommandList.append("echo -e;");
    m_lastCommandList.append("read -n1 -p \"" + StrConstants::getPressAnyKey() + "\"");

    m_unixCommand = new UnixCommand(this);

    QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                     this, SLOT( actionsProcessReadOutput() ));
    QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                     this, SLOT( actionsProcessRaisedError() ));

    disableTransactionActions();

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_REMOVE_INSTALL;
      m_unixCommand->executeCommand(command);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_unixCommand->runCommandInTerminal(m_lastCommandList);
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
    if (!doRemovePacmanLockFile()) return;

    QString command;
    command = "pacman -R --noconfirm " + listOfTargets;

    m_lastCommandList.clear();
    m_lastCommandList.append("pacman -R " + /*m_removeCommand +*/ listOfTargets + ";");
    m_lastCommandList.append("echo -e;");
    m_lastCommandList.append("read -n1 -p \"" + StrConstants::getPressAnyKey() + "\"");

    m_unixCommand = new UnixCommand(this);

    QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                     this, SLOT( actionsProcessReadOutput() ));
    QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                     this, SLOT( actionsProcessRaisedError() ));

    disableTransactionActions();

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_REMOVE;
      m_unixCommand->executeCommand(command);
    }

    if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_unixCommand->runCommandInTerminal(m_lastCommandList);
    }
  }
}

/*
 * If the Pacman lock file exists ("/var/run/pacman.lck"), removes it!
 */
bool MainWindow::doRemovePacmanLockFile()
{
  //If there are no means to run the actions, we must warn!
  if (!isSUAvailable()) return false;

  QString lockFilePath("/var/lib/pacman/db.lck");
  QFile lockFile(lockFilePath);

  if (lockFile.exists())
  {
    int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                    StrConstants::getRemovePacmanTransactionLockFileConfirmation(),
                                    QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

    if (res == QMessageBox::Yes)
    {
      qApp->processEvents();

      clearTabOutput();
      writeToTabOutputExt("<b>" + StrConstants::getRemovingPacmanTransactionLockFile() + "</b>");
      UnixCommand::execCommand("rm " + lockFilePath);
      writeToTabOutputExt("<b>" + StrConstants::getCommandFinishedOK() + "</b>");
    }
  }

  return true;
}

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
    if (package == NULL) {
      assert(false);
      continue;
    }
    if (package->repository != StrConstants::getForeignRepositoryName()) {
      std::cerr << "Octopi could not install selection using " <<
                   StrConstants::getForeignRepositoryToolName().toLatin1().data() << std::endl;
      return;
    }

    if (StrConstants::getForeignRepositoryToolName() != "pacaur" &&
        StrConstants::getForeignRepositoryToolName() != "kcp")
      listOfTargets += StrConstants::getForeignRepositoryTargetPrefix() + package->name + " ";
    else
      listOfTargets += package->name + " ";
  }

  if (listOfTargets.isEmpty()) {
    std::cerr << "Octopi could not install selection using " <<
                 StrConstants::getForeignRepositoryToolName().toLatin1().data() << std::endl;
    return;
  }

  m_lastCommandList.clear();

  if (UnixCommand::getLinuxDistro() == ectn_KAOS)
    m_lastCommandList.append(StrConstants::getForeignRepositoryToolName() + " -i " + listOfTargets + ";");
  else if (StrConstants::getForeignRepositoryToolName() == "pacaur")
    m_lastCommandList.append(StrConstants::getForeignRepositoryToolName() + " -Sa " + listOfTargets + ";");
  else if (StrConstants::getForeignRepositoryToolName() == "yaourt" ||
           StrConstants::getForeignRepositoryToolName() == "ccr")
    m_lastCommandList.append(StrConstants::getForeignRepositoryToolName() + " -S " + listOfTargets + ";");

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  disableTransactionActions();
  m_unixCommand = new UnixCommand(this);

  QObject::connect(m_unixCommand, SIGNAL( startedTerminal() ), this, SLOT( actionsProcessStarted()));
  QObject::connect(m_unixCommand, SIGNAL( finishedTerminal ( int, QProcess::ExitStatus )),
                   this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_unixCommand->runCommandInTerminalAsNormalUser(m_lastCommandList);
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
    if (package == NULL) {
      assert(false);
      continue;
    }

    listOfTargets += package->name + " ";
  }

  m_lastCommandList.clear();

  if (StrConstants::getForeignRepositoryToolName() == "ccr" ||
      StrConstants::getForeignRepositoryToolName() == "kcp")
  {
    m_lastCommandList.append("pacman -" + m_removeCommand + " " + listOfTargets + ";");
  }
  else
  {
    m_lastCommandList.append(StrConstants::getForeignRepositoryToolName() +
                             " -" + m_removeCommand + " " + listOfTargets + ";");
  }

  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  disableTransactionActions();
  m_unixCommand = new UnixCommand(this);

  QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                   this, SLOT( actionsProcessReadOutput() ));
  QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                   this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
  QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                   this, SLOT( actionsProcessRaisedError() ));

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_unixCommand->runCommandInTerminal(m_lastCommandList);
}

/*
 * Installs ALL the packages selected by the user with "pacman -S (INCLUDING DEPENDENCIES)" !
 */
void MainWindow::doInstall()
{
  QString listOfTargets = getTobeInstalledPackages();
  QList<PackageListData> *targets = Package::getTargetUpgradeList(listOfTargets);
  QString list;

  double totalDownloadSize = 0;
  foreach(PackageListData target, *targets)
  {
    totalDownloadSize += target.downloadSize;
    list = list + target.name + "-" + target.version + "\n";
  }
  list.remove(list.size()-1, 1);

  totalDownloadSize = totalDownloadSize / 1024;
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
    if (!doRemovePacmanLockFile()) return;

    QString command;
    command = "pacman -S --noconfirm " + listOfTargets;

    m_lastCommandList.clear();
    m_lastCommandList.append("pacman -S " + listOfTargets + ";");
    m_lastCommandList.append("echo -e;");
    m_lastCommandList.append("read -n1 -p \"" + StrConstants::getPressAnyKey() + "\"");

    disableTransactionActions();
    m_unixCommand = new UnixCommand(this);

    QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
    QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                     this, SLOT( actionsProcessReadOutput() ));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                     this, SLOT( actionsProcessRaisedError() ));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_INSTALL;
      m_unixCommand->executeCommand(command);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_unixCommand->runCommandInTerminal(m_lastCommandList);
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

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    if (!doRemovePacmanLockFile()) return;

    QString command;
    command = "pacman -U --force --noconfirm " + listOfTargets;

    m_lastCommandList.clear();
    m_lastCommandList.append("pacman -U --force " + listOfTargets + ";");
    m_lastCommandList.append("echo -e;");
    m_lastCommandList.append("read -n1 -p \"" + StrConstants::getPressAnyKey() + "\"");

    m_commandExecuting = ectn_INSTALL;
    disableTransactionActions();
    m_unixCommand = new UnixCommand(this);

    QObject::connect(m_unixCommand, SIGNAL( started() ), this, SLOT( actionsProcessStarted()));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardOutput()),
                     this, SLOT( actionsProcessReadOutput() ));
    QObject::connect(m_unixCommand, SIGNAL( finished ( int, QProcess::ExitStatus )),
                     this, SLOT( actionsProcessFinished(int, QProcess::ExitStatus) ));
    QObject::connect(m_unixCommand, SIGNAL( readyReadStandardError() ),
                     this, SLOT( actionsProcessRaisedError() ));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_INSTALL;
      m_unixCommand->executeCommand(command);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_unixCommand->runCommandInTerminal(m_lastCommandList);
    }
  }
}

/*
 * Clears the local package cache using "pacman -Sc"
 */
void MainWindow::doCleanCache()
{
  if (!doRemovePacmanLockFile()) return;

  int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                  StrConstants::getCleanCacheConfirmation(),
                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

  if (res == QMessageBox::Yes)
  {
    qApp->processEvents();

    clearTabOutput();
    writeToTabOutputExt("<b>" + StrConstants::getCleaningPackageCache() + "</b>");
    qApp->processEvents();
    bool res = UnixCommand::cleanPacmanCache();
    qApp->processEvents();

    if (res)
    {
      writeToTabOutputExt("<b>" + StrConstants::getCommandFinishedOK() + "</b>");
    }
    else
      writeToTabOutputExt("<b>" + StrConstants::getCommandFinishedWithErrors() + "</b>");
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
  bool state = isThereAPendingTransaction();
  if (value == true && state == true)
  {
    ui->actionCommit->setEnabled(true);
    ui->actionCancel->setEnabled(true);

    if(m_hasMirrorCheck) m_actionMirrorCheck->setEnabled(false);
    if(m_hasAURTool) m_actionSwitchToAURTool->setEnabled(false);

    ui->actionSyncPackages->setEnabled(false);
    ui->actionSystemUpgrade->setEnabled(false);
  }
  else if (value == true && state == false)
  {
    ui->actionCommit->setEnabled(false);
    ui->actionCancel->setEnabled(false);

    if(m_hasMirrorCheck) m_actionMirrorCheck->setEnabled(true);
    if(m_hasAURTool) m_actionSwitchToAURTool->setEnabled(true);

    ui->actionSyncPackages->setEnabled(true);
    if (value == true && m_outdatedStringList->count() > 0)
      ui->actionSystemUpgrade->setEnabled(true);
  }
  else if (value == false && state == false)
  {
    if(m_hasMirrorCheck) m_actionMirrorCheck->setEnabled(false);
    if(m_hasAURTool) m_actionSwitchToAURTool->setEnabled(false);

    ui->actionSyncPackages->setEnabled(false);
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

  m_actionSwitchToAURTool->setEnabled(value);
  ui->actionGetNews->setEnabled(value);

  ui->actionGetNews->setEnabled(value);
  ui->actionInstallLocalPackage->setEnabled(value);
  ui->actionOpenRootTerminal->setEnabled(value);
  ui->actionHelpUsage->setEnabled(value);
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
    m_actionMirrorCheck->setEnabled(value);
  }

  if (isAURGroupSelected() && StrConstants::getForeignRepositoryToolName() == "kcp")
  {
    ui->actionSyncPackages->setEnabled(true);
  }
  else
  {
    ui->actionSyncPackages->setEnabled(value);
  }

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
                        StrConstants::getCancelTransactionConfirmation(),
                        QMessageBox::Yes|QMessageBox::No,
                        QMessageBox::No);

  if(res == QMessageBox::Yes)
  {
    clearTransactionTreeView();
  }
}

/*
 * This SLOT is called whenever Pacman's process has just started execution
 */
void MainWindow::actionsProcessStarted()
{
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  m_iLoveCandy = UnixCommand::isILoveCandyEnabled();

  clearTabOutput();

  //First we output the name of action we are starting to execute!

  if (m_commandExecuting == ectn_SYNC_DATABASE)
  {
    writeToTabOutput("<b>" + StrConstants::getSyncDatabases() + "</b><br><br>");
  }
  else if (m_commandExecuting == ectn_SYSTEM_UPGRADE || m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL)
  {
    writeToTabOutput("<b>" + StrConstants::getSystemUpgrade() + "</b><br><br>");
  }
  else if (m_commandExecuting == ectn_REMOVE)
  {
    writeToTabOutput("<b>" + StrConstants::getRemovingPackages() + "</b><br><br>");
  }
  else if (m_commandExecuting == ectn_INSTALL)
  {
    writeToTabOutput("<b>" + StrConstants::getInstallingPackages() + "</b><br><br>");
  }
  else if (m_commandExecuting == ectn_REMOVE_INSTALL)
  {
    writeToTabOutput("<b>" + StrConstants::getRemovingAndInstallingPackages() + "</b><br><br>");
  }
  else if (m_commandExecuting == ectn_RUN_IN_TERMINAL)
  {
    writeToTabOutput("<b>" + StrConstants::getRunningCommandInTerminal() + "</b><br><br>");
  }

  QString msg = m_unixCommand->readAllStandardOutput();
  msg = msg.trimmed();

  if (!msg.isEmpty())
  {
    writeToTabOutputExt(msg);
  }
}

/*
 * This SLOT is called when Pacman's process has finished execution
 *
 */
void MainWindow::actionsProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  bool bRefreshGroups = true;
  m_progressWidget->close();

  ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName());

  //mate-terminal is returning code 255 sometimes...
  //TODO: Have to improve this test!
  if ((exitCode == 0 || exitCode == 255) && exitStatus == QProcess::NormalExit)
  {
    //First, we empty the tabs cache!
    m_cachedPackageInInfo = "";
    m_cachedPackageInFiles = "";

    writeToTabOutputExt("<br><b>" +
                     StrConstants::getCommandFinishedOK() + "</b><br>");
  }
  else
  {
    writeToTabOutputExt("<br><b>" +
                     StrConstants::getCommandFinishedWithErrors() + "</b><br>");
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
      if (m_commandExecuting == ectn_SYNC_DATABASE)
      {
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
          else if (StrConstants::getForeignRepositoryToolName() == "kcp")
          {
            metaBuildPackageList();
            delete m_unixCommand;
            m_commandExecuting = ectn_NONE;
            enableTransactionActions();
            return;
          }
        }
      }
      else if (m_commandExecuting == ectn_SYSTEM_UPGRADE ||
               m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL)
      {
        //buildPackageList(false);
        metaBuildPackageList();
      }
      else if (m_commandExecuting != ectn_MIRROR_CHECK)
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
        m_unixCommand->removeTemporaryFile();
        doSystemUpgrade();
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
      m_unixCommand->runCommandInTerminal(m_lastCommandList);         
      return;
    }
  }

  enableTransactionActions();
  if (isAURGroupSelected())
  {
    toggleSystemActions(false);
  }

  if (m_commandExecuting != ectn_MIRROR_CHECK && bRefreshGroups)
    refreshGroupsWidget();

  refreshMenuTools(); //Maybe some of octopi tools were added/removed...

  m_unixCommand->removeTemporaryFile();

  delete m_unixCommand; //EXPERIMENTAL

  m_commandExecuting = ectn_NONE;
}

/*
 * This SLOT is called whenever Mirror-check process has something to output to Standard ERROR out
 */
void MainWindow::actionsProcessReadOutputErrorMirrorCheck()
{
  QString msg = m_unixCommand->readAllStandardError();

  msg.remove("[01;33m");
  msg.remove("\033[01;37m");
  msg.remove("\033[00m");
  msg.remove("\033[00;32m");
  msg.remove("[00;31m");
  msg.remove("\n");

  if (msg.contains("Checking"), Qt::CaseInsensitive)
    msg += "<br>";

  writeToTabOutputExt(msg, ectn_DONT_TREAT_URL_LINK);
}

/*
 * This SLOT is called whenever Mirror-check process has something to output to Standard out
 */
void MainWindow::actionsProcessReadOutputMirrorCheck()
{
  QString msg = m_unixCommand->readAllStandardOutput();

  msg.remove("[01;33m");
  msg.remove("\033[01;37m");
  msg.remove("\033[00m");
  msg.remove("\033[00;32m");
  msg.remove("[00;31m");
  msg.replace("[", "'");
  msg.replace("]", "'");
  msg.remove("\n");

  if (msg.contains("Checking", Qt::CaseInsensitive))
  {
    msg += "<br>";
  }

  writeToTabOutputExt(msg, ectn_DONT_TREAT_URL_LINK);
}

/*
 * This SLOT is called whenever Pacman's process has something to output to Standard out
 */
void MainWindow::actionsProcessReadOutput()
{
  if (WMHelper::getSUCommand().contains("kdesu"))
  {
    QString msg = m_unixCommand->readAllStandardOutput();
    splitOutputStrings(msg);
  }
  else if (WMHelper::getSUCommand().contains("gksu"))
  {
    QString msg = m_unixCommand->readAllStandardOutput();
    msg = msg.trimmed();

    if(!msg.isEmpty() &&
       msg.indexOf(":: Synchronizing package databases...") == -1 &&
       msg.indexOf(":: Starting full system upgrade...") == -1)
    {
      writeToTabOutputExt(msg);
    }
  }
}

/*
 * Searches the given msg for a series of verbs that a Pacman transaction may produce
 */
bool MainWindow::searchForKeyVerbs(const QString &msg)
{
  return (msg.contains(QRegExp("checking ")) ||
          msg.contains(QRegExp("loading ")) ||
          msg.contains(QRegExp("installing ")) ||
          msg.contains(QRegExp("upgrading ")) ||
          msg.contains(QRegExp("downgrading ")) ||
          msg.contains(QRegExp("resolving ")) ||
          msg.contains(QRegExp("looking ")) ||
          msg.contains(QRegExp("removing ")));
}

/*
 * Processes the output of the 'pacman process' so we can update percentages and messages at real time
 */
void MainWindow::parsePacmanProcessOutput(const QString &pMsg)
{  
  if (m_commandExecuting == ectn_RUN_IN_TERMINAL ||
      m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL) return;

  bool continueTesting = false;
  QString perc;
  QString msg = pMsg;

  QString progressRun;
  QString progressEnd;
  msg.remove(QRegExp(".+\\[Y/n\\].+"));

  //Let's remove color codes from strings...
  msg.remove("\033[0;1m");
  msg.remove("\033[0m");
  msg.remove("[1;33m");
  msg.remove("[00;31m");
  msg.remove("\033[1;34m");
  msg.remove("\033[0;1m");

  msg.remove("c");
  msg.remove("C");
  msg.remove("");
  msg.remove("[m[0;37m");
  msg.remove("o");
  msg.remove("[m");
  msg.remove(";37m");
  msg.remove("[c");
  msg.remove("[mo");

  if (msg.contains("exists in filesystem")) return;

  //std::cout << "_treat: " << msg.toLatin1().data() << std::endl;

  if (m_iLoveCandy)
  {
    progressRun = "m]";
    progressEnd = "100%";
  }
  else
  {
    progressRun = "-]";
    progressEnd = "#]";
  }

  //If it is a percentage, we are talking about curl output...
  if(msg.indexOf(progressEnd) != -1)
  {
    perc = "100%";
    if (!m_progressWidget->isVisible()) m_progressWidget->show();
    m_progressWidget->setValue(100);
    continueTesting = true;
  }

  if (msg.indexOf(progressRun) != -1 || continueTesting)
  {
    if (!continueTesting){
      perc = msg.right(4).trimmed();
      //std::cout << "percentage is: " << perc.toLatin1().data() << std::endl;
    }

    continueTesting = false;

    int aux = msg.indexOf("[");
    if (aux > 0 && !msg.at(aux-1).isSpace()) return;

    QString target;
    if (m_commandExecuting == ectn_INSTALL ||
        m_commandExecuting == ectn_SYSTEM_UPGRADE ||
        m_commandExecuting == ectn_SYNC_DATABASE ||
        m_commandExecuting == ectn_REMOVE ||
        m_commandExecuting == ectn_REMOVE_INSTALL)
    {
      int ini = msg.indexOf(QRegExp("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "));
      if (ini == 0)
      {
        int rp = msg.indexOf(")");
        msg = msg.remove(0, rp+2);

        if (searchForKeyVerbs(msg))
        {
          int end = msg.indexOf("[");
          msg = msg.remove(end, msg.size()-end).trimmed() + " ";
          writeToTabOutputExt(msg);
        }
        else
        {
          //std::cout << "test1: " << target.toLatin1().data() << std::endl;
          int pos = msg.indexOf(" ");
          if (pos >=0)
          {
            target = msg.left(pos);
            target = target.trimmed() + " ";
            //std::cout << "target: " << target.toLatin1().data() << std::endl;

            if(!target.isEmpty())
            {
              writeToTabOutputExt("<b><font color=\"#b4ab58\">" + target + "</font></b>"); //#C9BE62
            }
          }
          else
          {
            writeToTabOutputExt("<b><font color=\"#b4ab58\">" + msg + "</font></b>"); //#C9BE62
          }
        }
      }
      else if (ini == -1)
      {
        if (searchForKeyVerbs(msg))
        {
          //std::cout << "test2: " << msg.toLatin1().data() << std::endl;

          int end = msg.indexOf("[");
          msg = msg.remove(end, msg.size()-end);
          msg = msg.trimmed() + " ";

          writeToTabOutputExt(msg);
        }
        else
        {
          int pos = msg.indexOf(" ");
          if (pos >=0)
          {
            target = msg.left(pos);
            target = target.trimmed() + " ";
            //std::cout << "target: " << target.toLatin1().data() << std::endl;

            if(!target.isEmpty() && !textInTabOutput(target))
            {
              if (target.indexOf(QRegExp("[a-z]+")) != -1)
              {
                if(m_commandExecuting == ectn_SYNC_DATABASE && !target.contains("/"))
                {
                  writeToTabOutputExt("<b><font color=\"#FF8040\">" +
                                      StrConstants::getSyncing() + " " + target + "</font></b>");
                }
                else if (m_commandExecuting != ectn_SYNC_DATABASE)
                {
                  writeToTabOutputExt("<b><font color=\"#b4ab58\">" +
                                      target + "</font></b>"); //#C9BE62
                }
              }
            }
          }
          else
          {
            writeToTabOutputExt("<b><font color=\"blue\">" + msg + "</font></b>");
          }
        }
      }
    }

    //Here we print the transaction percentage updating
    if(!perc.isEmpty() && perc.indexOf("%") > 0)
    {
      int percentage = perc.left(perc.size()-1).toInt();
      if (!m_progressWidget->isVisible()) m_progressWidget->show();
      m_progressWidget->setValue(percentage);
    }
  }
  //It's another error, so we have to output it
  else
  {      
    //Let's supress some annoying string bugs...
    msg.remove(QRegExp("\\(process.+"));
    msg.remove(QRegExp("Using the fallback.+"));
    msg.remove(QRegExp("Gkr-Message:.+"));
    msg.remove(QRegExp("kdesu.+"));
    msg.remove(QRegExp("kbuildsycoca.+"));
    msg.remove(QRegExp("Connecting to deprecated signal.+"));
    msg.remove(QRegExp("QVariant.+"));
    msg.remove(QRegExp("gksu-run.+"));
    msg.remove(QRegExp("GConf Error:.+"));
    msg.remove(QRegExp(":: Do you want.+"));
    msg.remove(QRegExp("org\\.kde\\."));
    msg.remove(QRegExp("QCommandLineParser"));
    msg.remove(QRegExp("QCoreApplication.+"));
    msg.remove(QRegExp("Fontconfig warning.+"));
    msg.remove(QRegExp("reading configurations from.+"));

    msg.remove(QRegExp(".+annot load library.+"));
    msg = msg.trimmed();

    //std::cout << "debug: " << msg.toLatin1().data() << std::endl;

    QString order;
    int ini = msg.indexOf(QRegExp("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "));
    if (ini == 0)
    {
      int rp = msg.indexOf(")");
      order = msg.left(rp+2);
      msg = msg.remove(0, rp+2);
    }

    if (!msg.isEmpty())
    {
      if (msg.contains(QRegExp("removing ")) && !textInTabOutput(msg + " "))
      {
        //Does this package exist or is it a proccessOutput buggy string???
        QString pkgName = msg.mid(9).trimmed();

        const PackageRepository::PackageData*const package = m_packageRepo.getFirstPackageByName(pkgName);
        if (pkgName.indexOf("...") != -1 || //TODO: maybe && was meant ?, what does pacman actually do here ?
            (package != NULL && package->installed()))
        {
          writeToTabOutputExt("<b><font color=\"#E55451\">" + msg + "</font></b>"); //RED
        }
      }
      else
      {
        QString altMsg = msg;

        if (msg.indexOf(":: Synchronizing package databases...") == -1 &&
            msg.indexOf(":: Starting full system upgrade...") == -1)
        {
          //std::cout << "Entered here: " << msg.toLatin1().data() << std::endl;

          if (m_commandExecuting == ectn_SYNC_DATABASE &&
              msg.indexOf("is up to date"))
          {
            if (!m_progressWidget->isVisible()) m_progressWidget->show();
            m_progressWidget->setValue(100);

            int blank = msg.indexOf(" ");
            QString repo = msg.left(blank);

            if (repo.contains("error", Qt::CaseInsensitive) ||
                repo.contains("gconf", Qt::CaseInsensitive) ||
                repo.contains("failed", Qt::CaseInsensitive) ||
                repo.contains("fontconfig", Qt::CaseInsensitive) ||
                repo.contains("reading", Qt::CaseInsensitive)) return;

            altMsg = repo + " " + StrConstants::getIsUpToDate();
          }

          writeToTabOutputExt(altMsg); //BLACK
        }
      }
    }
  }

  if(m_commandExecuting == ectn_NONE)
    ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName());
}

/*
 * Breaks the output generated by QProcess so we can parse the strings
 * and give a better feedback to our users (including showing percentages)
 *
 * Returns true if the given output was split
 */
bool MainWindow::splitOutputStrings(const QString &output)
{
  bool res = true;
  QString msg = output.trimmed();
  QStringList msgs = msg.split(QRegExp("\\n"), QString::SkipEmptyParts);

  foreach (QString m, msgs)
  {
    QStringList m2 = m.split(QRegExp("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "), QString::SkipEmptyParts);

    if (m2.count() == 1)
    {
      //Let's try another test... if it doesn't work, we give up.
      QStringList maux = m.split(QRegExp("%"), QString::SkipEmptyParts);
      if (maux.count() > 1)
      {
        foreach (QString aux, maux)
        {
          aux = aux.trimmed();
          if (!aux.isEmpty())
          {
            if (aux.at(aux.count()-1).isDigit())
            {
              aux += "%";
            }

            //std::cout << "Error1: " << aux.toLatin1().data() << std::endl;
            parsePacmanProcessOutput(aux);
          }
        }
      }
      else if (maux.count() == 1)
      {
        if (!m.isEmpty())
        {
          //std::cout << "Error2: " << m.toLatin1().data() << std::endl;
          parsePacmanProcessOutput(m);
        }
      }
    }
    else if (m2.count() > 1)
    {
      foreach (QString m3, m2)
      {
        if (!m3.isEmpty())
        {
          //std::cout << "Error3: " << m3.toLatin1().data() << std::endl;
          parsePacmanProcessOutput(m3);
        }
      }
    }
    else res = false;
  }

  return res;
}

/*
 * This SLOT is called whenever Pacman's process has something to output to Standard error
 */
void MainWindow::actionsProcessRaisedError()
{
  QString msg = m_unixCommand->readAllStandardError();
  splitOutputStrings(msg);
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
    positionTextEditCursorAtEnd();

    if(treatURLLinks == ectn_TREAT_URL_LINK)
    {
      text->insertHtml(Package::makeURLClickable(msg));
    }
    else
    {
      text->insertHtml(msg);
    }

    text->ensureCursorVisible();
  }
}

/*
 * A helper method which writes the given string to OutputTab's textbrowser
 * This is the EXTENDED version, it checks lots of things before writing msg
 */
void MainWindow::writeToTabOutputExt(const QString &msg, TreatURLLinks treatURLLinks)
{
  //std::cout << "To print: " << msg.toLatin1().data() << std::endl;
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textBrowser");
  if (text)
  {    
    //If the msg waiting to being print is from curl status OR any other unwanted string...
    if ((msg.contains(QRegExp("\\(\\d")) &&
         (!msg.contains("target", Qt::CaseInsensitive)) &&
         (!msg.contains("package", Qt::CaseInsensitive))) ||
       (msg.contains(QRegExp("\\d\\)")) &&
        (!msg.contains("target", Qt::CaseInsensitive)) &&
        (!msg.contains("package", Qt::CaseInsensitive))) ||

        msg.indexOf("Enter a selection", Qt::CaseInsensitive) == 0 ||
        msg.indexOf("Proceed with", Qt::CaseInsensitive) == 0 ||
        msg.indexOf("%") != -1 ||
        msg.indexOf("[") != -1 ||
        msg.indexOf("]") != -1 ||
        msg.indexOf("---") != -1)
    {
      return;
    }

    //If the msg waiting to being print has not yet been printed...
    if(textInTabOutput(msg))
    {
      return;
    }

    QString newMsg = msg;
    ensureTabVisible(ctn_TABINDEX_OUTPUT);
    positionTextEditCursorAtEnd();

    if(newMsg.contains(QRegExp("<font color")))
    {
      newMsg += "<br>";
    }
    else
    {
      if(newMsg.contains("removing ") ||
         newMsg.contains("could not ") ||
         newMsg.contains("error:", Qt::CaseInsensitive) ||
         newMsg.contains("failed") ||
         newMsg.contains("is not synced") ||
         newMsg.contains("could not be found"))
      {
        newMsg = "<b><font color=\"#E55451\">" + newMsg + "&nbsp;</font></b>"; //RED
      }
      else if(newMsg.contains("checking ") ||
              newMsg.contains("-- reinstalling") ||
              newMsg.contains("installing ") ||
              newMsg.contains("upgrading ") ||
              newMsg.contains("loading ") ||
              newMsg.contains("resolving ") ||
              newMsg.contains("looking "))
      {
         newMsg = "<b><font color=\"#4BC413\">" + newMsg + "</font></b>"; //GREEN
      }
      else if (newMsg.contains("warning") || (newMsg.contains("downgrading")))
      {
        newMsg = "<b><font color=\"#FF8040\">" + newMsg + "</font></b>"; //ORANGE
      }
      else if (!newMsg.contains("::"))
      {
        newMsg += "<br>";
      }
    }

    if (newMsg.contains("::"))
    {
      newMsg = "<br><B>" + newMsg + "</B><br><br>";
    }

    if (!newMsg.contains(QRegExp("<br"))) //It was an else!
    {
      newMsg += "<br>";
    }

    if (treatURLLinks == ectn_TREAT_URL_LINK)
      text->insertHtml(Package::makeURLClickable(newMsg));
    else
      text->insertHtml(newMsg);

    text->ensureCursorVisible();
  }
}
