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
#include "strconstants.h"
#include "transactiondialog.h"
#include "multiselectiondialog.h"
#include "searchlineedit.h"
#include "globals.h"
#include <iostream>
#include <cassert>
#include "termwidget.h"
#include "aurvote.h"
#include "constants.h"
#include "utils.h"

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

  if (!isAURGroupSelected()) ui->actionCheckUpdates->setEnabled(!state);

  if(m_hasMirrorCheck && !isAURGroupSelected())
  {
    m_actionMenuMirrorCheck->setEnabled(!state);
  }

  if(m_hasForeignTool) m_actionSwitchToForeignTool->setEnabled(!state);

  if (!state && m_outdatedStringList->count() > 0)
    ui->actionSystemUpgrade->setEnabled(true);
  else if (state)
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
  QStandardItemModel *sim = ui->twProperties->getModelTransaction();
  QStandardItem *si = nullptr;

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
  QStandardItemModel *sim = ui->twProperties->getModelTransaction();
  QStandardItem *si = nullptr;

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
  QTreeView *tvTransaction = ui->twProperties->getTvTransaction();

  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItem * siPackageToRemove = new QStandardItem(IconHelper::getIconRemoveItem(), pkgName);
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siRemoveParent->model());
  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  int slash = pkgName.indexOf(QLatin1String("/"));
  QString pkg = pkgName.mid(slash+1);
  siPackageToRemove->setText(pkg);

  if (foundItems.empty())
  {
    slash = pkgName.indexOf(QLatin1String("/"));
    pkg = pkgName.mid(slash+1);
    QList<QStandardItem *> aux = sim->findItems(pkg, Qt::MatchRecursive | Qt::MatchExactly);

    if (aux.count() == 0) siRemoveParent->appendRow(siPackageToRemove);
  }
  else if (foundItems.size() == 1 && foundItems.at(0)->parent() == siInstallParent)
  {
    siInstallParent->removeRow(foundItems.at(0)->row());
    siRemoveParent->appendRow(siPackageToRemove);
  }

  tvTransaction->expandAll();
  changeTransactionActionsState();
}

/*
 * Returns true if there are any package in the Install action tree view
 */
bool MainWindow::hasInstallActions()
{
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  return (siInstallParent->rowCount());
}

/*
 * Inserts the given package into the Install parent item of the Transaction treeview
 * If flag "isDep" is true, the package is marked for installation as dependency with "pacman -S --asdeps"
 */
void MainWindow::insertInstallPackageIntoTransaction(const QString &pkgName, bool isDep)
{
  QTreeView *tvTransaction = ui->twProperties->getTvTransaction();
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItem * siPackageToInstall = new QStandardItem(IconHelper::getIconInstallItem(), pkgName);
  if (isDep) siPackageToInstall->setStatusTip(QStringLiteral("isDep"));
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siInstallParent->model());
  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  if (foundItems.empty())
  {
    int slash = pkgName.indexOf(QLatin1String("/"));
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
    res += siRemoval->child(c)->text() + QLatin1Char(' ');
  }

  res = res.trimmed();
  return res;
}

/*
 * Retrieves the list of all packages scheduled to be installed
 * The returned QHash contains the name of the package and if it needs to be installed "--asdeps"
 */
QHash<QString, bool> MainWindow::getTobeInstalledPackages()
{
  QStandardItem * siInstall = getInstallTransactionParentItem();
  QHash<QString, bool> res;
  QString aux;

  for(int c=0; c < siInstall->rowCount(); c++)
  {
    aux=siInstall->child(c)->text();
    aux=aux.trimmed();
    res.insert(aux, siInstall->child(c)->statusTip()==QStringLiteral("isDep"));
  }

  //res = res.trimmed();
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

  bool checkDependencies=false;
  //ensureTabVisible(ctn_TABINDEX_ACTIONS);
  QModelIndexList selectedRows;

  if (indexToInclude == nullptr)
    selectedRows = ui->tvPackages->selectionModel()->selectedRows();
  else
  {
    selectedRows.append(*indexToInclude);
  }

  //First, let's see if we are dealing with a package group
  if(!isAURGroupSelected() && !isAllGroupsSelected())
  {
    //If we are trying to remove all the group's packages, why not remove the entire group?
    if(selectedRows.count() == m_packageModel->getPackageCount())
    {
      insertRemovePackageIntoTransaction(getSelectedGroup());
      return;
    }
  }

  QString removeCmd = m_removeCommand;
  if (removeCmd == QLatin1String("Rcs") )
  {
    checkDependencies = true;
  }

  int res = 0;
  for(QModelIndex item: qAsConst(selectedRows))
  {
    const PackageRepository::PackageData*const package = m_packageModel->getData(item);
    if (package == nullptr) {
      assert(false);
      continue;
    }

    if(checkDependencies)
    {
      QStringList *targets = Package::getTargetRemovalList(package->name, removeCmd);

      for(const QString& target: qAsConst(*targets))
      {
        int separator = target.lastIndexOf(QLatin1String("-"));
        QString candidate = target.left(separator);
        separator = candidate.lastIndexOf(QLatin1String("-"));
        candidate = candidate.left(separator);

        if (candidate != package->name)
        {
          dependencies.append(candidate);
        }
      }

      if (dependencies.count() > 0)
      {
        if (!dependencies.at(0).contains(QLatin1String("HoldPkg was found in")))
        {
          res = insertIntoRemovePackageDeps(dependencies);
          if (res == -1)
          {
            return;
          }
        }
      }
    }

    insertRemovePackageIntoTransaction(package->repository + QLatin1Char('/') + package->name);
    if (res == 0) checkDependencies = false;
  }// end of FOR
}

/*
 * Inserts the current selected group for removal into the Transaction Treeview
 */
void MainWindow::insertGroupIntoRemovePackage()
{
  //ensureTabVisible(ctn_TABINDEX_ACTIONS);
  insertRemovePackageIntoTransaction(getSelectedGroup());
}

/*
 * Whenever user selects a bunch of packages from the main package list to change their "Install Reason"
 */
void MainWindow::onChangeInstallReason()
{
  QHash<QString, QString> pkgList;
  QModelIndexList selectedRows = ui->tvPackages->selectionModel()->selectedRows();

  for(QModelIndex item: selectedRows)
  {
    const PackageRepository::PackageData*const package = m_packageModel->getData(item);
    if (package == nullptr) {
      assert(false);
      continue;
    }

    if (SettingsManager::hasPacmanBackend())
      pkgList.insert(package->name, Package::getInstallReasonByPkgName(package->name));
    else
      pkgList.insert(package->name, package->installReason);
  }

  doChangeInstallReason(pkgList);
}

