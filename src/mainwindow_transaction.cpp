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
#include <iostream>
#include <QComboBox>
#include <QProgressBar>
#include <QMessageBox>
#include <QStandardItem>
#include <QSortFilterProxyModel>
#include <QTextBrowser>

/*
 * Watches the state of tvTransaction treeview to see if Commit/Rollback actions must be activated/deactivated
 */
void MainWindow::changeTransactionActionsState()
{
  bool state = _isThereAPendingTransaction();

  ui->actionCommit->setEnabled(state);
  ui->actionRollback->setEnabled(state);
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
bool MainWindow::_isThereAPendingTransaction()
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
  QStandardItem *si;

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
  QStandardItem *si;

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
  _ensureTabVisible(ctn_TABINDEX_TRANSACTION);

  QStandardItemModel *sim=_getCurrentSelectedModel();

  //First, let's see if we are dealing with a package group
  if(m_cbGroups->currentIndex() != 0)
  {
    //If we are trying to remove all the group's packages, why not remove the entire group?
    if(ui->tvPackages->selectionModel()->selectedRows().count() == sim->rowCount())
    {
      insertRemovePackageIntoTransaction(m_cbGroups->currentText());
      return;
    }
  }

  foreach(QModelIndex item, ui->tvPackages->selectionModel()->selectedRows())
  {
    QModelIndex mi = m_proxyModelPackages->mapToSource(item);
    QStandardItem *si = sim->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);
    QStandardItem *siRepository = sim->item(mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);

    insertRemovePackageIntoTransaction(siRepository->text() + "/" + si->text());
    //insertRemovePackageIntoTransaction(si->text());
  }
}

/*
 * Inserts the current selected group for removal into the Transaction Treeview
 */
void MainWindow::insertGroupIntoRemovePackage()
{
  _ensureTabVisible(ctn_TABINDEX_TRANSACTION);
  insertRemovePackageIntoTransaction(m_cbGroups->currentText());
}

/*
 * Inserts the current selected packages for installation into the Transaction Treeview
 * This is the SLOT, it needs to call insertInstallPackageIntoTransaction(PackageName) to work!
 */
void MainWindow::insertIntoInstallPackage()
{
  _ensureTabVisible(ctn_TABINDEX_TRANSACTION);

  QStandardItemModel *sim=_getCurrentSelectedModel();

  //First, let's see if we are dealing with a package group
  if(m_cbGroups->currentIndex() != 0)
  {
    //If we are trying to insert all the group's packages, why not insert the entire group?
    if(ui->tvPackages->selectionModel()->selectedRows().count() == sim->rowCount())
    {
      insertInstallPackageIntoTransaction(m_cbGroups->currentText());
      return;
    }
  }

  foreach(QModelIndex item, ui->tvPackages->selectionModel()->selectedRows())
  {
    QModelIndex mi = m_proxyModelPackages->mapToSource(item);
    QStandardItem *si = sim->item(mi.row(), ctn_PACKAGE_NAME_COLUMN);
    QStandardItem *siRepository = sim->item(mi.row(), ctn_PACKAGE_REPOSITORY_COLUMN);

    insertInstallPackageIntoTransaction(siRepository->text() + "/" + si->text());
    //insertInstallPackageIntoTransaction(si->text());
  }
}

/*
 * Inserts the current selected group for removal into the Transaction Treeview
 */
void MainWindow::insertGroupIntoInstallPackage()
{
  _ensureTabVisible(ctn_TABINDEX_TRANSACTION);
  insertInstallPackageIntoTransaction(m_cbGroups->currentText());
}

/*
 * Adjust the count and selection count status of the selected tvTransaction item (Remove or Insert parents)
 */
void MainWindow::_tvTransactionAdjustItemText(QStandardItem *item)
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
  _tvTransactionAdjustItemText(getRemoveTransactionParentItem());
  _tvTransactionAdjustItemText(getInstallTransactionParentItem());
}

/*
 * Method called every time some item is inserted or removed in tvTransaction treeview
 */
void MainWindow::_tvTransactionRowsChanged(const QModelIndex& parent)
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
      _tvTransactionAdjustItemText(itemRemove);
    }
    else itemRemove->setText(StrConstants::getTransactionRemoveText());
  }
  else if (item == itemInstall)
  {
    if (item->rowCount() > 0)
    {
      itemInstall->setText(StrConstants::getTransactionInstallText() + " (" + count + ")");
      _tvTransactionAdjustItemText(itemInstall);
    }
    else itemInstall->setText(StrConstants::getTransactionInstallText());
  }
}

