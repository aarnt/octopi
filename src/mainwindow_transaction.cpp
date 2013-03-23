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
 * This is a MainWindow's Transaction related code
 */

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "uihelper.h"
#include "wmhelper.h"
#include "strconstants.h"
#include <iostream>
#include <QComboBox>
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
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
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
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
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
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItem * siPackageToRemove = new QStandardItem(pkgName);
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siRemoveParent->model());

  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  if (foundItems.size() == 0)
  {
    siRemoveParent->appendRow(siPackageToRemove);
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
  QTreeView *tvTransaction = ui->twProperties->widget(ctn_TABINDEX_TRANSACTION)->findChild<QTreeView*>("tvTransaction");
  QStandardItem * siInstallParent = getInstallTransactionParentItem();
  QStandardItem * siPackageToInstall = new QStandardItem(pkgName);
  QStandardItem * siRemoveParent = getRemoveTransactionParentItem();
  QStandardItemModel *sim = qobject_cast<QStandardItemModel *>(siInstallParent->model());

  QList<QStandardItem *> foundItems = sim->findItems(pkgName, Qt::MatchRecursive | Qt::MatchExactly);

  if (foundItems.size() == 0)
  {
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
 * Retrieve the list of all packages scheduled to be installed
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

  QStandardItemModel *sim;
  if(ui->actionNonInstalledPackages->isChecked())
  {
    if(m_cbGroups->currentIndex() == 0)
      sim = m_modelPackages;
    else
      sim = m_modelPackagesFromGroup;
  }
  else
  {
    if(m_cbGroups->currentIndex() == 0)
      sim = m_modelInstalledPackages;
    else
      sim = m_modelInstalledPackagesFromGroup;
  }

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

    insertRemovePackageIntoTransaction(si->text());
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

  QStandardItemModel *sim;
  if(ui->actionNonInstalledPackages->isChecked())
  {
    if(m_cbGroups->currentIndex() == 0)
      sim = m_modelPackages;
    else
      sim = m_modelPackagesFromGroup;
  }
  else
  {
    if(m_cbGroups->currentIndex() == 0)
      sim = m_modelInstalledPackages;
    else
      sim = m_modelInstalledPackagesFromGroup;
  }

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

    insertInstallPackageIntoTransaction(si->text());
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

  QTreeView *tvTransaction = ui->twProperties->currentWidget()->findChild<QTreeView*>("tvTransaction");
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
 * Does a repository sync with "pacman -Sy" !
 */
void MainWindow::doSyncDatabase()
{
  //If there are no means to run the actions, we must warn!
  if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand() +
                           "\n" + StrConstants::getYoullNeedSuFrontend());
    return;
  }

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
  m_unixCommand->executeCommand(command);
}

/*
 * Does a system upgrade with "pacman -Su" !
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

    question.setWindowTitle(StrConstants::getConfirmation());
    question.setInformativeText(StrConstants::getConfirmationQuestion());
    question.setDetailedText(list);
    question.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Close);
    question.setDefaultButton(QMessageBox::No);

    int result = question.exec();
    if(result == QMessageBox::Yes)
    {
      //If there are no means to run the actions, we must warn!
      if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
        QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand() +
                               "\n" + StrConstants::getYoullNeedSuFrontend());
        return;
      }

      m_commandExecuting = ectn_SYSTEM_UPGRADE;

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
      command = "pacman -Su --noconfirm";
      m_unixCommand->executeCommand(command);
      m_commandQueued = ectn_NONE;
    }
  }
}

/*
 * Removes ALL the packages selected by the user with "pacman -Rcs (CASCADE)" !
 */
void MainWindow::doRemove()
{
  QString listOfTargets = getTobeRemovedPackages();
  m_targets = Package::getTargetRemovalList(listOfTargets);
  QString list;

  foreach(QString target, *m_targets)
  {
    list = list + target + "\n";
  }
  list.remove(list.size()-1, 1);

  QMessageBox question;

  Q_ASSERT(m_targets->count() > 0);

  //Shows a dialog indicating the targets which will be removed and asks for the user's permission.
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

  if (getNumberOfTobeRemovedPackages() < m_targets->count())
    question.setWindowTitle(StrConstants::getWarning());
  else
    question.setWindowTitle(StrConstants::getConfirmation());

  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  question.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Close);
  question.setDefaultButton(QMessageBox::No);

  int result = question.exec();
  if(result == QMessageBox::Yes)
  {
    //If there are no means to run the actions, we must warn!
    if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
      QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand() +
                             "\n" + StrConstants::getYoullNeedSuFrontend());
      return;
    }

    m_commandExecuting = ectn_REMOVE;

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
    command = "pacman -Rcs --noconfirm " + listOfTargets;
    m_unixCommand->executeCommand(command);
  }
}