/*
 * Changes "Install Reason" field for every package in listOfTargets (uses "pacman -D")
 */
void MainWindow::doChangeInstallReason(const QHash<QString, QString> &listOfTargets)
{
  disableTransactionActions();
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  m_pacmanExec->setSharedMemory(m_sharedMemory);
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished (int, QProcess::ExitStatus)),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  m_commandExecuting = ectn_CHANGE_INSTALL_REASON;
  m_pacmanExec->doChangeInstallReason(listOfTargets);
}

/*
 * Inserts the current selected packages for installation into the Transaction Treeview
 * This is the SLOT, it needs to call insertInstallPackageIntoTransaction(PackageName) to work!
 */
void MainWindow::insertIntoInstallPackage(QModelIndex *indexToInclude)
{
  qApp->processEvents();

  //if (!isAURGroupSelected())
  {
    //ensureTabVisible(ctn_TABINDEX_ACTIONS);
    QModelIndexList selectedRows;

    if (indexToInclude == nullptr)
      selectedRows = ui->tvPackages->selectionModel()->selectedRows();
    else
    {
      selectedRows.append(*indexToInclude);
    }

    //First, let's see if we are dealing with a package group    
    if(!isAURGroupSelected() && !isAllGroupsSelected())
    {
      //If we are trying to insert all the group's packages, why not insert the entire group?
      if(selectedRows.count() == m_packageModel->getPackageCount())
      {
        insertInstallPackageIntoTransaction(getSelectedGroup());
        return;
      }
    }

    for(QModelIndex item: qAsConst(selectedRows))
    {
      const PackageRepository::PackageData*const package = m_packageModel->getData(item);
      if (package == nullptr) {
        assert(false);
        continue;
      }

      insertIntoInstallPackageOptDeps(package->name); //Do we have any deps???
      insertInstallPackageIntoTransaction(package->repository + QLatin1Char('/') + package->name);
    }
  }
  /*else
  {
    doInstallAURPackage();
  }*/
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
  QList<QStandardItem *> foundItems = sim->findItems(repo + QLatin1Char('/') + pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  return (!foundItems.empty());
}

/*
 * Searches in Remove Transaction queue for the given package
 */
bool MainWindow::isPackageInRemoveTransaction(const QString &pkgName)
{
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siRemoveParent->model());
  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  return (!foundItems.empty());
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

  for(const QString& optDep: optDeps)
  {
    QString candidate = optDep;
    int points = candidate.indexOf(QLatin1String(":"));
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
    msd->setWindowTitle(packageName + QLatin1String(": ") + StrConstants::getOptionalDeps());
    msd->setWindowIcon(windowIcon());
    QStringList selectedPackages;

    for(const PackageRepository::PackageData* candidate: optionalPackages)
    {
      QString desc = candidate->description;
      int space = desc.indexOf(QLatin1String(" "));
      desc = desc.mid(space+1);

      msd->addPackageItem(candidate->name, candidate->description, candidate->repository);
    }

    delete cic;
    if (msd->exec() == QMessageBox::Ok)
    {
      selectedPackages = msd->getSelectedPackages();

      for(const QString& pkg: qAsConst(selectedPackages))
      {
        insertInstallPackageIntoTransaction(pkg, true);
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
 * Return <NUMBER of dependencies selected> if the user click OK/ENTER
 * Return -1 if the user click CANCEL
 */
int MainWindow::insertIntoRemovePackageDeps(const QStringList &dependencies)
{
  QList<const PackageRepository::PackageData*> newDeps;
  for(const QString& dep: dependencies)
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

    for(const PackageRepository::PackageData* dep: newDeps)
    {
      QString desc = dep->description;
      int space = desc.indexOf(QLatin1String(" "));
      desc = desc.mid(space+1);
      msd->addPackageItem(dep->name, desc, dep->repository);
    }

    msd->setAllSelected();
    delete cic;
    int res = msd->exec();

    if (res == QMessageBox::Ok)
    {
      selectedPackages = msd->getSelectedPackages();
      for(const QString& pkg: qAsConst(selectedPackages))
      {
        insertRemovePackageIntoTransaction(pkg);
      }
    }

    delete msd;

    if (res == QMessageBox::Ok)
    {
      return selectedPackages.count();
    }
    else return -1;
  }
  else return 0; //true
}

/*
 * Inserts the current selected group for removal into the Transaction Treeview
 */
void MainWindow::insertGroupIntoInstallPackage()
{
  //ensureTabVisible(ctn_TABINDEX_ACTIONS);
  insertInstallPackageIntoTransaction(getSelectedGroup());
}

/*
 * Adjust the count and selection count status of the selected tvTransaction item (Remove or Insert parents)
 */
void MainWindow::tvTransactionAdjustItemText(QStandardItem *item)
{
  int countSelected=0;
  QTreeView *tvTransaction = ui->twProperties->getTvTransaction();

  if (!tvTransaction) return;

  for(int c=0; c < item->rowCount(); c++)
  {
    if(tvTransaction->selectionModel()->isSelected(item->child(c)->index()))
    {
      countSelected++;
    }
  }

  QString itemText = item->text();
  int slash = itemText.indexOf(QLatin1String("/"));
  int pos = itemText.indexOf(QLatin1String(")"));

  if (slash > 0){
    itemText.remove(slash, pos-slash);
  }

  pos = itemText.indexOf(QLatin1String(")"));
  itemText.insert(pos, QLatin1Char('/') + QString::number(countSelected));
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
  //QStandardItem *item = m_modelTransaction->itemFromIndex(parent);
  QStandardItem *item = ui->twProperties->getModelTransaction()->itemFromIndex(parent);
  QString count = QString::number(item->rowCount());
  QStandardItem * itemRemove = getRemoveTransactionParentItem();
  QStandardItem * itemInstall = getInstallTransactionParentItem();

  if (item == itemRemove)
  {
    if (item->rowCount() > 0)
    {
      itemRemove->setText(StrConstants::getTransactionRemoveText() + QLatin1String(" (") + count + QLatin1Char(')'));
      tvTransactionAdjustItemText(itemRemove);
    }
    else itemRemove->setText(StrConstants::getTransactionRemoveText());
  }
  else if (item == itemInstall)
  {
    if (item->rowCount() > 0)
    {
      itemInstall->setText(StrConstants::getTransactionInstallText() + QLatin1String(" (") + count + QLatin1Char(')'));
      tvTransactionAdjustItemText(itemInstall);
    }
    else itemInstall->setText(StrConstants::getTransactionInstallText());
  }

  int lToInstall=itemInstall->rowCount();
  int lToRemove=itemRemove->rowCount();

  if (lToInstall > 0 || lToRemove > 0)
  {
    QString newText=StrConstants::getActions() + QLatin1String(" (");

    if (lToInstall > 0)
      newText += QLatin1String("+") + QString::number(lToInstall);
    if (lToRemove > 0)
      newText += QLatin1String("-") + QString::number(lToRemove);

    newText += QLatin1String(")");
    ui->twProperties->setTabText(ctn_TABINDEX_ACTIONS, newText);
  }
  else
  {
    ui->twProperties->setTabText(ctn_TABINDEX_ACTIONS, StrConstants::getActions());
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
    QTreeView *tvTransaction = ui->twProperties->getTvTransaction();

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
          if (ui->twProperties->getModelTransaction()->itemFromIndex(mi)->parent() != nullptr)
          {
            ui->twProperties->getModelTransaction()->removeRow(mi.row(), mi.parent());
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
                           QLatin1Char('\n') + StrConstants::getYoullNeedSuFrontend());
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
    UnixCommand::execCommandAsNormalUser(QStringLiteral("kcp"), QStringList() << QStringLiteral("-u"));

  m_commandExecuting = ectn_CHECK_UPDATES;
  disableTransactionActions();

  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();
  ensureTabVisible(ctn_TABINDEX_OUTPUT);

  m_pacmanExec = new PacmanExec();
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished (int, QProcess::ExitStatus)),
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

  QObject::connect(m_pacmanExec, SIGNAL( finished (int, QProcess::ExitStatus)),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));

  m_pacmanExec->doMirrorCheck();
}

/*
 * Updates ALL the outdated AUR packages with "yaourt -S <list>"
 */
void MainWindow::doAURUpgrade()
{
  QString listOfTargets;
  QString auxPkg;

  for(const QString& pkg: qAsConst(*m_outdatedAURStringList))
  {
    auxPkg = pkg;
    auxPkg.remove(QStringLiteral("[1;39m"));
    auxPkg.remove(QStringLiteral("[0m"));
    auxPkg.remove(QStringLiteral(""));
    listOfTargets += auxPkg + QLatin1Char(' ');
  }

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  m_pacmanExec->setSharedMemory(m_sharedMemory);
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished (int, QProcess::ExitStatus)),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_pacmanExec->doAURUpgrade(listOfTargets);
}

/*
 * Updates the selected AUR package with "yaourt -S pkg"
 */
void MainWindow::doUpdateAURPackage()
{
  const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();

  if (selectionModel->selectedRows().count() <= 0) return;

  QString listOfTargets;
  QModelIndexList selectedRows = selectionModel->selectedRows();
  for (QModelIndex item: selectedRows)
  {
    const PackageRepository::PackageData*const selectedPackage = m_packageModel->getData(item);
    listOfTargets = listOfTargets + QStringLiteral(" ") + selectedPackage->name ;
  }

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  m_pacmanExec->setSharedMemory(m_sharedMemory);
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL( finished (int, QProcess::ExitStatus)),
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
  m_pacmanExec->setSharedMemory(m_sharedMemory);
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL(finished (int, QProcess::ExitStatus)),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus)));

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
  double totalDownloadSize = 0;

  if (isNotifierBusy()) return;

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

    if( (m_checkupdatesStringList->count() != 0 && m_checkupdatesStringList->contains(QStringLiteral("pacman"))) ||
        (m_outdatedStringList->count() != 0 && m_outdatedStringList->contains(QStringLiteral("pacman"))) )
    {
      m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
      m_pacmanExec->doSystemUpgradeInTerminal();
      m_commandQueued = ectn_NONE;
    }
    else
    {
      m_commandExecuting = ectn_SYSTEM_UPGRADE;
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
      writeToTabOutput(QLatin1String("<b>") + StrConstants::getNoNewUpdatesAvailable() + QLatin1String("</b><br>"));
      return;
    }
    else if (targets->count() < m_outdatedStringList->count())
    {
      //This is a bug and should be shown to the user!
      //This is bug, let us find if "breaks dependency" string is here:
      if (UnixCommand::getTargetUpgradeList().contains("breaks dependency") ||
          UnixCommand::getTargetUpgradeList().contains(": requires "))
      {
        QString msg = StrConstants::getThereHasBeenATransactionError() + QLatin1Char('\n') +
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
          m_pacmanExec->doSystemUpgradeInTerminal(ectn_SYNC_DATABASE);
          m_commandQueued = ectn_NONE;
        }

        return;
      }
      else
      {
        totalDownloadSize = UnixCommand::getCheckUpdatesSize();

        targets->clear();
        for(const QString& name: qAsConst(*m_checkupdatesStringList))
        {
          PackageListData aux;
          aux = PackageListData(name, m_checkUpdatesNameNewVersion->value(name), QStringLiteral("0")); //size);
          targets->append(aux);
        }
      }
    }

    QString list;

    if (totalDownloadSize == 0)
    {
      for(const PackageListData& target: qAsConst(*targets))
      {
        totalDownloadSize += target.downloadSize;
        list = list + target.name + QLatin1Char('-') + target.version + QLatin1Char('\n');
      }
    }
    else
    {
      for(const PackageListData& target: qAsConst(*targets))
      {
        list = list + target.name + QLatin1Char('-') + target.version + QLatin1Char('\n');
      }
    }

    list.remove(list.size()-1, 1);

    //Let's build the system upgrade transaction dialog...
    QString ds;
    if (totalDownloadSize >= 0)
      ds = Package::kbytesToSize(totalDownloadSize);

    TransactionDialog question(this);

    if(targets->count()==1)
      question.setText(StrConstants::getRetrievePackage() +
                       QLatin1String("\n\n") + StrConstants::getTotalDownloadSize().arg(ds).remove(QStringLiteral(" KB")));
    else
      question.setText(StrConstants::getRetrievePackages(targets->count()) +
                       QLatin1String("\n\n") + StrConstants::getTotalDownloadSize().arg(ds).remove(QStringLiteral(" KB")));

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
        m_pacmanExec->doSystemUpgrade();
        m_commandQueued = ectn_NONE;
      }
      else if (result == QDialogButtonBox::AcceptRole)
      {
        m_commandExecuting = ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL;
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
 * "pacman -R alltoRemove; AUR-TOOL -S alltoInstall"
 */
void MainWindow::doRemoveAndInstallAUR()
{
  QString listOfRemoveTargets = getTobeRemovedPackages();
  listOfRemoveTargets.replace(StrConstants::getForeignRepositoryGroupName() + QLatin1Char('/'), QLatin1String(""));
  QHash<QString, bool> listToBeInst = getTobeInstalledPackages();
  QString listOfInstallTargets = listToBeInst.keys().join(QStringLiteral(" "));
  listOfInstallTargets.replace(StrConstants::getForeignRepositoryGroupName() + QLatin1Char('/'), QLatin1String(""));

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  m_pacmanExec->setSharedMemory(m_sharedMemory);
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL(finished (int, QProcess::ExitStatus)),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_pacmanExec->doAURRemoveAndInstallInTerminal(listOfRemoveTargets, listOfInstallTargets);
}

/*
 * Removes and Installs all selected packages in one transaction just using:
 * "pacman -R alltoRemove; pacman -S alltoInstall"
 */
void MainWindow::doRemoveAndInstall()
{
  QString listOfRemoveTargets = getTobeRemovedPackages();
  QString listOfInstallTargets;
  QString listOfInstallDeps;
  QStringList *pRemoveTargets = Package::getTargetRemovalList(listOfRemoveTargets, m_removeCommand);
  QString removeList;
  QString allLists;
  QStringList pkgsToInstall;

  TransactionDialog question(this);
  QString dialogText;

  QStringList removeTargets = listOfRemoveTargets.split(QStringLiteral(" "), Qt::SkipEmptyParts);
  for(const QString& target: removeTargets)
  {
    removeList = removeList + StrConstants::getRemove() + QLatin1Char(' ')  + target + QLatin1Char('\n');
  }

  QHash<QString, bool> listToBeInst = getTobeInstalledPackages();
  listOfInstallTargets = listToBeInst.keys().join(QStringLiteral(" "));
  QList<PackageListData> *installTargets = Package::getTargetUpgradeList(listOfInstallTargets);
  QString installList;

  listOfInstallTargets.clear();
  QHash<QString, bool>::const_iterator i = listToBeInst.constBegin();
  while (i != listToBeInst.constEnd())
  {
    if (i.value())
    {
      listOfInstallDeps += i.key() + QLatin1Char(' ');
    }
    else
    {
      listOfInstallTargets += i.key() + QLatin1Char(' ');
    }

    ++i;
  }

  double totalDownloadSize = 0;
  for(const PackageListData& installTarget: qAsConst(*installTargets))
  {
    totalDownloadSize += installTarget.downloadSize;
    installList.append(StrConstants::getInstall() + QLatin1Char(' ') +
                       installTarget.name + QLatin1Char('-') + installTarget.version + QLatin1Char('\n'));
    pkgsToInstall.append(installTarget.name);
  }

  installList.remove(installList.size()-1, 1);

  if (hasPartialUpgrade(pkgsToInstall)) return;

  QString ds = Package::kbytesToSize(totalDownloadSize);

  if (installList.length() == 0)
  {
    installTargets->append(PackageListData(listOfInstallTargets, QLatin1String(""), QString::number(0)));
    installList.append(StrConstants::getInstall() + QLatin1Char(' ') + listOfInstallTargets);
  }

  allLists.append(removeList);
  allLists.append(installList);

  if(removeTargets.count()==1)
  {
    if (pRemoveTargets->at(0).indexOf(QLatin1String("HoldPkg was found in")) != -1)
    {
      QMessageBox::warning(
            this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
  }
  else if(installTargets->count()==1)
  {
    if (installTargets->at(0).name.indexOf(QLatin1String("HoldPkg was found in")) != -1)
    {
      QMessageBox::warning(
            this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
  }

  if (removeTargets.count() == 1)
  {
    dialogText = StrConstants::getRemovePackage() + QLatin1Char('\n');
  }
  else if (removeTargets.count() > 1)
  {
    dialogText = StrConstants::getRemovePackages(removeTargets.count()) + QLatin1Char('\n');
  }
  if (installTargets->count() == 1)
  {
    dialogText += StrConstants::getRetrievePackage() +
      QLatin1String("\n\n") + StrConstants::getTotalDownloadSize().arg(ds).remove(QStringLiteral(" KB"));
  }
  else if (installTargets->count() > 1)
  {
    dialogText += StrConstants::getRetrievePackages(installTargets->count()) +
      QLatin1String("\n\n") + StrConstants::getTotalDownloadSize().arg(ds).remove(QStringLiteral(" KB"));
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
    m_pacmanExec->setSharedMemory(m_sharedMemory);
    if (m_debugInfo) m_pacmanExec->setDebugMode(true);

    QObject::connect(m_pacmanExec, SIGNAL(finished(int, QProcess::ExitStatus)),
                     this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

    QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
    QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_REMOVE_INSTALL;
      m_pacmanExec->doRemoveAndInstall(listOfRemoveTargets, listOfInstallTargets, listOfInstallDeps);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_pacmanExec->doRemoveAndInstallInTerminal(listOfRemoveTargets, listOfInstallTargets, listOfInstallDeps);
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

  QStringList targets = listOfTargets.split(QStringLiteral(" "), Qt::SkipEmptyParts);
  for(const QString& target: targets)
  {
    list = list + target + QLatin1Char('\n');
  }

  TransactionDialog question(this);

  //Shows a dialog indicating the targets which will be removed and asks for the user's permission.
  if(targets.count()==1)
  {
    if (m_targets->at(0).indexOf(QLatin1String("HoldPkg was found in")) != -1)
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
    m_pacmanExec->setSharedMemory(m_sharedMemory);
    if (m_debugInfo) m_pacmanExec->setDebugMode(true);

    QObject::connect(m_pacmanExec, SIGNAL(finished (int, QProcess::ExitStatus)),
                     this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

    QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
    QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_REMOVE;
      m_pacmanExec->doRemove(listOfTargets);
    }

    if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_pacmanExec->doRemoveInTerminal(listOfTargets);
    }
  }
}

/*
 * Installs the selected package with "yaourt -S"
 */
void MainWindow::doInstallAURPackage()
{
  const QItemSelectionModel*const selectionModel = ui->tvPackages->selectionModel();
  if (selectionModel == nullptr || selectionModel->selectedRows().count() < 1 || !m_hasForeignTool) {
    std::cerr << "Octopi could not install selection using AUR tool" << std::endl;
    return;
  }
  QString listOfTargets;
  QModelIndexList selectedRows = selectionModel->selectedRows();
  for(QModelIndex item: selectedRows)
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
      listOfTargets += StrConstants::getForeignRepositoryTargetPrefix() + package->name + QLatin1Char(' ');
    else
      listOfTargets += package->name + QLatin1Char(' ');
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
  m_pacmanExec->setSharedMemory(m_sharedMemory);
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL(finished (int,QProcess::ExitStatus)),
                   this, SLOT( pacmanProcessFinished(int,QProcess::ExitStatus) ));

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
  writeToTabOutput(StrConstants::getDownloadingTempYay() + QLatin1String("<br>"));

  //Here we start the download thread... and bind its finished state to doInstallYayPackage()
  QFuture<bool> f;
  QEventLoop el;
  f = QtConcurrent::run(downloadTempYayHelper);
  connect(&g_fwDownloadTempYayHelper, SIGNAL(finished()), &el, SLOT(quit()));
  g_fwDownloadTempYayHelper.setFuture(f);
  el.exec();
  doInstallYayPackage();
}

/*
 * Installs yay-bin AUR package using temporary yay-bin
 */
void MainWindow::doInstallYayPackage()
{
  bool downloadedYay = g_fwDownloadTempYayHelper.result();
  if (!downloadedYay)
  {
    //We print a message saying the download did not go right
    writeToTabOutput(StrConstants::getErrorCouldNotDownloadTempYay() + QLatin1String("<br>"));
    enableTransactionActions();
  }

  writeToTabOutput(StrConstants::getTempYayDownloaded() + QLatin1String("<br>"));
  m_pacmanExec = new PacmanExec();
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL(finished(int, QProcess::ExitStatus)),
                   this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  writeToTabOutput(QLatin1String("<b>") + StrConstants::getInstallingPackages() + QLatin1String("</b><br>"));
  m_commandExecuting = ectn_INSTALL_YAY;
  m_pacmanExec->doInstallYayUsingTempYay();
}

/*
 * Removes selected packages with the proper AUR tool
 */
void MainWindow::doRemoveAUR()
{
  QString listOfTargets = getTobeRemovedPackages();
  //listOfTargets = listToBeInst.keys().join(QStringLiteral(" "));
  listOfTargets.replace(StrConstants::getForeignRepositoryGroupName() + QLatin1Char('/'), QStringLiteral(""));

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  m_pacmanExec->setSharedMemory(m_sharedMemory);
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL(finished(int,QProcess::ExitStatus)),
                   this, SLOT(pacmanProcessFinished(int,QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_pacmanExec->doAURRemove(listOfTargets);
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
  for(QModelIndex item: selectedRows)
  {
    const PackageRepository::PackageData*const package = m_packageModel->getData(item);
    if (package == nullptr) {
      assert(false);
      continue;
    }

    listOfTargets += package->name + QLatin1Char(' ');
  }

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  m_pacmanExec->setSharedMemory(m_sharedMemory);
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL(finished(int,QProcess::ExitStatus)),
                   this, SLOT( pacmanProcessFinished(int,QProcess::ExitStatus) ));

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
    if (m_actionSwitchToForeignTool->isChecked())
    {
      ui->actionUseInstantSearch->setEnabled(true);
      ui->twProperties->setTabEnabled(ctn_TABINDEX_ACTIONS, true);

      if (ui->actionUseInstantSearch->isChecked())
      {
        disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
        disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
        connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(reapplyPackageFilter()));
      }

      m_packageModel->applyFilter(QLatin1String(""));
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
      m_cachedPackageInInfo=QLatin1String("");

      //Let's clear the list of visited packages (pkg anchors in Info tab)
      m_listOfVisitedPackages.clear();
      m_indOfVisitedPackage = 0;

      switchToViewAllPackages();
      m_selectedRepository = QLatin1String("");
      m_actionRepositoryAll->setChecked(true);
      m_refreshPackageLists = false;

      if (!ui->actionUseInstantSearch->isChecked())
      {
        disconnect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
        connect(m_leFilterPackage, SIGNAL(textChanged(QString)), this, SLOT(lightPackageFilter()));
      }

      m_actionSwitchToForeignTool->setChecked(false);
      metaBuildPackageList();
      refreshInfoAndFileTabs();
    }

    m_hasForeignTool = false;
    //m_actionSwitchToAURTool->setVisible(false);
    m_refreshForeignPackageList = false;
    m_outdatedAURPackagesNameVersion->clear();
    m_outdatedAURStringList->clear();

    m_actionSwitchToForeignTool->setText(QLatin1String(""));
    m_actionSwitchToForeignTool->setToolTip(QStringLiteral("AUR"));
    m_actionSwitchToForeignTool->setCheckable(false);
    m_actionSwitchToForeignTool->setChecked(false);
  }
  else //We are using pacaur/yaourt tool
  {
    m_hasForeignTool = true;

    if (!isAURGroupSelected())
    {
      //m_actionSwitchToAURTool->setVisible(true);
      m_actionSwitchToForeignTool->setCheckable(true);
      m_actionSwitchToForeignTool->setChecked(false);
    }

    m_actionSwitchToForeignTool->setText(StrConstants::getUseForeignTool());
    m_actionSwitchToForeignTool->setToolTip(m_actionSwitchToForeignTool->text() + QLatin1String("  (Ctrl+Shift+Y)"));
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
    delete m_aurVote;
    m_aurVote = new AurVote(this);
    m_aurVote->setUserName(SettingsManager::getAURUserName());
    m_aurVote->setPassword(SettingsManager::getAURPassword());
    m_aurVote->login();
    refreshHelpUsageText();
  }
  else
  {
    delete m_aurVote;
    m_aurVote = nullptr;
    refreshHelpUsageText();
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
    for(const QString& n: pkgsToInstall)
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

    for(const QString& n: pkgsToInstall)
    {
      if (m_outdatedStringList->contains(n)) found++;
    }

    result = found > 0;
  }
  else if (m_numberOfOutdatedPackages == pkgsToInstall.count())
  {
    //We have to compare the lists...
    result = *m_outdatedStringList == pkgsToInstall;
  }

  if (result)
  {
    QMessageBox::warning(
          this, StrConstants::getAttention(), StrConstants::getPartialUpdatesNotSupported(), QMessageBox::Ok);
  }

  return result;
}

/*
 * Installs ALL AUR packages selected by the user
 */
void MainWindow::doInstallAUR()
{
  QString listOfTargets;
  QHash<QString, bool> listToBeInst = getTobeInstalledPackages();
  listOfTargets = listToBeInst.keys().join(QStringLiteral(" "));
  listOfTargets.replace(StrConstants::getForeignRepositoryGroupName() + QLatin1Char('/'), QStringLiteral(""));

  disableTransactionActions();
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  clearTabOutput();

  m_pacmanExec = new PacmanExec();
  m_pacmanExec->setSharedMemory(m_sharedMemory);
  if (m_debugInfo) m_pacmanExec->setDebugMode(true);

  QObject::connect(m_pacmanExec, SIGNAL(finished(int,QProcess::ExitStatus)),
                   this, SLOT( pacmanProcessFinished(int,QProcess::ExitStatus) ));

  QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
  QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
  QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

  m_commandExecuting = ectn_RUN_IN_TERMINAL;
  m_pacmanExec->doAURInstall(listOfTargets);

  //QMessageBox::warning(this, StrConstants::getApplicationName(), listOfTargets);
}

/*
 * Installs ALL the packages selected by the user with "pacman -S (INCLUDING DEPENDENCIES)" !
 */
void MainWindow::doInstall()
{
  QString listOfTargets;
  QString listOfDeps;
  QString list;
  QStringList pkgsToInstall;

  QHash<QString, bool> listToBeInst = getTobeInstalledPackages();
  listOfTargets = listToBeInst.keys().join(QStringLiteral(" "));
  QList<PackageListData> *targets = Package::getTargetUpgradeList(listOfTargets);

  listOfTargets.clear();
  QHash<QString, bool>::const_iterator i = listToBeInst.constBegin();
  while (i != listToBeInst.constEnd())
  {
    if (i.value())
    {
      listOfDeps += i.key() + QLatin1Char(' ');
    }
    else
    {
      listOfTargets += i.key() + QLatin1Char(' ');
    }

    ++i;
  }

  double totalDownloadSize = 0;
  for(const PackageListData& target: qAsConst(*targets))
  {
    totalDownloadSize += target.downloadSize;
    pkgsToInstall.append(target.name);
    list += target.name + QLatin1Char('-') + target.version + QLatin1Char('\n');
  }

  list.remove(list.size()-1, 1);

  if (hasPartialUpgrade(pkgsToInstall)) return;

  QString ds = Package::kbytesToSize(totalDownloadSize);

  if (list.length() == 0)
  {
    targets->append(PackageListData(listOfTargets, QLatin1String(""), QString::number(0)));
    list.append(listOfTargets);
  }

  TransactionDialog question(this);

  if(targets->count()==1)
  {
    if (targets->at(0).name.indexOf(QLatin1String("HoldPkg was found in")) != -1)
    {
      QMessageBox::warning(
            this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
    else question.setText(StrConstants::getRetrievePackage() +
                          QLatin1String("\n\n") + StrConstants::getTotalDownloadSize().arg(ds).remove(QStringLiteral(" KB")));
  }
  else
    question.setText(StrConstants::getRetrievePackages(targets->count()) +
                     QLatin1String("\n\n") + StrConstants::getTotalDownloadSize().arg(ds).remove(QStringLiteral(" KB")));

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
    m_pacmanExec->setSharedMemory(m_sharedMemory);
    if (m_debugInfo) m_pacmanExec->setDebugMode(true);

    QObject::connect(m_pacmanExec, SIGNAL(finished(int, QProcess::ExitStatus)),
                     this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

    QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
    QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_INSTALL;
      m_pacmanExec->doInstall(listOfTargets, listOfDeps);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
      m_pacmanExec->doInstallInTerminal(listOfTargets, listOfDeps);
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

  for(const QString& target: qAsConst(m_packagesToInstallList))
  {
    fi.setFile(target);
    list = list + fi.fileName() + QLatin1Char('\n');
  }

  for(const QString& pkgToInstall: qAsConst(m_packagesToInstallList))
  {
    listOfTargets += pkgToInstall + QLatin1String("; ");
  }

  TransactionDialog question(this);
  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);

  if(m_packagesToInstallList.count()==1)
  {
    if (m_packagesToInstallList.at(0).indexOf(QLatin1String("HoldPkg was found in")) != -1)
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

    QObject::connect(m_pacmanExec, SIGNAL(finished(int, QProcess::ExitStatus)),
                     this, SLOT( pacmanProcessFinished(int, QProcess::ExitStatus) ));

    QObject::connect(m_pacmanExec, SIGNAL(percentage(int)), this, SLOT(incrementPercentage(int)));
    QObject::connect(m_pacmanExec, SIGNAL(textToPrintExt(QString)), this, SLOT(outputText(QString)));
    QObject::connect(m_pacmanExec, SIGNAL(canStopTransaction(bool)), this, SLOT(onCanStopTransaction(bool)));
    QObject::connect(m_pacmanExec, SIGNAL(commandToExecInQTermWidget(QString)), this, SLOT(onExecCommandInTabTerminal(QString)));

    if (result == QDialogButtonBox::Yes)
    {
      m_commandExecuting = ectn_INSTALL;
      m_pacmanExec->doInstallLocal(listOfTargets);
    }
    else if (result == QDialogButtonBox::AcceptRole)
    {
      m_commandExecuting = ectn_RUN_IN_TERMINAL;
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

  if (value && pendingTransaction)
  {
    ui->actionApply->setEnabled(true);
    ui->actionCancel->setEnabled(true);

    if(m_hasMirrorCheck) m_actionMenuMirrorCheck->setEnabled(false);
    if(m_hasForeignTool) m_actionSwitchToForeignTool->setEnabled(false);

    ui->actionCheckUpdates->setEnabled(false);
    ui->actionSystemUpgrade->setEnabled(false);
  }
  else if (value && !pendingTransaction)
  {
    ui->actionApply->setEnabled(false);
    ui->actionCancel->setEnabled(false);

    if(m_hasMirrorCheck && !isAURGroupSelected()) m_actionMenuMirrorCheck->setEnabled(true);
    if(m_hasForeignTool && m_commandExecuting == ectn_NONE && m_initializationCompleted) m_actionSwitchToForeignTool->setEnabled(true);

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
  else if (!value)
  {
    ui->actionApply->setEnabled(false);
    ui->actionCancel->setEnabled(false);

    if(m_hasMirrorCheck) m_actionMenuMirrorCheck->setEnabled(false);
    if(m_hasForeignTool) m_actionSwitchToForeignTool->setEnabled(false);

    ui->actionCheckUpdates->setEnabled(false);
    ui->actionSystemUpgrade->setEnabled(false);
  }

  m_actionPackageInfo->setEnabled(value);
  ui->actionFindFileInPackage->setEnabled(value);
  ui->actionInstall->setEnabled(value);
  ui->actionInstallGroup->setEnabled(value);
  ui->actionInstallAUR->setEnabled(value);
  m_actionInstallPacmanUpdates->setEnabled(value);
  m_actionInstallAURUpdates->setEnabled(value);
  m_actionAUROpenPKGBUILD->setEnabled(value);
  m_actionAURShowPKGBUILDDiff->setEnabled(value);
  m_actionUpdateAURPackage->setEnabled(value);

  ui->actionRemoveTransactionItem->setEnabled(value);
  ui->actionRemoveTransactionItems->setEnabled(value);
  ui->actionRemove->setEnabled(value);

  ui->actionPacmanLogViewer->setEnabled(value);
  ui->actionCacheCleaner->setEnabled(value);
  ui->actionRepositoryEditor->setEnabled(value);
  m_actionSysInfo->setEnabled(value);

  if (value && m_initializationCompleted) m_actionSwitchToForeignTool->setEnabled(value);

  //ui->actionGetNews->setEnabled(value);

  if (!isAURGroupSelected())
  {
    ui->actionInstallLocalPackage->setEnabled(value);
    m_actionMenuOptions->setEnabled(value);
    ui->actionGetNews->setEnabled(value);
    m_actionChangeInstallReason->setEnabled(value);
  }
  else
  {
    ui->actionInstallLocalPackage->setEnabled(false);
    m_actionMenuOptions->setEnabled(false);
    ui->actionGetNews->setEnabled(false);
    ui->actionCheckUpdates->setEnabled(false);
    m_actionChangeInstallReason->setEnabled(false);
  }

  ui->actionHelpUsage->setEnabled(value);
  ui->actionDonate->setEnabled(value);
  ui->actionHelpAbout->setEnabled(value);

  //View menu
  ui->actionViewAllPackages->setEnabled(value);
  ui->actionViewInstalledPackages->setEnabled(value);
  ui->actionViewNonInstalledPackages->setEnabled(value);
  ui->actionViewOutdated->setEnabled(value);
  m_actionMenuRepository->setEnabled(value);

  //Search menu
  ui->actionSearchByFile->setEnabled(value);
  ui->actionSearchByName->setEnabled(value);
  ui->actionSearchByDescription->setEnabled(value);
  ui->actionUseInstantSearch->setEnabled(value);

  m_leFilterPackage->setEnabled(value);

  disconnect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));
  ui->twProperties->setTabEnabled(ctn_TABINDEX_INFORMATION, value);
  ui->twProperties->setTabEnabled(ctn_TABINDEX_FILES, value);
  connect(ui->twProperties, SIGNAL(currentChanged(int)), this, SLOT(changedTabIndex()));

  //We have to toggle the combobox groups as well
  if (m_initializationCompleted) ui->twGroups->setEnabled(value);
}

void MainWindow::toggleSystemActions(const bool value)
{
  if (value && m_commandExecuting != ectn_NONE) return;

  if(m_hasMirrorCheck)
  {
    if (!isAURGroupSelected())
      m_actionMenuMirrorCheck->setEnabled(value);
    else
    {
      if (!value) m_actionMenuMirrorCheck->setEnabled(false);
    }
  }

  /*if (isAURGroupSelected() && Package::getForeignRepositoryToolName() == ctn_KCP_TOOL)
  {
    ui->actionCheckUpdates->setEnabled(true);
  }*;
  else if (Package::getForeignRepositoryToolName() != ctn_KCP_TOOL)
  {
    ui->actionCheckUpdates->setEnabled(value);
  }*/
  if (!isAURGroupSelected())
  {
    m_actionMenuOptions->setEnabled(value);
    ui->actionGetNews->setEnabled(value);
    ui->actionInstallLocalPackage->setEnabled(value);
    ui->actionCheckUpdates->setEnabled(value);
    m_actionChangeInstallReason->setEnabled(value);
  }
  else
  {
    m_actionMenuOptions->setEnabled(false);
    ui->actionGetNews->setEnabled(false);
    ui->actionInstallLocalPackage->setEnabled(false);
    ui->actionCheckUpdates->setEnabled(false);
    m_actionChangeInstallReason->setEnabled(false);
  }

  if (value && m_outdatedStringList->count() > 0)
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

  if (isNotifierBusy()) return;

  //Are there any remove actions to be commited?
  if(getRemoveTransactionParentItem()->rowCount() > 0 && getInstallTransactionParentItem()->rowCount() > 0)
  {
    if (!isAURGroupSelected())
      doRemoveAndInstall();
    else
      doRemoveAndInstallAUR();
  }
  else if(getRemoveTransactionParentItem()->rowCount() > 0)
  {
    if (!isAURGroupSelected())
      doRemove();
    else
      doRemoveAUR();
  }
  else if(getInstallTransactionParentItem()->rowCount() > 0)
  {
    if (!isAURGroupSelected())
      doInstall();
    else
      doInstallAUR();
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
bool MainWindow::stopTransaction()
{
  bool ret=false;

  if (m_commandExecuting != ectn_NONE && m_pacmanExec != nullptr)
  {
    int res = m_pacmanExec->cancelProcess();
    if (res != 1)
    {
      m_sharedMemory->detach();
      ret=true;
    }
  }

  return ret;
}

/*
 * Called whenever Octopi's parser detects a potential for enabling/disabling stop transaction button
 */
void MainWindow::onCanStopTransaction(bool yesNo)
{
  if (yesNo && m_progressWidget->isHidden()) return;
  if (SettingsManager::getShowStopTransaction()) m_toolButtonStopTransaction->setVisible(yesNo);
}

/*
 * This SLOT is called when Pacman's process has finished execution [PacmanExec based!!!]
 */
void MainWindow::pacmanProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
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

      for(const QString& pkg: pkgs)
      {
        QStringList names = pkg.split(QStringLiteral(" "), Qt::SkipEmptyParts);
        if (names.count() > 0)
        {
          m_checkupdatesStringList->append(names.at(0));
          m_checkUpdatesNameNewVersion->insert(names.at(0), names.at(3));
        }
      }
    }
    else if (pkgs.count()==0)
    {
      writeToTabOutput(StrConstants::getNoUpdatesAvailable() + QLatin1String("<br>"));
    }
  }

  if ((exitCode == 0 || (exitCode == 2 && m_commandExecuting == ectn_CHECK_UPDATES)) &&
      exitStatus == QProcess::NormalExit)
  {
    //First, we empty the tabs cache!
    m_cachedPackageInInfo = QLatin1String("");
    m_cachedPackageInFiles = QLatin1String("");

    //If there are .pacnew files to print...
    QStringList dotPacnewFiles = m_pacmanExec->getDotPacnewFileList();
    if (dotPacnewFiles.count() > 0)
    {
      writeToTabOutput(QStringLiteral("<br>"));
      for(const QString& dotPacnewFile: dotPacnewFiles)
      {
        if (!dotPacnewFile.contains(QLatin1String("<br>")))
          writeToTabOutput( QLatin1String("<br>") + dotPacnewFile, ectn_DONT_TREAT_URL_LINK);
        else
          writeToTabOutput(dotPacnewFile, ectn_DONT_TREAT_URL_LINK);
      }
    }

    writeToTabOutput(QLatin1String("<br><b>") + StrConstants::getCommandFinishedOK() + QLatin1String("</b><br>"));
  }
  else if (exitCode == ctn_PACMAN_PROCESS_EXECUTING)
  {
    writeToTabOutput(StrConstants::getErrorPacmanProcessExecuting() + QLatin1String("<br>"));
    writeToTabOutput(QLatin1String("<br><b>") + StrConstants::getCommandFinishedWithErrors() + QLatin1String("</b><br>"));
  }
  else
  {
    writeToTabOutput(QLatin1String("<br><b>") + StrConstants::getCommandFinishedWithErrors() + QLatin1String("</b><br>"));
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
    if(exitCode == 0)
    {
      //After the command, we can refresh the package list, so any change can be seem.
      if (m_commandExecuting == ectn_CHECK_UPDATES)
      {
        //Sets NOW as the last sync time value
        SettingsManager::setLastCheckUpdatesTime(QDateTime::currentDateTime());

        //Retrieves the RSS News from respective Distro site...
        refreshDistroNews(true, false);

        //Did it synchronize any repo? If so, let's refresh some things...
        if (UnixCommand::isAppRunning(QStringLiteral("octopi-notifier"), true) ||
            (IsSyncingRepoInTabOutput()))
        {
          bool aurGroup = isAURGroupSelected();
          if (!aurGroup)
          {
            //metaBuildPackageList();
            refreshOutdatedPackageList();
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
        {
          QStringList sl;
          sl << QStringLiteral("--print-format") << QStringLiteral("%n %v %s") << QStringLiteral("-Spu");
          execCommandInAnotherThread(QStringLiteral("pacman"), sl);
        }
      }
      else if (m_commandExecuting == ectn_SYSTEM_UPGRADE ||
               m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL)
      {        
        m_checkupdatesStringList->clear();
        m_checkUpdatesNameNewVersion->clear();
        m_leFilterPackage->clear();
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

  if (exitCode != 0 && (textInTabOutput(QStringLiteral("conflict")))) //|| _textInTabOutput("could not satisfy dependencies")))
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
    m_actionSwitchToForeignTool->setCheckable(true);
    m_actionSwitchToForeignTool->setChecked(false);
    m_actionSwitchToForeignTool->setText(StrConstants::getUseForeignTool());
    m_actionSwitchToForeignTool->setToolTip(m_actionSwitchToForeignTool->text() + QLatin1String("  (Ctrl+Shift+Y)"));
  }
  else if (aurTools.count() > 1) //It seems the AUR tool has just been removed...
  {
    m_actionSwitchToForeignTool->setText(QLatin1String(""));
    m_actionSwitchToForeignTool->setToolTip(QStringLiteral("AUR"));
    m_actionSwitchToForeignTool->setCheckable(false);
    m_actionSwitchToForeignTool->setChecked(false);
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
    QString octopiConfDir = QDir::homePath() + QDir::separator() + QLatin1String(".config/octopi");
    QString yaySymlink = octopiConfDir + QDir::separator() + QLatin1String("yay");
    QFileInfo info(yaySymlink);
    if (info.isSymLink())
    {
      QFileInfo fi(info.symLinkTarget());
      QFile::remove(yaySymlink);
      QProcess remove;
      remove.start(QStringLiteral("rm"), QStringList() << QStringLiteral("-Rf") << fi.canonicalPath());
      remove.waitForFinished();
    }

    if (UnixCommand::hasTheExecutable(QStringLiteral("yay")))
    {
      refreshHelpUsageText();
      SettingsManager::setAURTool(ctn_YAY_TOOL);
      m_actionSwitchToForeignTool->setToolTip(StrConstants::getUseForeignTool());
      m_actionSwitchToForeignTool->setToolTip(m_actionSwitchToForeignTool->toolTip() + QLatin1String("  (Ctrl+Shift+Y)"));
      m_actionSwitchToForeignTool->setCheckable(true);
      m_actionSwitchToForeignTool->setChecked(false);
      writeToTabOutput(QLatin1String("<br><b>") + StrConstants::getCommandFinishedOK() + QLatin1String("</b><br>"));
    }
    else
    {
      writeToTabOutput(QLatin1String("<br><b>") + StrConstants::getCommandFinishedWithErrors() + QLatin1String("</b><br>"));
    }
  }

  m_progressWidget->setValue(0);
  m_progressWidget->show();
  bool bRefreshGroups = true;
  clearTransactionTreeView();

  if (m_commandExecuting == ectn_RUN_SYSTEM_UPGRADE_IN_TERMINAL)
  {
    m_checkupdatesStringList->clear();
    m_checkUpdatesNameNewVersion->clear();
    m_leFilterPackage->clear();
  }

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

  delete m_pacmanExec;

  m_commandExecuting = ectn_NONE;
  m_console->execute(QLatin1String(""));
  m_console->setFocus();

  if (m_cic)
  {
    delete m_cic;
    m_cic = nullptr;
  }
}

/*
 * Whenever a user strikes Ctrl+C, Ctrl+D or Ctrl+Z in the terminal
 */
void MainWindow::onCancelControlKey()
{
  if (m_commandExecuting != ectn_NONE)
  {
    //clearTransactionTreeView();
    enableTransactionActions();

    delete m_pacmanExec;

    m_pacmanExec = nullptr;
    m_commandExecuting = ectn_NONE;
  }
}

/*
 * A helper method which writes the given string to OutputTab's textbrowser
 */
void MainWindow::writeToTabOutput(const QString &msg, TreatURLLinks treatURLLinks)
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));
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
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>(QStringLiteral("textBrowser"));
  if (text)
  {
    if (m_commandExecuting == ectn_CHECK_UPDATES)
    {
      QString newstr = output;
      newstr.replace(QStringLiteral("\n"), QStringLiteral("<br>"));
      text->insertHtml(newstr);
      text->ensureCursorVisible();
      return;
    }
    else if ((m_commandExecuting == ectn_RUN_IN_TERMINAL && SettingsManager::getTerminal() != ctn_QTERMWIDGET) ||
        (m_commandExecuting != ectn_RUN_IN_TERMINAL && SettingsManager::getTerminal() == ctn_QTERMWIDGET))
    {
      ensureTabVisible(ctn_TABINDEX_OUTPUT);
      positionTextEditCursorAtEnd();
    }

    text->insertHtml(output);
    text->ensureCursorVisible();
  }
}