/*
 * SLOT called each time some item is inserted into tvTransaction
 */
void MainWindow::tvTransactionRowsInserted(const QModelIndex& parent, int, int)
{
  _tvTransactionRowsChanged(parent);
}

/*
 * SLOT called each time some item is removed from tvTransaction
 */
void MainWindow::tvTransactionRowsRemoved(const QModelIndex& parent, int, int)
{
  _tvTransactionRowsChanged(parent);
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
bool MainWindow::_isSUAvailable()
{
  //If there are no means to run the actions, we must warn!
  if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
    QMessageBox::critical( 0, StrConstants::getApplicationName(),
                           StrConstants::getErrorNoSuCommand() +
                           "\n" + StrConstants::getYoullNeedSuFrontend());
    return false;
  }
  else
    return true;
}

/*
 * Does a repository sync with "pacman -Sy" !
 */
void MainWindow::doSyncDatabase()
{
  //If there are no means to run the actions, we must warn!
  if (!_isSUAvailable()) return;

  //Retrieves the RSS News from respective Distro site...
  refreshDistroNews(true, false);

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

  QString command = "pacman -Sy";

  m_lastCommandList.clear();
  m_lastCommandList.append("pacman -Sy;");
  m_lastCommandList.append("echo -e;");
  m_lastCommandList.append("read -n1 -p \"" + StrConstants::getPressAnyKey() + "\"");

  m_unixCommand->executeCommand(command);
}

/*
 * Does a system upgrade with "pacman -Su" !
 */