/*
 * Installs ALL the packages selected by the user with "pacman -S (INCLUDING DEPENDENCIES)" !
 */
void MainWindow::doInstall()
{
  QString listOfTargets = getTobeInstalledPackages();
  m_targets = Package::getTargetUpgradeList(listOfTargets);
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

  question.setWindowTitle(StrConstants::getConfirmation());
  question.setInformativeText(StrConstants::getConfirmationQuestion());
  question.setDetailedText(list);
  question.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Close);
  question.setDefaultButton(QMessageBox::No);

  int result = question.exec();
  if(result == QMessageBox::Yes)
  {
    //If there are no means to run the actions, we must warn!
    if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
      QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand() +
                             "\n" + StrConstants::getYoullNeedSuFrontend());
      return;
    }

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

    QString command;
    command = "pacman -S --noconfirm " + listOfTargets;
    m_unixCommand->executeCommand(command);
  }
}

/*
 * Clears the local package cache using "pacman -Sc"
 */
void MainWindow::doCleanCache()
{
  //If there are no means to run the actions, we must warn!
  if (WMHelper::getSUCommand() == ctn_NO_SU_COMMAND){
    QMessageBox::critical( 0, StrConstants::getApplicationName(), StrConstants::getErrorNoSuCommand() +
                           "\n" + StrConstants::getYoullNeedSuFrontend());
    return;
  }

  int res = QMessageBox::question(this, StrConstants::getConfirmation(),
                                  StrConstants::getCleanCacheConfirmation(),
                                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No);

  if (res == QMessageBox::Yes)
  {
    clearTabOutput();
    writeToTabOutput("<b>" + StrConstants::getCleaningPackageCache() + "</b>");
    qApp->processEvents();
    CPUIntensiveComputing cic;
    UnixCommand::cleanPacmanCache();
    UnixCommand::removeTemporaryActionFile();
    qApp->processEvents();
    writeToTabOutput("<b>" + StrConstants::getCommandFinishedOK() + "</b>");
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
  ui->actionSystemUpgrade->setEnabled(value);
}

/*
 * Triggers the especific methods that need to be called given the packages in the transaction
 */
void MainWindow::doCommitTransaction()
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
  disconnect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

  clearTabOutput();

  //First we output the name of action we are starting to execute!
  if (m_commandExecuting == ectn_SYNC_DATABASE)
  {
    writeToTabOutput("<b>" + StrConstants::getSyncDatabases() + "</b><br>");
  }
  else if (m_commandExecuting == ectn_SYSTEM_UPGRADE)
  {
    writeToTabOutput("<b>" + StrConstants::getSystemUpgrade() + "</b><br>");
  }
  else if (m_commandExecuting == ectn_REMOVE)
  {
    writeToTabOutput("<b>" + StrConstants::getRemovingPackages() + "</b><br>");
  }
  else if (m_commandExecuting == ectn_INSTALL)
  {
    writeToTabOutput("<b>" + StrConstants::getInstallingPackages() + "</b><br>");
  }

  QString msg = m_unixCommand->readAllStandardOutput();
  msg = msg.trimmed();

  if (!msg.isEmpty())
  {
    writeToTabOutput(msg);
  }
}

/*
 * This SLOT is called when Pacman's process has finished execution
 */
void MainWindow::actionsProcessFinished(int exitCode, QProcess::ExitStatus)
{
  ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName());

  if (exitCode == 0){
    writeToTabOutput("<br><b>" +
                     StrConstants::getCommandFinishedOK() + "</b><br>");
  }
  else
  {
    writeToTabOutput("<br><b>" +
                     StrConstants::getCommandFinishedWithErrors() + "</b><br>");
  }

  if(m_commandQueued == ectn_SYSTEM_UPGRADE)
  {
    doSystemUpgrade(false);
    m_commandQueued = ectn_NONE;
  }
  else if (m_commandQueued == ectn_INSTALL && m_commandExecuting == ectn_REMOVE)
  {
    if(exitCode == 0) //If the removal actions were OK...
    {
      removePackagesFromRemoveTransaction();
      doInstall();
      removePackagesFromInstallTransaction();
      m_commandQueued = ectn_NONE;
      return;
    }
  }
  else if (m_commandQueued == ectn_NONE)
  {
    if(exitCode == 0)
    {
      //After the command, we can refresh the package list, so any change can be seem.
      if (m_commandExecuting == ectn_SYNC_DATABASE)
      {
        int oldIndex = m_cbGroups->currentIndex();
        m_cbGroups->setCurrentIndex(0);
        refreshComboBoxGroups();

        if (oldIndex == 0) buildPackageList();
      }
      else
      {
        metaBuildPackageList();
      }

      connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));

      clearTransactionTreeView();
    }
    else
      connect(m_pacmanDatabaseSystemWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(metaBuildPackageList()));
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
      writeToTabOutput(msg);
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
  bool continueTesting = false;
  QString perc;
  QString msg = pMsg;
  msg.remove(QRegExp(".+\\[Y/n\\].+"));
  msg.remove(QRegExp("warning:\\S{0}"));

  //if (msg.contains("removing")) std::cout << "_treat: " << msg.toAscii().data() << std::endl;

  //If it is a percentage, we are talking about curl output...
  if(msg.indexOf("#]") != -1)
  {
    perc = "100%";
    ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName() + " (100%)");    
    continueTesting = true;
  }

  if (msg.indexOf("-]") != -1 || continueTesting) //IT WAS AN ELSE HERE!!!
  {
    if (!continueTesting){
      perc = msg.right(4).trimmed();
    }

    continueTesting = false;

    QString target;
    if (m_commandExecuting == ectn_INSTALL ||
        m_commandExecuting == ectn_SYSTEM_UPGRADE ||
        m_commandExecuting == ectn_SYNC_DATABASE ||
        m_commandExecuting == ectn_REMOVE)
    {
      QString order;
      int ini = msg.indexOf(QRegExp("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "));
      if (ini == 0)
      {
        int rp = msg.indexOf(")");
        order = msg.left(rp+2);
        msg = msg.remove(0, rp+2);

        //std::cout << "Finding for: " << msg.toAscii().data() << std::endl;
        if (_searchForKeyVerbs(msg))
        {
          int end = msg.indexOf("[");
          msg = msg.remove(end, msg.size()-end).trimmed() + " ";

          //std::cout << "Finding for: " << msg.toAscii().data() << std::endl;

          //if(!_textInTabOutput(msg))
          {
            writeToTabOutput(msg);
          }
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

            if(!target.isEmpty() /*&& !_textInTabOutput(target)*/)
            {
              writeToTabOutput("<b><font color=\"#b4ab58\">" + target + "</font></b>"); //#C9BE62
            }
          }
          else /*if (!_textInTabOutput(msg))*/
            writeToTabOutput("<b><font color=\"#b4ab58\">" + msg + "</font></b>"); //#C9BE62
        }
      }
      else if (ini == -1)
      {
        //std::cout << "test2: " << msg.toAscii().data() << std::endl;
        if (_searchForKeyVerbs(msg))
        {
          //std::cout << "test2: " << msg.toAscii().data() << std::endl;

          int end = msg.indexOf("[");
          msg = msg.remove(end, msg.size()-end);
          msg = msg.trimmed() + " ";

          //if(!_textInTabOutput(msg))
            writeToTabOutput(msg);
        }
        else //NOT HERE
        {
          int pos = msg.indexOf(" ");
          if (pos >=0)
          {
            target = msg.left(pos);
            target = target.trimmed() + " ";
            //std::cout << "target: " << target.toAscii().data() << std::endl;

            if(!target.isEmpty() && !_textInTabOutput(target))
            {
              if(m_commandExecuting == ectn_SYNC_DATABASE)
              {
                writeToTabOutput("<b><font color=\"#FF8040\">" + StrConstants::getSynchronizing() + " " + target + "</font></b>");
              }
              else
              {
                writeToTabOutput("<b><font color=\"#b4ab58\">" + target + "</font></b>"); //#C9BE62
              }
            }
          }
          else /*if (!_textInTabOutput(msg))*/
            writeToTabOutput("<b><font color=\"blue\">" + msg + "</font></b>");
        }
      }
    }
    /*else if (m_commandExecuting == ectn_REMOVE)
    {
      std::cout << "remove: " << msg.toAscii().data() << std::endl;

      QString order;
      int ini = msg.indexOf(QRegExp("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "));
      if (ini == 0)
      {
        int rp = msg.indexOf(")");
        order = msg.left(rp+2);
        msg = msg.remove(0, rp+2);
      }

      int pos = msg.indexOf("[");
      if (pos >=0)
      {
        target = msg.left(pos);
        target = target.trimmed() + " ";
        //std::cout << "target: " << target.toAscii().data() << std::endl;
        //std::cout << "Finding for: " << target.toAscii().data() << std::endl;

        if(!target.isEmpty() && !_textInTabOutput(target))
        {
          writeToTabOutput("<font color=\"#E55451\">" + target + "</font>");
        }
      }
      else
      {
        msg = msg.trimmed() + " ";
        if (!_textInTabOutput(msg))
        {
          writeToTabOutput("<font color=\"#E55451\">" + msg + "</font>");
        }
      }
    }*/

    if(!perc.isEmpty() && perc.indexOf("%") > 0)
    {
      ui->twProperties->setTabText(ctn_TABINDEX_OUTPUT, StrConstants::getTabOutputName() + " (" + perc + ")");
    }
  }
  //It's another error, so we have to output it
  else
  {      
    //Let's supress some annoying string bugs...
    msg.remove(QRegExp("\\(process.+"));
    msg.remove(QRegExp("Using the fallback.+"));
    msg.remove(QRegExp("Gkr-Message: secret service operation failed:.+"));
    msg.remove(QRegExp("kdesu.+"));
    msg.remove(QRegExp("kbuildsycoca.+"));
    msg.remove(QRegExp("Connecting to deprecated signal.+"));
    msg.remove(QRegExp("QVariant.+"));
    msg.remove(QRegExp("gksu-run.+"));
    msg.remove(QRegExp("Do you want.+"));

    msg = msg.trimmed();

    //std::cout << "Another: " << msg.toAscii().data() << std::endl;

    QString order;
    int ini = msg.indexOf(QRegExp("\\(\\s{0,3}[0-9]{1,4}/[0-9]{1,4}\\) "));
    if (ini == 0)
    {
      int rp = msg.indexOf(")");
      order = msg.left(rp+2);
      msg = msg.remove(0, rp+2);
    }

    if (!msg.isEmpty() /*&& !_textInTabOutput(msg)*/)
    {
      if (msg.contains(QRegExp("removing ")))
        writeToTabOutput("<b><font color=\"#E55451\">" + msg + "</font></b>");
      else
      {
        if (msg.indexOf(":: Synchronizing package databases...") == -1 &&
            msg.indexOf(":: Starting full system upgrade...") == -1)
        {
          writeToTabOutput(msg); //it was font color = black
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
  //std::cout << "Print: " << msg.toAscii().data() << std::endl;
  QTextBrowser *text = ui->twProperties->widget(ctn_TABINDEX_OUTPUT)->findChild<QTextBrowser*>("textOutputEdit");
  if (text)
  {
    if(_textInTabOutput(msg))
    {
      return;
    }

    QString newMsg = msg;
    _ensureTabVisible(ctn_TABINDEX_OUTPUT);
    _positionTextEditCursorAtEnd();

    if(newMsg.contains(QRegExp("<font color"))) //&& !newMsg.contains(QRegExp("<br")))
    {
      //std::cout << "Already coloured: " << newMsg.toAscii().data() << std::endl;
      newMsg += "<br>";
    }
    else
    {
      if(newMsg.contains("removing ") ||
         newMsg.contains("could not ") ||
         newMsg.contains("error"))
      {
        newMsg = "<b><font color=\"#E55451\">" + newMsg + "</font></b>"; //RED
      }
      else if(newMsg.contains("checking ") ||
              newMsg.contains("-- reinstalling") ||
              newMsg.contains("installing ") ||
              newMsg.contains("upgrading ") ||
              newMsg.contains("loading ") ||
              newMsg.contains("resolving ") ||
              newMsg.contains("looking "))
       {
         newMsg = "<b><font color=\"#59E817\">" + newMsg + "</font></b>"; //GREEN
         //std::cout << "alt: " << newMsg.toAscii().data() << std::endl;
       }
      else if (newMsg.contains("warning"))
      {
        newMsg = "<b><font color=\"#FF8040\">" + newMsg + "</font></b>"; //ORANGE
      }
      else
      {
        newMsg += "<br>";
      }
    }
    if(!newMsg.contains(QRegExp("</b")))
    {
      if(newMsg.contains("::"))
      {
        if (!newMsg.contains(QRegExp("<br")))
          newMsg = "<br><B>" + newMsg + "</B><br>";
        else
          newMsg = "<br><B>" + newMsg + "</B>";
      }
    }
    else if (!newMsg.contains(QRegExp("<br")))
    {
      newMsg += "<br>";
    }

    text->insertHtml(newMsg);
    text->ensureCursorVisible();    
  }
}