void MainWindow::doSystemUpgrade(bool syncDatabase)
{
  qApp->processEvents();

  if(syncDatabase)
  {
    m_commandQueued = ectn_SYSTEM_UPGRADE;
    doSyncDatabase();
  }
  else
  {
    //Shows a dialog indicating the targets needed to be retrieved and asks for the user's permission.
    QList<PackageListData> * targets = Package::getTargetUpgradeList();

    //There are no new updates to install!
    if (targets->count() == 0)
    {
      clearTabOutput();
      writeToTabOutputExt("<b>" + StrConstants::getNoNewUpdatesAvailable() + "</b>");
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

    totalDownloadSize = totalDownloadSize / 1024;
    QString ds = QString::number(totalDownloadSize, 'f', 2);

    TransactionDialog question(this);

    if(targets->count()==1)
      question.setText(StrConstants::getRetrieveTarget() +
                       "\n\n" + StrConstants::getTotalDownloadSize().arg(ds));
    else
      question.setText(StrConstants::getRetrieveTargets().arg(targets->count()) +
                       "\n\n" + StrConstants::getTotalDownloadSize().arg(ds));

    question.setWindowTitle(StrConstants::getConfirmation());
    question.setInformativeText(StrConstants::getConfirmationQuestion());
    question.setDetailedText(list);
    int result = question.exec();

    if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
    {
      //If there are no means to run the actions, we must warn!
      if (!_isSUAvailable()) return;

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
        m_commandExecuting = ectn_RUN_IN_TERMINAL;
        m_unixCommand->runCommandInTerminal(m_lastCommandList);
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
  QStringList *removeTargets = Package::getTargetRemovalList(listOfRemoveTargets, m_removeCommand);
  QString removeList;
  QString allLists;
  TransactionDialog question(this);
  QString dialogText;

  foreach(QString removeTarget, *removeTargets)
  {
    removeList = removeList + StrConstants::getRemove() + " "  + removeTarget + "\n";
  }

  if (removeList.count() == 0)
  {
    removeTargets->append(listOfRemoveTargets);
    removeList.append(StrConstants::getRemove() + " "  + listOfRemoveTargets);
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
  QString ds = QString::number(totalDownloadSize, 'f', 2);

  if (installList.count() == 0)
  {
    installTargets->append(PackageListData(listOfInstallTargets, "", 0));
    installList.append(StrConstants::getInstall() + " " + listOfInstallTargets);
  }

  allLists.append(removeList);
  allLists.append(installList);

  if(removeTargets->count()==1)
  {
    if (removeTargets->at(0).indexOf("HoldPkg was found in") != -1)
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

  dialogText = StrConstants::getRemoveTargets().arg(removeTargets->count()) + "\n";
  dialogText += StrConstants::getRetrieveTargets().arg(installTargets->count()) +
      "\n\n" + StrConstants::getTotalDownloadSize().arg(ds);

  question.setText(dialogText);
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(allLists);
  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    //If there are no means to run the actions, we must warn!
    if (!_isSUAvailable()) return;

    QString command;
    command = "pacman -" + m_removeCommand + " --noconfirm " + listOfRemoveTargets +
        "; pacman -S --noconfirm " + listOfInstallTargets;

    m_lastCommandList.clear();
    m_lastCommandList.append("pacman -" + m_removeCommand + " " + listOfRemoveTargets + ";");
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

  foreach(QString target, *m_targets)
  {
    list = list + target + "\n";
  }
  list.remove(list.size()-1, 1);

  if (list.count() == 0)
  {
    m_targets->append(listOfTargets);
    list.append(listOfTargets);
  }

  TransactionDialog question(this);

  //Shows a dialog indicating the targets which will be removed and asks for the user's permission.
  if(m_targets->count()==1)
  {
    if (m_targets->at(0).indexOf("HoldPkg was found in") != -1)
    {
      QMessageBox::warning(
            this, StrConstants::getAttention(), StrConstants::getWarnHoldPkgFound(), QMessageBox::Ok);
      return;
    }
    else question.setText(StrConstants::getRemoveTarget());
  }
  else
    question.setText(StrConstants::getRemoveTargets().arg(m_targets->count()));

  if (getNumberOfTobeRemovedPackages() < m_targets->count())
    question.setWindowTitle(StrConstants::getWarning());
  else
    question.setWindowTitle(StrConstants::getConfirmation());

  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    //If there are no means to run the actions, we must warn!
    if (!_isSUAvailable()) return;

    QString command;
    command = "pacman -" + m_removeCommand + " --noconfirm " + listOfTargets;

    m_lastCommandList.clear();
    m_lastCommandList.append("pacman -" + m_removeCommand + " " + listOfTargets + ";");
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
  QString ds = QString::number(totalDownloadSize, 'f', 2);

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
    else question.setText(StrConstants::getRetrieveTarget() +
                          "\n\n" + StrConstants::getTotalDownloadSize().arg(ds));
  }
  else
    question.setText(StrConstants::getRetrieveTargets().arg(targets->count()) +
                     "\n\n" + StrConstants::getTotalDownloadSize().arg(ds));

  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  int result = question.exec();

  if(result == QDialogButtonBox::Yes || result == QDialogButtonBox::AcceptRole)
  {
    //If there are no means to run the actions, we must warn!
    if (!_isSUAvailable()) return;

    QString command;
    command = "pacman -S --noconfirm " + listOfTargets;

    m_lastCommandList.clear();
    m_lastCommandList.append("pacman -S " + listOfTargets + ";");
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
  //If there are no means to run the actions, we must warn!
  if (!_isSUAvailable()) return;

  int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                  StrConstants::getCleanCacheConfirmation(),
                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

  if (res == QMessageBox::Yes)
  {
    disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));
    clearTabOutput();
    writeToTabOutputExt("<b>" + StrConstants::getCleaningPackageCache() + "</b>");
    qApp->processEvents();
    CPUIntensiveComputing cic;

    bool res = UnixCommand::cleanPacmanCache();
    qApp->processEvents();

    if (res)
    {
      writeToTabOutputExt("<b>" + StrConstants::getCommandFinishedOK() + "</b>");
      metaBuildPackageList();
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
  bool state = _isThereAPendingTransaction();

  if (value == true && state == true)
  {
    ui->actionCommit->setEnabled(true);
    ui->actionRollback->setEnabled(true);
  }
  else if ((value == true && state == false) || value == false)
  {
    ui->actionCommit->setEnabled(false);
    ui->actionRollback->setEnabled(false);
  }

  ui->actionInstall->setEnabled(value);
  ui->actionRemove->setEnabled(value);
  ui->actionSyncPackages->setEnabled(value);

  /*if (value == false)
  {
    ui->actionSystemUpgrade->setEnabled(false);
  }
  else
  {
    //Let's see if we can enable system upgrade
    QList<PackageListData> * targets = Package::getTargetUpgradeList();
    if (targets->count() == 0)
      ui->actionSystemUpgrade->setEnabled(false);
    else
      ui->actionSystemUpgrade->setEnabled(true);
  }*/

  ui->actionSystemUpgrade->setEnabled(value);

  ui->actionGetNews->setEnabled(value);
}

/*
 * Triggers the especific methods that need to be called given the packages in the transaction
 */
void MainWindow::doCommitTransaction()
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
void MainWindow::doRollbackTransaction()
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

/*
 * This SLOT is called whenever Pacman's process has just started execution
 */
void MainWindow::actionsProcessStarted()
{
  m_progressWidget->setValue(0);
  m_progressWidget->setMaximum(100);
  m_iLoveCandy = UnixCommand::isILoveCandyEnabled();

  disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  clearTabOutput();

  //First we output the name of action we are starting to execute!
  if (m_commandExecuting == ectn_SYNC_DATABASE)
  {
    writeToTabOutput("<b>" + StrConstants::getSyncDatabases() + "</b><br><br>");
  }
  else if (m_commandExecuting == ectn_SYSTEM_UPGRADE)
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
 */
void MainWindow::actionsProcessFinished(int exitCode, QProcess::ExitStatus)
{
  m_progressWidget->close();

  ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName());

  if (exitCode == 0){
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
    doSystemUpgrade(false);
    m_commandQueued = ectn_NONE;
  }
  else if (m_commandQueued == ectn_NONE)
  {
    if(exitCode == 0)
    {
      //After the command, we can refresh the package list, so any change can be seem.
      if (m_commandExecuting == ectn_SYNC_DATABASE)
      {
        //Did it synchronize any repo? If so, let's refresh some things...
        if (_textInTabOutput(StrConstants::getSyncing()))
        {
          int oldIndex = m_cbGroups->currentIndex();
          m_cbGroups->setCurrentIndex(0);
          refreshComboBoxGroups();

          if (oldIndex == 0)
          {
            buildPackageList();
          }
        }
      }
      else
      {
        buildPackageList();
      }

      connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

      clearTransactionTreeView();
    }
    else
      connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));
  }

  if (exitCode != 0 && (_textInTabOutput("conflict"))) //|| _textInTabOutput("could not satisfy dependencies")))
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
  m_commandExecuting = ectn_NONE;
  m_unixCommand->removeTemporaryActionFile();
}

/*
 * This SLOT is called whenever Pacman's process has something to output to Standard out
 */
void MainWindow::actionsProcessReadOutput()
{
  if (WMHelper::getSUCommand().contains("kdesu"))
  {
    QString msg = m_unixCommand->readAllStandardOutput();
    _splitOutputStrings(msg);
  }
  else if (WMHelper::getSUCommand().contains("gksu"))
  {
    QString msg = m_unixCommand->readAllStandardOutput();
    msg = msg.trimmed();

    //std::cout << "Out: " << msg.toAscii().data() << std::endl;

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
bool MainWindow::_searchForKeyVerbs(const QString &msg)
{
  return (msg.contains(QRegExp("checking ")) ||
          msg.contains(QRegExp("loading ")) ||
          msg.contains(QRegExp("installing ")) ||
          msg.contains(QRegExp("upgrading ")) ||
          msg.contains(QRegExp("resolving ")) ||
          msg.contains(QRegExp("looking ")) ||
          msg.contains(QRegExp("removing ")));
}

/*
 * Processes the output of the 'pacman process' so we can update percentages and messages at real time
 */
void MainWindow::_treatProcessOutput(const QString &pMsg)
{  
  if (m_commandExecuting == ectn_RUN_IN_TERMINAL) return;

  bool continueTesting = false;
  QString perc;
  QString msg = pMsg;

  QString progressRun;
  QString progressEnd;

  msg.remove(QRegExp(".+\\[Y/n\\].+"));
  msg.remove(QRegExp("warning:\\S{0}"));

  //std::cout << "_treat: " << msg.toAscii().data() << std::endl;

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
  if(msg.indexOf(progressEnd) != -1) //indexOf("#]") != -1)
  {
    perc = "100%";
    if (!m_progressWidget->isVisible()) m_progressWidget->show();
    m_progressWidget->setValue(100);
    continueTesting = true;
  }

  if (msg.indexOf(progressRun) != -1 /*indexOf("-]") != -1*/ || continueTesting)
  {
    if (!continueTesting){
      perc = msg.right(4).trimmed();
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

        if (_searchForKeyVerbs(msg))
        {
          int end = msg.indexOf("[");
          msg = msg.remove(end, msg.size()-end).trimmed() + " ";
          writeToTabOutputExt(msg);
        }
        else
        {
          //std::cout << "test1: " << target.toAscii().data() << std::endl;

          int pos = msg.indexOf(" ");
          if (pos >=0)
          {
            target = msg.left(pos);
            target = target.trimmed() + " ";
            //std::cout << "target: " << target.toAscii().data() << std::endl;

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
        if (_searchForKeyVerbs(msg))
        {
          //std::cout << "test2: " << msg.toAscii().data() << std::endl;

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
            //std::cout << "target: " << target.toAscii().data() << std::endl;

            if(!target.isEmpty() && !_textInTabOutput(target))
            {
              if (target.indexOf(QRegExp("[a-z]+")) != -1)
              {
                if(m_commandExecuting == ectn_SYNC_DATABASE)
                {
                  writeToTabOutputExt("<b><font color=\"#FF8040\">" +
                                      StrConstants::getSyncing() + " " + target + "</font></b>");
                }
                else
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
      //std::cout << "percentage is: " << percentage << std::endl;
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
    msg.remove(QRegExp("Do you want.+"));

    msg = msg.trimmed();

    //std::cout << "debug: " << msg.toAscii().data() << std::endl;

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
      if (msg.contains(QRegExp("removing ")) && !_textInTabOutput(msg + " "))
      {
        //Does this package exist or is it a proccessOutput buggy string???
        QString pkgName = msg.mid(9).trimmed();

        if (pkgName.indexOf("...") != -1 ||
            m_modelInstalledPackages->findItems(pkgName,
                                                Qt::MatchExactly,
                                                ctn_PACKAGE_NAME_COLUMN).count() != 0)
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
          //std::cout << "Entered here: " << msg.toAscii().data() << std::endl;

          if (m_commandExecuting == ectn_SYNC_DATABASE &&
              msg.indexOf("is up to date"))
          {
            if (!m_progressWidget->isVisible()) m_progressWidget->show();
            m_progressWidget->setValue(100);

            int blank = msg.indexOf(" ");
            QString repo = msg.left(blank);

            if (repo.contains("error")) return;

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
bool MainWindow::_splitOutputStrings(const QString &output)
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

            //std::cout << "Error1: " << aux.toAscii().data() << std::endl;
            _treatProcessOutput(aux);
          }
        }
      }
      else if (maux.count() == 1)
      {
        if (!m.isEmpty())
        {
          //std::cout << "Error2: " << m.toAscii().data() << std::endl;
          _treatProcessOutput(m);
        }
      }
    }
    else if (m2.count() > 1)
    {
      foreach (QString m3, m2)
      {
        if (!m3.isEmpty())
        {
          //std::cout << "Error3: " << m3.toAscii().data() << std::endl;
          _treatProcessOutput(m3);
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
  _splitOutputStrings(msg);
}

/*
 * A helper method which writes the given string to OutputTab's textbrowser
 */
void MainWindow::writeToTabOutput(const QString &msg)
{
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (text)
  {
    _ensureTabVisible(ctn_TABINDEX_OUTPUT);
    _positionTextEditCursorAtEnd();
    text->insertHtml(Package::makeURLClickable(msg));
    text->ensureCursorVisible();
  }
}

/*
 * A helper method which writes the given string to OutputTab's textbrowser
 * This is the EXTENDED version, it checks lots of things before writing msg
 */
void MainWindow::writeToTabOutputExt(const QString &msg)
{
  //std::cout << "To print: " << msg.toAscii().data() << std::endl;

  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
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
    if(_textInTabOutput(msg))
    {
      return;
    }

    QString newMsg = msg;
    _ensureTabVisible(ctn_TABINDEX_OUTPUT);
    _positionTextEditCursorAtEnd();

    if(newMsg.contains(QRegExp("<font color")))
    {
      newMsg += "<br>";
    }
    else
    {
      if(newMsg.contains("removing ") ||
         newMsg.contains("could not ") ||
         newMsg.contains("error") ||
         newMsg.contains("failed"))
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
      else if (newMsg.contains("warning"))
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

    text->insertHtml(Package::makeURLClickable(newMsg));
    text->ensureCursorVisible();
  }
}
